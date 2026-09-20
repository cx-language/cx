#include "analyzer.h"
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <fstream>
#include <optional>
#include <sstream>
#pragma warning(push, 0)
#include <llvm/ADT/STLExtras.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/Process.h>
#pragma warning(pop)
#include "../ast/decl.h"
#include "../ast/expr.h"
#include "../ast/module.h"
#include "../ast/stmt.h"
#include "../ast/type.h"
#include "../build/config.h"
#include "../driver/driver.h"
#include "../parser/parse.h"
#include "../sema/typecheck.h"

using namespace cx;

namespace cx::lsp {

/// Converts a hexadecimal digit to its numeric value. The caller must ensure
/// that `ch` is a valid hexadecimal digit (e.g. with std::isxdigit).
static unsigned hexValue(char ch) {
    if (ch >= '0' && ch <= '9') return static_cast<unsigned>(ch - '0');
    if (ch >= 'a' && ch <= 'f') return static_cast<unsigned>(ch - 'a' + 10);
    return static_cast<unsigned>(ch - 'A' + 10);
}

std::string uriToPath(const std::string& uri) {
    const std::string filePrefix = "file://";
    if (uri.compare(0, filePrefix.size(), filePrefix) != 0) return uri;
    std::string rest = uri.substr(filePrefix.size());
    // Split off the authority (e.g. "localhost" in file://localhost/path).
    std::string path;
    size_t slash = rest.find('/');
    std::string authority = slash == std::string::npos ? rest : rest.substr(0, slash);
    std::string afterAuthority = slash == std::string::npos ? "" : rest.substr(slash);
    if (authority.empty() || authority == "localhost") {
        path = afterAuthority.empty() ? "/" : afterAuthority;
    } else {
        // Non-local host: best effort, keep it addressable as an absolute path.
        path = "/" + rest;
    }
    // Percent-decode.
    std::string out;
    for (size_t i = 0; i < path.size(); ++i) {
        if (path[i] == '%' && i + 2 < path.size() && std::isxdigit(path[i + 1]) && std::isxdigit(path[i + 2])) {
            out += static_cast<char>(hexValue(path[i + 1]) * 16 + hexValue(path[i + 2]));
            i += 2;
        } else {
            out += path[i];
        }
    }
    // Convert file:///C:/path to C:/path (drive-letter paths cannot occur on
    // POSIX hosts anyway).
    if (out.size() >= 3 && out[0] == '/' && std::isalpha(static_cast<unsigned char>(out[1])) && out[2] == ':') {
        out.erase(out.begin());
    }
    return out;
}

std::string pathToUri(const std::string& path) {
    // Normalize to forward slashes first so Windows paths encode correctly.
    std::string normalized;
    normalized.reserve(path.size() + 1);
    // A bare drive-relative path (C:/...) needs a leading slash for file:/// form.
    if (path.size() >= 2 && std::isalpha(static_cast<unsigned char>(path[0])) && path[1] == ':') normalized += '/';
    for (char ch : path) {
        normalized += (ch == '\\' ? '/' : ch);
    }
    std::string out = "file://";
    for (unsigned char ch : normalized) {
        if (std::isalnum(ch) || ch == '/' || ch == '-' || ch == '_' || ch == '.' || ch == '~' || ch == ':') {
            out += static_cast<char>(ch);
        } else {
            char escaped[4] = {};
            std::snprintf(escaped, sizeof(escaped), "%%%02X", ch);
            out += escaped;
        }
    }
    return out;
}

LspRange nameRange(const Location& loc, size_t nameLength) {
    LspRange range;
    if (!loc.isValid()) return range;
    range.start.line = loc.line - 1;
    range.start.character = loc.column - 1;
    range.end.line = loc.line - 1;
    range.end.character = loc.column - 1 + static_cast<int>(nameLength);
    return range;
}

LspRange locationToRange(const Location& loc, const std::string& lineText) {
    LspRange range;
    if (!loc.isValid()) return range;
    range.start.line = loc.line - 1;
    range.start.character = loc.column - 1;
    range.end.line = loc.line - 1;
    int end = loc.column - 1;
    if (!lineText.empty() && end >= 0 && end < static_cast<int>(lineText.size())) {
        // Extend over identifier characters for a useful squiggle.
        if (std::isalnum(static_cast<unsigned char>(lineText[end])) || lineText[end] == '_') {
            while (end < static_cast<int>(lineText.size()) && (std::isalnum(static_cast<unsigned char>(lineText[end])) || lineText[end] == '_')) {
                ++end;
            }
        } else {
            ++end;
        }
    } else {
        ++end;
    }
    range.end.character = end;
    return range;
}

namespace {

struct DiagnosticCollectorScope {
    explicit DiagnosticCollectorScope(std::vector<CollectedDiagnostic>* d) { diagnosticCollector = d; }
    ~DiagnosticCollectorScope() { diagnosticCollector = nullptr; }
};

std::string readLineFromDisk(const std::string& filePath, int line1Based) {
    std::ifstream file(filePath);
    if (!file) return "";
    std::string line;
    for (int i = 1; i <= line1Based; ++i) {
        if (!std::getline(file, line)) return "";
    }
    // Strip trailing \r for CRLF files.
    if (!line.empty() && line.back() == '\r') line.pop_back();
    return line;
}

/// Finds the build root containing filePath by walking up from parentDir
/// looking for a build.cx file, or nullopt if the file stands alone.
/// Only locates the target root (mirroring `cx build` without running it);
/// dependencies are still resolved via import search paths, never fetched.
std::optional<std::string> findBuildRoot(const std::string& filePath, const std::string& parentDir) {
    std::string dir = parentDir;
    while (true) {
        std::string buildFilePath = dir + "/" + BuildConfig::buildFileName;
        bool isFile = false;
        if (!llvm::sys::fs::is_regular_file(buildFilePath, isFile) && isFile) {
            BuildConfig config{std::string(dir)};
            for (auto& root : config.getTargetRootDirectories()) {
                // Either separator: file paths may use backslashes on Windows
                // while roots built from URIs use forward slashes.
                if (filePath == root || llvm::StringRef(filePath).starts_with(root + "/") || llvm::StringRef(filePath).starts_with(root + "\\")) {
                    return root;
                }
            }
            return std::nullopt;
        }
        llvm::StringRef parent = llvm::sys::path::parent_path(dir);
        if (parent == dir) return std::nullopt; // Filesystem root reached.
        dir = parent.str();
    }
}

/// The main module plus every imported module, visiting the main module only
/// once when it also serves as an imported package (see runFrontendOnce).
std::vector<Module*> allModules(Module* mainModule) {
    std::vector<Module*> modules;
    if (mainModule) modules.push_back(mainModule);
    for (auto* imported : Module::getAllImportedModules()) {
        if (imported != mainModule) modules.push_back(imported);
    }
    return modules;
}

bool locationCovers(const Location& loc, size_t nameLength, const std::string& file, int line0, int char0) {
    if (!loc.isValid() || !loc.file) return false;
    if (file != loc.file) return false;
    if (loc.line - 1 != line0) return false;
    int start = loc.column - 1;
    return char0 >= start && char0 < start + static_cast<int>(nameLength ? nameLength : 1);
}

std::string declKindLabel(const Decl& decl) {
    switch (decl.kind) {
    case DeclKind::FunctionDecl:
    case DeclKind::MethodDecl:
    case DeclKind::ConstructorDecl:
    case DeclKind::DestructorDecl:
        return "function";
    case DeclKind::FunctionTemplate:
        return "function";
    case DeclKind::TypeDecl:
        if (auto* typeDecl = llvm::dyn_cast<TypeDecl>(&decl)) {
            if (typeDecl->isStruct()) return "struct";
            if (typeDecl->isInterface()) return "interface";
            if (typeDecl->isUnion()) return "union";
        }
        return "type";
    case DeclKind::TypeTemplate:
        return "type";
    case DeclKind::EnumDecl:
        return "enum";
    case DeclKind::EnumCase:
        return "enum case";
    case DeclKind::VarDecl:
        return "variable";
    case DeclKind::FieldDecl:
        return "field";
    case DeclKind::ParamDecl:
        return "parameter";
    case DeclKind::GenericParamDecl:
        return "type parameter";
    case DeclKind::ImportDecl:
        return "import";
    }
    return "declaration";
}

std::string formatFunctionSignature(const FunctionDecl& decl) {
    std::ostringstream out;
    out << decl.getReturnType().toString() << " " << decl.getQualifiedName() << "(";
    auto params = decl.getParams();
    for (size_t i = 0; i < params.size(); ++i) {
        if (i != 0) out << ", ";
        if (params[i].type) out << params[i].type.toString() << " ";
        out << params[i].getName().str();
    }
    if (decl.isVariadic()) {
        if (!params.empty()) out << ", ";
        out << "...";
    }
    out << ")";
    return out.str();
}

std::string hoverForDecl(const Decl& decl) {
    std::ostringstream out;
    switch (decl.kind) {
    case DeclKind::FunctionDecl:
    case DeclKind::MethodDecl:
    case DeclKind::ConstructorDecl:
    case DeclKind::DestructorDecl: {
        auto& fn = llvm::cast<FunctionDecl>(decl);
        out << formatFunctionSignature(fn);
        break;
    }
    case DeclKind::FunctionTemplate: {
        auto& tmpl = llvm::cast<FunctionTemplate>(decl);
        if (!tmpl.functionDecl) return "";
        out << formatFunctionSignature(*tmpl.functionDecl) << "  // generic";
        break;
    }
    case DeclKind::TypeDecl: {
        auto& typeDecl = llvm::cast<TypeDecl>(decl);
        if (typeDecl.isStruct())
            out << "struct ";
        else if (typeDecl.isInterface())
            out << "interface ";
        else if (typeDecl.isUnion())
            out << "union ";
        out << typeDecl.getQualifiedName();
        break;
    }
    case DeclKind::TypeTemplate: {
        auto& tmpl = llvm::cast<TypeTemplate>(decl);
        out << "struct " << tmpl.getName().str() << "<...>";
        break;
    }
    case DeclKind::EnumDecl: {
        auto& enumDecl = llvm::cast<EnumDecl>(decl);
        out << "enum " << enumDecl.getName().str();
        break;
    }
    case DeclKind::EnumCase: {
        auto& enumCase = llvm::cast<EnumCase>(decl);
        if (auto* enumDecl = enumCase.getEnumDecl()) {
            out << enumCase.type.toString() << " " << enumDecl->getName().str() << "." << enumCase.getName().str();
        } else {
            out << enumCase.getName().str();
        }
        break;
    }
    case DeclKind::VarDecl: {
        auto& var = llvm::cast<VarDecl>(decl);
        out << (var.type ? var.type.toString() + " " : "") << var.getName().str();
        break;
    }
    case DeclKind::FieldDecl: {
        auto& field = llvm::cast<FieldDecl>(decl);
        out << (field.type ? field.type.toString() + " " : "") << field.getQualifiedName();
        break;
    }
    case DeclKind::ParamDecl: {
        auto& param = llvm::cast<ParamDecl>(decl);
        out << (param.type ? param.type.toString() + " " : "") << param.getName().str();
        break;
    }
    case DeclKind::GenericParamDecl:
        out << "type " << decl.getName().str();
        break;
    case DeclKind::ImportDecl:
        out << "import " << llvm::cast<ImportDecl>(decl).target;
        break;
    }
    return out.str();
}

/// Walks the AST to find the reference or definition under the cursor. The
/// deepest match wins, so references inside nested expressions and bodies
/// resolve to the innermost declaration.
struct Finder {
    const std::string& file;
    int line;
    int character;
    SymbolInfo best;
    int bestDepth = -1;

    void consider(Decl* decl, const Location& refLoc, size_t nameLength, Expr* expr, bool isDef, int depth) {
        if (!locationCovers(refLoc, nameLength, file, line, character)) return;
        if (depth < bestDepth) return;
        best.decl = decl;
        best.expr = expr;
        best.refLocation = refLoc;
        best.isDefinition = isDef;
        bestDepth = depth;
    }

    void visitExpr(Expr* expr, int depth);
    void visitStmt(Stmt* stmt, int depth);
    void visitDecl(Decl* decl, int depth);
    // Resolves type references (annotations, return types, sizeof operands)
    // so hover/definition work on type names too.
    void visitType(Type type, int depth) {
        if (!type) return;
        if (TypeDecl* typeDecl = type.getDecl()) {
            // Approximate the source span with the printed spelling; it
            // matches the source in the common cases (`Point`, `const Point`,
            // `Point?`, `int[10]`). Generic arguments are visited below at a
            // deeper level so they win over the outer name.
            consider(typeDecl, type.location, type.toString().size(), nullptr, false, depth);
        }
        switch (type.getKind()) {
        case TypeKind::BasicType:
            for (Type arg : type.getGenericArgs())
                visitType(arg, depth + 1);
            break;
        case TypeKind::ArrayType:
            visitType(type.getElementType(), depth + 1);
            break;
        case TypeKind::TupleType:
            for (auto& element : type.getTupleElements())
                visitType(element.type, depth + 1);
            break;
        case TypeKind::FunctionType:
            visitType(type.getReturnType(), depth + 1);
            for (Type param : type.getParamTypes())
                visitType(param, depth + 1);
            break;
        case TypeKind::PointerType:
            visitType(type.getPointee(), depth + 1);
            break;
        case TypeKind::UnresolvedType:
            break;
        }
    }
};

void Finder::visitExpr(Expr* expr, int depth) {
    if (!expr) return;
    switch (expr->kind) {
    case ExprKind::VarExpr: {
        auto* var = llvm::cast<VarExpr>(expr);
        if (var->decl) consider(var->decl, var->location, var->identifier.size(), expr, false, depth);
        return;
    }
    case ExprKind::MemberExpr: {
        auto* member = llvm::cast<MemberExpr>(expr);
        visitExpr(member->base, depth + 1);
        if (member->decl) consider(member->decl, member->location, member->member.size(), expr, false, depth);
        return;
    }
    case ExprKind::CallExpr:
    case ExprKind::UnaryExpr:
    case ExprKind::BinaryExpr:
    case ExprKind::IndexExpr:
    case ExprKind::IndexAssignmentExpr: {
        auto* call = llvm::cast<CallExpr>(expr);
        visitExpr(call->callee, depth + 1);
        // Plain function calls resolve through CallExpr::calleeDecl; the
        // callee VarExpr itself is never typechecked so its decl stays null.
        if (call->calleeDecl) {
            if (auto* var = llvm::dyn_cast<VarExpr>(call->callee)) {
                consider(call->calleeDecl, var->location, var->identifier.size(), call->callee, false, depth + 1);
            } else if (auto* member = llvm::dyn_cast<MemberExpr>(call->callee)) {
                consider(call->calleeDecl, member->location, member->member.size(), call->callee, false, depth + 1);
            }
        }
        for (auto& arg : call->args)
            visitExpr(arg.value, depth + 1);
        return;
    }
    case ExprKind::ArrayLiteralExpr: {
        auto* arr = llvm::cast<ArrayLiteralExpr>(expr);
        for (auto* el : arr->elements)
            visitExpr(el, depth + 1);
        return;
    }
    case ExprKind::TupleExpr: {
        auto* tuple = llvm::cast<TupleExpr>(expr);
        for (auto& el : tuple->elements)
            visitExpr(el.value, depth + 1);
        return;
    }
    case ExprKind::UnwrapExpr:
        visitExpr(llvm::cast<UnwrapExpr>(expr)->operand, depth + 1);
        return;
    case ExprKind::LambdaExpr: {
        auto* lambda = llvm::cast<LambdaExpr>(expr);
        if (lambda->functionDecl) visitDecl(lambda->functionDecl, depth + 1);
        return;
    }
    case ExprKind::IfExpr: {
        auto* ifExpr = llvm::cast<IfExpr>(expr);
        visitExpr(ifExpr->condition, depth + 1);
        visitExpr(ifExpr->thenExpr, depth + 1);
        visitExpr(ifExpr->elseExpr, depth + 1);
        return;
    }
    case ExprKind::ImplicitCastExpr:
        visitExpr(llvm::cast<ImplicitCastExpr>(expr)->operand, depth + 1);
        return;
    case ExprKind::VarDeclExpr: {
        auto* varDeclExpr = llvm::cast<VarDeclExpr>(expr);
        visitDecl(varDeclExpr->varDecl, depth + 1);
        return;
    }
    case ExprKind::StringLiteralExpr:
    case ExprKind::CharacterLiteralExpr:
    case ExprKind::IntLiteralExpr:
    case ExprKind::FloatLiteralExpr:
    case ExprKind::BoolLiteralExpr:
    case ExprKind::NullLiteralExpr:
    case ExprKind::UndefinedLiteralExpr:
        return;
    case ExprKind::SizeofExpr:
        visitType(llvm::cast<SizeofExpr>(expr)->operandType, depth + 1);
        return;
    }
}

void Finder::visitStmt(Stmt* stmt, int depth) {
    if (!stmt) return;
    switch (stmt->kind) {
    case StmtKind::ReturnStmt:
        visitExpr(llvm::cast<ReturnStmt>(stmt)->value, depth + 1);
        return;
    case StmtKind::VarStmt: {
        auto* varStmt = llvm::cast<VarStmt>(stmt);
        visitDecl(varStmt->decl, depth + 1);
        return;
    }
    case StmtKind::ExprStmt:
        visitExpr(llvm::cast<ExprStmt>(stmt)->expr, depth + 1);
        return;
    case StmtKind::DeferStmt:
        visitExpr(llvm::cast<DeferStmt>(stmt)->expr, depth + 1);
        return;
    case StmtKind::IfStmt: {
        auto* ifStmt = llvm::cast<IfStmt>(stmt);
        visitExpr(ifStmt->condition, depth + 1);
        for (auto* s : ifStmt->thenBody)
            visitStmt(s, depth + 1);
        for (auto* s : ifStmt->elseBody)
            visitStmt(s, depth + 1);
        return;
    }
    case StmtKind::SwitchStmt: {
        auto* switchStmt = llvm::cast<SwitchStmt>(stmt);
        visitExpr(switchStmt->condition, depth + 1);
        for (auto& c : switchStmt->cases) {
            visitExpr(c.value, depth + 1);
            if (c.associatedValue) visitDecl(c.associatedValue, depth + 1);
            for (auto* s : c.stmts)
                visitStmt(s, depth + 1);
        }
        for (auto* s : switchStmt->defaultStmts)
            visitStmt(s, depth + 1);
        return;
    }
    case StmtKind::WhileStmt: {
        auto* whileStmt = llvm::cast<WhileStmt>(stmt);
        visitExpr(whileStmt->condition, depth + 1);
        for (auto* s : whileStmt->body)
            visitStmt(s, depth + 1);
        return;
    }
    case StmtKind::ForStmt: {
        auto* forStmt = llvm::cast<ForStmt>(stmt);
        if (forStmt->variable) visitStmt(forStmt->variable, depth + 1);
        visitExpr(forStmt->condition, depth + 1);
        visitExpr(forStmt->increment, depth + 1);
        for (auto* s : forStmt->body)
            visitStmt(s, depth + 1);
        return;
    }
    case StmtKind::ForEachStmt: {
        auto* forEach = llvm::cast<ForEachStmt>(stmt);
        if (forEach->variable) visitDecl(forEach->variable, depth + 1);
        visitExpr(forEach->range, depth + 1);
        for (auto* s : forEach->body)
            visitStmt(s, depth + 1);
        return;
    }
    case StmtKind::BreakStmt:
    case StmtKind::ContinueStmt:
    case StmtKind::CompoundStmt:
        if (auto* compound = llvm::dyn_cast<CompoundStmt>(stmt)) {
            for (auto* s : compound->body)
                visitStmt(s, depth + 1);
        }
        return;
    }
}

void Finder::visitDecl(Decl* decl, int depth) {
    if (!decl) return;
    // Definition name under cursor?
    consider(decl, decl->getLocation(), decl->getName().size(), nullptr, true, depth);

    switch (decl->kind) {
    case DeclKind::FunctionDecl:
    case DeclKind::MethodDecl:
    case DeclKind::ConstructorDecl:
    case DeclKind::DestructorDecl: {
        auto* fn = llvm::cast<FunctionDecl>(decl);
        visitType(fn->getReturnType(), depth + 1);
        for (auto& param : fn->getParams()) {
            consider(&param, param.getLocation(), param.getName().size(), nullptr, true, depth + 1);
            visitType(param.type, depth + 1);
        }
        if (fn->body) {
            for (auto* s : *fn->body)
                visitStmt(s, depth + 1);
        }
        return;
    }
    case DeclKind::FunctionTemplate: {
        auto* tmpl = llvm::cast<FunctionTemplate>(decl);
        visitDecl(tmpl->functionDecl, depth + 1);
        return;
    }
    case DeclKind::TypeDecl: {
        auto* typeDecl = llvm::cast<TypeDecl>(decl);
        for (Type interface : typeDecl->interfaces)
            visitType(interface, depth + 1);
        for (Type arg : typeDecl->genericArgs)
            visitType(arg, depth + 1);
        for (auto& field : typeDecl->fields) {
            consider(&field, field.getLocation(), field.getName().size(), nullptr, true, depth + 1);
            visitType(field.type, depth + 1);
            if (field.defaultValue) visitExpr(field.defaultValue, depth + 2);
        }
        for (auto* method : typeDecl->methods)
            visitDecl(method, depth + 1);
        return;
    }
    case DeclKind::TypeTemplate:
        visitDecl(llvm::cast<TypeTemplate>(decl)->typeDecl, depth + 1);
        return;
    case DeclKind::EnumDecl: {
        auto* enumDecl = llvm::cast<EnumDecl>(decl);
        for (auto& c : enumDecl->cases) {
            consider(&c, c.getLocation(), c.getName().size(), nullptr, true, depth + 1);
        }
        return;
    }
    case DeclKind::VarDecl: {
        auto* var = llvm::cast<VarDecl>(decl);
        visitType(var->type, depth + 1);
        if (var->initializer) visitExpr(var->initializer, depth + 1);
        return;
    }
    case DeclKind::FieldDecl: {
        auto* field = llvm::cast<FieldDecl>(decl);
        visitType(field->type, depth + 1);
        if (field->defaultValue) visitExpr(field->defaultValue, depth + 1);
        return;
    }
    default:
        return;
    }
}

/// Collects the definition and all reference locations of a single
/// declaration for the references query.
struct ReferenceCollector {
    Decl* target;
    std::vector<std::pair<Location, size_t>> locations;

    void visitExpr(Expr* expr);
    void visitStmt(Stmt* stmt);
    void visitDecl(Decl* decl);
    void visitType(Type type) {
        if (!type) return;
        if (type.getDecl() == target) locations.emplace_back(type.location, type.toString().size());
        switch (type.getKind()) {
        case TypeKind::BasicType:
            for (Type arg : type.getGenericArgs())
                visitType(arg);
            break;
        case TypeKind::ArrayType:
            visitType(type.getElementType());
            break;
        case TypeKind::TupleType:
            for (auto& element : type.getTupleElements())
                visitType(element.type);
            break;
        case TypeKind::FunctionType:
            visitType(type.getReturnType());
            for (Type param : type.getParamTypes())
                visitType(param);
            break;
        case TypeKind::PointerType:
            visitType(type.getPointee());
            break;
        case TypeKind::UnresolvedType:
            break;
        }
    }
};

void ReferenceCollector::visitExpr(Expr* expr) {
    if (!expr) return;
    switch (expr->kind) {
    case ExprKind::VarExpr: {
        auto* var = llvm::cast<VarExpr>(expr);
        if (var->decl == target) locations.emplace_back(var->location, var->identifier.size());
        return;
    }
    case ExprKind::MemberExpr: {
        auto* member = llvm::cast<MemberExpr>(expr);
        visitExpr(member->base);
        if (member->decl == target) locations.emplace_back(member->location, member->member.size());
        return;
    }
    case ExprKind::CallExpr:
    case ExprKind::UnaryExpr:
    case ExprKind::BinaryExpr:
    case ExprKind::IndexExpr:
    case ExprKind::IndexAssignmentExpr: {
        auto* call = llvm::cast<CallExpr>(expr);
        visitExpr(call->callee);
        // See Finder: plain calls resolve through calleeDecl, leaving the
        // callee VarExpr's decl null.
        if (call->calleeDecl == target) {
            if (auto* var = llvm::dyn_cast<VarExpr>(call->callee)) {
                locations.emplace_back(var->location, var->identifier.size());
            } else if (auto* member = llvm::dyn_cast<MemberExpr>(call->callee)) {
                locations.emplace_back(member->location, member->member.size());
            }
        }
        for (auto& arg : call->args)
            visitExpr(arg.value);
        return;
    }
    case ExprKind::ArrayLiteralExpr:
        for (auto* el : llvm::cast<ArrayLiteralExpr>(expr)->elements)
            visitExpr(el);
        return;
    case ExprKind::TupleExpr:
        for (auto& el : llvm::cast<TupleExpr>(expr)->elements)
            visitExpr(el.value);
        return;
    case ExprKind::UnwrapExpr:
        visitExpr(llvm::cast<UnwrapExpr>(expr)->operand);
        return;
    case ExprKind::LambdaExpr:
        if (auto* fn = llvm::cast<LambdaExpr>(expr)->functionDecl) visitDecl(fn);
        return;
    case ExprKind::IfExpr: {
        auto* ifExpr = llvm::cast<IfExpr>(expr);
        visitExpr(ifExpr->condition);
        visitExpr(ifExpr->thenExpr);
        visitExpr(ifExpr->elseExpr);
        return;
    }
    case ExprKind::ImplicitCastExpr:
        visitExpr(llvm::cast<ImplicitCastExpr>(expr)->operand);
        return;
    case ExprKind::VarDeclExpr:
        visitDecl(llvm::cast<VarDeclExpr>(expr)->varDecl);
        return;
    case ExprKind::SizeofExpr:
        visitType(llvm::cast<SizeofExpr>(expr)->operandType);
        return;
    default:
        return;
    }
}

void ReferenceCollector::visitStmt(Stmt* stmt) {
    if (!stmt) return;
    switch (stmt->kind) {
    case StmtKind::ReturnStmt:
        visitExpr(llvm::cast<ReturnStmt>(stmt)->value);
        return;
    case StmtKind::VarStmt:
        visitDecl(llvm::cast<VarStmt>(stmt)->decl);
        return;
    case StmtKind::ExprStmt:
        visitExpr(llvm::cast<ExprStmt>(stmt)->expr);
        return;
    case StmtKind::DeferStmt:
        visitExpr(llvm::cast<DeferStmt>(stmt)->expr);
        return;
    case StmtKind::IfStmt: {
        auto* ifStmt = llvm::cast<IfStmt>(stmt);
        visitExpr(ifStmt->condition);
        for (auto* s : ifStmt->thenBody)
            visitStmt(s);
        for (auto* s : ifStmt->elseBody)
            visitStmt(s);
        return;
    }
    case StmtKind::SwitchStmt: {
        auto* switchStmt = llvm::cast<SwitchStmt>(stmt);
        visitExpr(switchStmt->condition);
        for (auto& c : switchStmt->cases) {
            visitExpr(c.value);
            if (c.associatedValue) visitDecl(c.associatedValue);
            for (auto* s : c.stmts)
                visitStmt(s);
        }
        for (auto* s : switchStmt->defaultStmts)
            visitStmt(s);
        return;
    }
    case StmtKind::WhileStmt: {
        auto* whileStmt = llvm::cast<WhileStmt>(stmt);
        visitExpr(whileStmt->condition);
        for (auto* s : whileStmt->body)
            visitStmt(s);
        return;
    }
    case StmtKind::ForStmt: {
        auto* forStmt = llvm::cast<ForStmt>(stmt);
        if (forStmt->variable) visitStmt(forStmt->variable);
        visitExpr(forStmt->condition);
        visitExpr(forStmt->increment);
        for (auto* s : forStmt->body)
            visitStmt(s);
        return;
    }
    case StmtKind::ForEachStmt: {
        auto* forEach = llvm::cast<ForEachStmt>(stmt);
        if (forEach->variable) visitDecl(forEach->variable);
        visitExpr(forEach->range);
        for (auto* s : forEach->body)
            visitStmt(s);
        return;
    }
    case StmtKind::CompoundStmt:
        for (auto* s : llvm::cast<CompoundStmt>(stmt)->body)
            visitStmt(s);
        return;
    default:
        return;
    }
}

void ReferenceCollector::visitDecl(Decl* decl) {
    if (!decl) return;
    if (decl == target) {
        locations.emplace_back(decl->getLocation(), decl->getName().size());
    }
    switch (decl->kind) {
    case DeclKind::FunctionDecl:
    case DeclKind::MethodDecl:
    case DeclKind::ConstructorDecl:
    case DeclKind::DestructorDecl: {
        auto* fn = llvm::cast<FunctionDecl>(decl);
        visitType(fn->getReturnType());
        for (auto& param : fn->getParams()) {
            if (&param == target) locations.emplace_back(param.getLocation(), param.getName().size());
            visitType(param.type);
        }
        if (fn->body) {
            for (auto* s : *fn->body)
                visitStmt(s);
        }
        return;
    }
    case DeclKind::FunctionTemplate:
        visitDecl(llvm::cast<FunctionTemplate>(decl)->functionDecl);
        return;
    case DeclKind::TypeDecl: {
        auto* typeDecl = llvm::cast<TypeDecl>(decl);
        for (Type interface : typeDecl->interfaces)
            visitType(interface);
        for (Type arg : typeDecl->genericArgs)
            visitType(arg);
        for (auto& field : typeDecl->fields) {
            if (&field == target) locations.emplace_back(field.getLocation(), field.getName().size());
            visitType(field.type);
            if (field.defaultValue) visitExpr(field.defaultValue);
        }
        for (auto* method : typeDecl->methods)
            visitDecl(method);
        return;
    }
    case DeclKind::TypeTemplate:
        visitDecl(llvm::cast<TypeTemplate>(decl)->typeDecl);
        return;
    case DeclKind::EnumDecl: {
        auto* enumDecl = llvm::cast<EnumDecl>(decl);
        for (auto& c : enumDecl->cases) {
            if (&c == target) locations.emplace_back(c.getLocation(), c.getName().size());
        }
        return;
    }
    case DeclKind::VarDecl: {
        auto* var = llvm::cast<VarDecl>(decl);
        visitType(var->type);
        if (auto* init = var->initializer) visitExpr(init);
        return;
    }
    case DeclKind::FieldDecl: {
        auto* field = llvm::cast<FieldDecl>(decl);
        visitType(field->type);
        if (auto* def = field->defaultValue) visitExpr(def);
        return;
    }
    default:
        return;
    }
}

/// Maps a resolved declaration to its highlight type, or null when it has no
/// highlightable name (imports).
const char* tokenTypeForDecl(const Decl& decl) {
    switch (decl.kind) {
    case DeclKind::FunctionDecl:
        return "function";
    case DeclKind::MethodDecl:
    case DeclKind::ConstructorDecl:
    case DeclKind::DestructorDecl:
        return "method";
    case DeclKind::FunctionTemplate: {
        auto* fn = llvm::cast<FunctionTemplate>(&decl)->functionDecl;
        if (fn && fn->isMethodDecl()) return "method";
        return "function";
    }
    case DeclKind::TypeDecl:
        if (llvm::cast<TypeDecl>(&decl)->isInterface()) return "interface";
        return "struct";
    case DeclKind::TypeTemplate: {
        auto* inner = llvm::cast<TypeTemplate>(&decl)->typeDecl;
        if (inner && inner->isInterface()) return "interface";
        return "struct";
    }
    case DeclKind::EnumDecl:
        return "enum";
    case DeclKind::EnumCase:
        return "enumMember";
    case DeclKind::VarDecl:
        return "variable";
    case DeclKind::FieldDecl:
        return "property";
    case DeclKind::ParamDecl:
        return "parameter";
    case DeclKind::GenericParamDecl:
        return "typeParameter";
    case DeclKind::ImportDecl:
        return nullptr;
    }
    return nullptr;
}

/// Tolerant single-pass scanner for syntax-only tokens (comments, strings,
/// numbers, keywords). Deliberately independent of Lexer: it never reports
/// errors or throws, so half-typed code still highlights something sane.
/// Identifiers are left to the AST pass, which knows their semantic type.
void collectSyntaxTokens(const std::string& content, std::vector<SemanticToken>& out) {
    // Mirrors the keyword table in lex.cpp; hash-directives highlight as macros.
    static const llvm::StringMap<const char*> keywords = {
        {"break", "keyword"},     {"case", "keyword"},   {"const", "keyword"},     {"continue", "keyword"}, {"default", "keyword"}, {"defer", "keyword"},
        {"else", "keyword"},      {"enum", "keyword"},   {"extern", "keyword"},    {"false", "keyword"},    {"for", "keyword"},     {"if", "keyword"},
        {"import", "keyword"},    {"in", "keyword"},     {"interface", "keyword"}, {"null", "keyword"},     {"private", "keyword"}, {"public", "keyword"},
        {"return", "keyword"},    {"sizeof", "keyword"}, {"struct", "keyword"},    {"switch", "keyword"},   {"this", "keyword"},    {"true", "keyword"},
        {"undefined", "keyword"}, {"var", "keyword"},    {"while", "keyword"},     {"#if", "macro"},        {"#else", "macro"},     {"#endif", "macro"},
    };
    auto emit = [&](int line, int start, int length, const char* type) {
        if (length <= 0) return;
        SemanticToken token;
        token.line = line;
        token.start = start;
        token.length = length;
        token.type = type;
        out.push_back(std::move(token));
    };

    size_t i = 0;
    int line = 0;
    int col = 0;
    while (i < content.size()) {
        char ch = content[i];
        if (ch == '\n') {
            ++i;
            ++line;
            col = 0;
            continue;
        }
        if (ch == '\r') {
            ++i;
            continue;
        }
        if (ch == '/' && i + 1 < content.size() && content[i + 1] == '/') {
            int start = col;
            while (i < content.size() && content[i] != '\n') {
                if (content[i] != '\r') ++col;
                ++i;
            }
            emit(line, start, col - start, "comment");
            continue;
        }
        if (ch == '/' && i + 1 < content.size() && content[i + 1] == '*') {
            // Nested block comment, split per line for the single-line LSP encoding.
            int nest = 0;
            int segLine = line;
            int segStart = col;
            while (i < content.size()) {
                if (content[i] == '\n') {
                    emit(segLine, segStart, col - segStart, "comment");
                    ++i;
                    ++line;
                    col = 0;
                    segLine = line;
                    segStart = 0;
                    continue;
                }
                if (content[i] == '\r') {
                    ++i;
                    continue;
                }
                if (content[i] == '/' && i + 1 < content.size() && content[i + 1] == '*') {
                    ++nest;
                    i += 2;
                    col += 2;
                    continue;
                }
                if (content[i] == '*' && i + 1 < content.size() && content[i + 1] == '/') {
                    --nest;
                    i += 2;
                    col += 2;
                    if (nest == 0) break;
                    continue;
                }
                ++i;
                ++col;
            }
            emit(segLine, segStart, col - segStart, "comment");
            continue;
        }
        if (ch == '"' || ch == '\'') {
            char delimiter = ch;
            int start = col;
            ++i;
            ++col;
            while (i < content.size()) {
                char c = content[i];
                if (c == '\n' || c == '\r') break; // Unterminated: highlight to end of line.
                if (c == '\\' && i + 1 < content.size() && content[i + 1] != '\n' && content[i + 1] != '\r') {
                    i += 2;
                    col += 2;
                    continue;
                }
                ++i;
                ++col;
                if (c == delimiter) break;
            }
            emit(line, start, col - start, "string");
            continue;
        }
        if (ch >= '0' && ch <= '9') {
            int start = col;
            // Like lex.cpp readNumber: only 0x/0b/0o-prefixed literals take
            // letters, so `123abc` highlights as `123` plus an identifier.
            bool prefixed = ch == '0' && i + 1 < content.size() && (content[i + 1] == 'x' || content[i + 1] == 'b' || content[i + 1] == 'o');
            while (i < content.size()) {
                char c = content[i];
                if (std::isdigit(static_cast<unsigned char>(c)) || c == '_') {
                    ++i;
                    ++col;
                    continue;
                }
                if (prefixed && std::isalpha(static_cast<unsigned char>(c))) {
                    ++i;
                    ++col;
                    continue;
                }
                // A dot only continues the number before a digit (`1.5`, not `0..10` or `0.foo`).
                if (!prefixed && c == '.' && i + 1 < content.size() && std::isdigit(static_cast<unsigned char>(content[i + 1]))) {
                    ++i;
                    ++col;
                    continue;
                }
                break;
            }
            emit(line, start, col - start, "number");
            continue;
        }
        if (std::isalpha(static_cast<unsigned char>(ch)) || ch == '_' || ch == '#') {
            int start = col;
            size_t begin = i;
            ++i;
            ++col;
            while (i < content.size() && (std::isalnum(static_cast<unsigned char>(content[i])) || content[i] == '_')) {
                ++i;
                ++col;
            }
            auto it = keywords.find(llvm::StringRef(content.data() + begin, i - begin));
            if (it != keywords.end()) emit(line, start, col - start, it->second);
            continue;
        }
        ++i;
        ++col;
    }
}

/// Emits one highlight token per declaration and reference in the target file.
/// Traversal mirrors Finder; only filePath locations are kept.
struct SemanticCollector {
    const std::string& file;
    const std::vector<std::string>& lines;
    std::vector<SemanticToken>& out;
    std::vector<std::string> genericParamNames; // In-scope generic parameters, for typeParameter uses.

    void emit(const Location& loc, llvm::StringRef name, const char* type, bool definition) {
        if (!loc.isValid() || !loc.file || name.empty()) return;
        if (file != loc.file) return;
        // Synthesized nodes (autogenerated constructors) reuse real source
        // locations, so only highlight when the source actually spells the name.
        if (loc.line - 1 >= static_cast<int>(lines.size())) return;
        const std::string& lineText = lines[loc.line - 1];
        if (loc.column - 1 + static_cast<int>(name.size()) > static_cast<int>(lineText.size())) return;
        if (lineText.compare(loc.column - 1, name.size(), name.data(), name.size()) != 0) return;
        SemanticToken token;
        token.line = loc.line - 1;
        token.start = loc.column - 1;
        token.length = static_cast<int>(name.size());
        token.type = type;
        token.definition = definition;
        out.push_back(std::move(token));
    }

    void emitDecl(Decl* decl, bool definition) {
        if (!decl) return;
        if (const char* type = tokenTypeForDecl(*decl)) emit(decl->getLocation(), decl->getName(), type, definition);
    }

    void visitExpr(Expr* expr);
    void visitStmt(Stmt* stmt);
    void visitDecl(Decl* decl);
    void visitType(Type type) {
        if (!type) return;
        switch (type.getKind()) {
        case TypeKind::BasicType: {
            // Optional (`T?`) and ArrayRef (`T[]`) wrappers are synthesized: the
            // location points at the sugar or is invalid, so only the wrapped
            // type highlights. The direct generic args (not getWrappedType())
            // preserve the inner locations.
            if ((type.isOptionalType() || type.isArrayRef()) && !type.getGenericArgs().empty()) {
                for (Type arg : type.getGenericArgs())
                    visitType(arg);
                return;
            }
            if (TypeDecl* typeDecl = type.getDecl()) {
                if (const char* tokenType = tokenTypeForDecl(*typeDecl)) emit(type.location, type.getName(), tokenType, false);
            } else if (llvm::is_contained(genericParamNames, type.getName())) {
                emit(type.location, type.getName(), "typeParameter", false);
            } else {
                // Builtins without decls (`void`) and unresolved names.
                emit(type.location, type.getName(), "type", false);
            }
            for (Type arg : type.getGenericArgs())
                visitType(arg);
            return;
        }
        case TypeKind::ArrayType:
            visitType(llvm::cast<ArrayType>(type.typeBase)->elementType);
            return;
        case TypeKind::TupleType:
            for (auto& element : llvm::cast<TupleType>(type.typeBase)->elements)
                visitType(element.type);
            return;
        case TypeKind::FunctionType: {
            auto* fn = llvm::cast<FunctionType>(type.typeBase);
            visitType(fn->returnType);
            for (Type param : fn->paramTypes)
                visitType(param);
            return;
        }
        case TypeKind::PointerType:
            visitType(llvm::cast<PointerType>(type.typeBase)->pointeeType);
            return;
        case TypeKind::UnresolvedType:
            return;
        }
    }
};

void SemanticCollector::visitExpr(Expr* expr) {
    if (!expr) return;
    switch (expr->kind) {
    case ExprKind::VarExpr: {
        auto* var = llvm::cast<VarExpr>(expr);
        if (var->decl) {
            if (const char* type = tokenTypeForDecl(*var->decl)) emit(var->location, var->identifier, type, false);
        }
        return;
    }
    case ExprKind::MemberExpr: {
        auto* member = llvm::cast<MemberExpr>(expr);
        visitExpr(member->base);
        if (member->decl) {
            if (const char* type = tokenTypeForDecl(*member->decl)) emit(member->location, member->member, type, false);
        }
        return;
    }
    case ExprKind::CallExpr:
    case ExprKind::UnaryExpr:
    case ExprKind::BinaryExpr:
    case ExprKind::IndexExpr:
    case ExprKind::IndexAssignmentExpr: {
        auto* call = llvm::cast<CallExpr>(expr);
        visitExpr(call->callee);
        // See Finder: plain calls resolve through calleeDecl, leaving the
        // callee VarExpr's decl null.
        if (call->calleeDecl) {
            if (const char* type = tokenTypeForDecl(*call->calleeDecl)) {
                if (auto* var = llvm::dyn_cast<VarExpr>(call->callee)) {
                    emit(var->location, var->identifier, type, false);
                } else if (auto* member = llvm::dyn_cast<MemberExpr>(call->callee)) {
                    emit(member->location, member->member, type, false);
                }
            }
        }
        for (auto& arg : call->args)
            visitExpr(arg.value);
        for (Type arg : call->genericArgs)
            visitType(arg);
        return;
    }
    case ExprKind::ArrayLiteralExpr:
        for (auto* el : llvm::cast<ArrayLiteralExpr>(expr)->elements)
            visitExpr(el);
        return;
    case ExprKind::TupleExpr:
        for (auto& el : llvm::cast<TupleExpr>(expr)->elements)
            visitExpr(el.value);
        return;
    case ExprKind::UnwrapExpr:
        visitExpr(llvm::cast<UnwrapExpr>(expr)->operand);
        return;
    case ExprKind::LambdaExpr:
        if (auto* fn = llvm::cast<LambdaExpr>(expr)->functionDecl) visitDecl(fn);
        return;
    case ExprKind::IfExpr: {
        auto* ifExpr = llvm::cast<IfExpr>(expr);
        visitExpr(ifExpr->condition);
        visitExpr(ifExpr->thenExpr);
        visitExpr(ifExpr->elseExpr);
        return;
    }
    case ExprKind::ImplicitCastExpr:
        visitExpr(llvm::cast<ImplicitCastExpr>(expr)->operand);
        return;
    case ExprKind::VarDeclExpr:
        visitDecl(llvm::cast<VarDeclExpr>(expr)->varDecl);
        return;
    case ExprKind::SizeofExpr:
        visitType(llvm::cast<SizeofExpr>(expr)->operandType);
        return;
    default:
        return;
    }
}

void SemanticCollector::visitStmt(Stmt* stmt) {
    if (!stmt) return;
    switch (stmt->kind) {
    case StmtKind::ReturnStmt:
        visitExpr(llvm::cast<ReturnStmt>(stmt)->value);
        return;
    case StmtKind::VarStmt:
        visitDecl(llvm::cast<VarStmt>(stmt)->decl);
        return;
    case StmtKind::ExprStmt:
        visitExpr(llvm::cast<ExprStmt>(stmt)->expr);
        return;
    case StmtKind::DeferStmt:
        visitExpr(llvm::cast<DeferStmt>(stmt)->expr);
        return;
    case StmtKind::IfStmt: {
        auto* ifStmt = llvm::cast<IfStmt>(stmt);
        visitExpr(ifStmt->condition);
        for (auto* s : ifStmt->thenBody)
            visitStmt(s);
        for (auto* s : ifStmt->elseBody)
            visitStmt(s);
        return;
    }
    case StmtKind::SwitchStmt: {
        auto* switchStmt = llvm::cast<SwitchStmt>(stmt);
        visitExpr(switchStmt->condition);
        for (auto& c : switchStmt->cases) {
            visitExpr(c.value);
            if (c.associatedValue) visitDecl(c.associatedValue);
            for (auto* s : c.stmts)
                visitStmt(s);
        }
        for (auto* s : switchStmt->defaultStmts)
            visitStmt(s);
        return;
    }
    case StmtKind::WhileStmt: {
        auto* whileStmt = llvm::cast<WhileStmt>(stmt);
        visitExpr(whileStmt->condition);
        for (auto* s : whileStmt->body)
            visitStmt(s);
        return;
    }
    case StmtKind::ForStmt: {
        auto* forStmt = llvm::cast<ForStmt>(stmt);
        if (forStmt->variable) visitStmt(forStmt->variable);
        visitExpr(forStmt->condition);
        visitExpr(forStmt->increment);
        for (auto* s : forStmt->body)
            visitStmt(s);
        return;
    }
    case StmtKind::ForEachStmt: {
        auto* forEach = llvm::cast<ForEachStmt>(stmt);
        if (forEach->variable) visitDecl(forEach->variable);
        visitExpr(forEach->range);
        for (auto* s : forEach->body)
            visitStmt(s);
        return;
    }
    case StmtKind::CompoundStmt:
        for (auto* s : llvm::cast<CompoundStmt>(stmt)->body)
            visitStmt(s);
        return;
    default:
        return;
    }
}

void SemanticCollector::visitDecl(Decl* decl) {
    if (!decl) return;
    emitDecl(decl, true);
    switch (decl->kind) {
    case DeclKind::FunctionDecl:
    case DeclKind::MethodDecl:
    case DeclKind::ConstructorDecl:
    case DeclKind::DestructorDecl: {
        auto* fn = llvm::cast<FunctionDecl>(decl);
        visitType(fn->getReturnType());
        for (auto& param : fn->getParams()) {
            emitDecl(&param, true);
            visitType(param.type);
        }
        if (fn->body) {
            for (auto* s : *fn->body)
                visitStmt(s);
        }
        return;
    }
    case DeclKind::FunctionTemplate: {
        auto* tmpl = llvm::cast<FunctionTemplate>(decl);
        size_t scope = genericParamNames.size();
        for (auto& param : tmpl->genericParams)
            genericParamNames.push_back(param.getName().str());
        for (auto& param : tmpl->genericParams) {
            emitDecl(&param, true);
            for (Type constraint : param.constraints)
                visitType(constraint);
        }
        visitDecl(tmpl->functionDecl);
        genericParamNames.resize(scope);
        return;
    }
    case DeclKind::TypeDecl: {
        auto* typeDecl = llvm::cast<TypeDecl>(decl);
        for (Type interface : typeDecl->interfaces)
            visitType(interface);
        for (Type arg : typeDecl->genericArgs)
            visitType(arg);
        for (auto& field : typeDecl->fields) {
            emitDecl(&field, true);
            visitType(field.type);
            if (field.defaultValue) visitExpr(field.defaultValue);
        }
        for (auto* method : typeDecl->methods)
            visitDecl(method);
        return;
    }
    case DeclKind::TypeTemplate: {
        auto* tmpl = llvm::cast<TypeTemplate>(decl);
        size_t scope = genericParamNames.size();
        for (auto& param : tmpl->genericParams)
            genericParamNames.push_back(param.getName().str());
        for (auto& param : tmpl->genericParams) {
            emitDecl(&param, true);
            for (Type constraint : param.constraints)
                visitType(constraint);
        }
        visitDecl(tmpl->typeDecl);
        genericParamNames.resize(scope);
        return;
    }
    case DeclKind::EnumDecl: {
        auto* enumDecl = llvm::cast<EnumDecl>(decl);
        for (auto& c : enumDecl->cases) {
            emitDecl(&c, true);
            visitType(c.associatedType);
        }
        return;
    }
    case DeclKind::VarDecl: {
        auto* var = llvm::cast<VarDecl>(decl);
        visitType(var->type);
        if (var->initializer) visitExpr(var->initializer);
        return;
    }
    case DeclKind::FieldDecl: {
        auto* field = llvm::cast<FieldDecl>(decl);
        visitType(field->type);
        if (field->defaultValue) visitExpr(field->defaultValue);
        return;
    }
    default:
        return;
    }
}

} // namespace

FrontendResult runFrontendOnce(const LspQuery& query) {
    FrontendResult result;
    result.filePath = query.filePath;
    result.content = query.content;
    const std::string& filePath = query.filePath;
    const std::string& content = query.content;

    diagnosticOptions.disableWarnings = false;
    diagnosticOptions.warningsAsErrors = false;
    diagnosticOptions.errorLimit = 0; // unlimited; the editor gets everything collected so far

    DiagnosticCollectorScope scope(&result.diagnostics);

    try {
        // Intentionally leaked: this process runs one compilation and exits,
        // so the OS reclaims everything - just like a normal `cx` invocation.
        Module* module = new Module("main");

        CompileOptions options;
        options.noUnusedWarnings = true; // unused warnings are noisy during editing
        options.defines = query.defines;
        // Import search paths: file's directory first, then workspace folders,
        // then explicit extras, then the repo root (for std/) and system paths.
        std::string parentDir = llvm::sys::path::parent_path(filePath).str();
        if (!parentDir.empty()) options.importSearchPaths.push_back(parentDir);
        for (auto& folder : query.workspaceFolders)
            options.importSearchPaths.push_back(folder);
        for (auto& path : query.importSearchPaths)
            options.importSearchPaths.push_back(path);
        options.importSearchPaths.push_back(CX_ROOT_DIR);
#ifdef CLANG_BUILTIN_INCLUDE_PATH
        options.importSearchPaths.push_back(CLANG_BUILTIN_INCLUDE_PATH);
#endif
        options.importSearchPaths.push_back("/usr/include");
        options.importSearchPaths.push_back("/usr/local/include");
        // Same bonus search paths as `cx build` (see driver.cpp). Unlike the
        // driver, queries don't probe the external C compiler for its header
        // paths - C-header imports relying on those need explicit
        // initializationOptions.importSearchPaths.
        for (const char* name : {"CPATH", "C_INCLUDE_PATH", "INCLUDE"}) {
            if (auto paths = llvm::sys::Process::GetEnv(name)) {
                llvm::SmallVector<llvm::StringRef, 16> split;
                llvm::StringRef(*paths).split(split, llvm::sys::EnvPathSeparator, -1, false);
                for (llvm::StringRef path : split)
                    options.importSearchPaths.push_back(path.str());
            }
        }

        // Determine which files form the open file's module: an importable
        // package (registered below so the import resolves to it), a
        // build.cx target root, or just the file itself when standalone.
        std::optional<std::string> moduleDir;
        bool registerAsStd = false;
        if (!parentDir.empty()) {
            for (llvm::StringRef searchPath : options.importSearchPaths) {
                auto candidate = (searchPath + "/std").str();
                if (llvm::sys::fs::is_directory(candidate) && llvm::sys::fs::equivalent(candidate, parentDir)) {
                    moduleDir = parentDir;
                    registerAsStd = true;
                    break;
                }
            }
            if (!registerAsStd) {
                moduleDir = findBuildRoot(filePath, parentDir);
            }
        }

        // Main file from memory, siblings from disk or the openDocs overlay.
        // Build files are config, not source, so never load them as code.
        std::vector<std::string> siblingPaths;
        if (moduleDir) {
            std::error_code ec;
            for (llvm::sys::fs::recursive_directory_iterator it(*moduleDir, ec), end; it != end && !ec; it.increment(ec)) {
                if (llvm::sys::path::extension(it->path()) == ".cx" && it->path() != filePath
                    && llvm::sys::path::filename(it->path()) != BuildConfig::buildFileName) {
                    siblingPaths.push_back(it->path());
                }
            }
            llvm::sort(siblingPaths);
        }

        auto mainBuffer = llvm::MemoryBuffer::getMemBufferCopy(content, filePath);
        module->fileBuffers.push_back(std::move(mainBuffer));
        for (auto& sibling : siblingPaths) {
            auto overlay = query.openDocs.find(sibling);
            if (overlay != query.openDocs.end()) {
                auto buffer = llvm::MemoryBuffer::getMemBufferCopy(overlay->second, sibling);
                module->fileBuffers.push_back(std::move(buffer));
            } else if (auto buffer = llvm::MemoryBuffer::getFile(sibling)) {
                module->fileBuffers.push_back(std::move(*buffer));
            }
        }

        for (auto& fileBuffer : module->fileBuffers) {
            Parser parser(fileBuffer->getMemBufferRef(), *module, options);
            parser.parse();
        }

        // When the open file lives inside the standard library itself, the
        // typechecker's unconditional "std" import would otherwise load the
        // same sources a second time as a separate module, drowning the file
        // in ambiguous-reference errors. Analyze them as that package instead
        // by resolving the import to the main module. This mirrors
        // importModule's lookup order, so it only triggers when the import
        // would actually find this directory.
        if (registerAsStd) {
            Module::getAllImportedModulesMap()["std"] = module;
        }

        Typechecker typechecker(options);
        for (auto* imported : module->getImportedModules()) {
            typechecker.typecheckModule(*imported, nullptr);
        }
        typechecker.typecheckModule(*module, nullptr);
        typechecker.checkUnusedDecls(*module);

        result.mainModule = module;
    } catch (const CompileError& error) {
        if (!error.message.empty()) {
            CollectedDiagnostic diagnostic;
            diagnostic.location = error.location;
            diagnostic.severity = "error";
            diagnostic.message = error.message;
            diagnostic.notes = error.notes;
            result.diagnostics.push_back(std::move(diagnostic));
        }
    } catch (const std::exception& error) {
        CollectedDiagnostic diagnostic;
        diagnostic.location = Location();
        diagnostic.severity = "error";
        diagnostic.message = std::string("internal compiler error: ") + error.what();
        result.diagnostics.push_back(std::move(diagnostic));
    } catch (...) {
        CollectedDiagnostic diagnostic;
        diagnostic.location = Location();
        diagnostic.severity = "error";
        diagnostic.message = "internal compiler error";
        result.diagnostics.push_back(std::move(diagnostic));
    }

    return result;
}

std::vector<LspDiagnostic> toLspDiagnostics(const std::vector<CollectedDiagnostic>& collected, const std::string& filePath, const std::string& content) {
    std::vector<LspDiagnostic> converted;
    // Split the analyzed text once instead of scanning it per diagnostic.
    std::vector<std::string> contentLines;
    {
        std::string current;
        for (char ch : content) {
            if (ch == '\n') {
                contentLines.push_back(current);
                current.clear();
            } else if (ch != '\r') {
                current += ch;
            }
        }
        contentLines.push_back(current);
    }
    for (auto& diagnostic : collected) {
        LspDiagnostic out;
        std::string diagFile = diagnostic.location.file ? diagnostic.location.file : filePath;
        out.filePath = diagFile;
        std::string lineText;
        if (diagFile == filePath) {
            if (diagnostic.location.line >= 1 && diagnostic.location.line <= static_cast<int>(contentLines.size())) {
                lineText = contentLines[diagnostic.location.line - 1];
            }
        } else {
            lineText = readLineFromDisk(diagFile, diagnostic.location.line);
        }
        if (diagnostic.location.isValid()) {
            out.range = locationToRange(diagnostic.location, lineText);
        } else {
            out.range.start = {0, 0};
            out.range.end = {0, 1};
            out.filePath = filePath;
        }
        out.severity = diagnostic.severity == "warning" ? 2 : 1;
        out.message = diagnostic.message;
        out.relatedNotes = diagnostic.notes;
        converted.push_back(std::move(out));
    }
    return converted;
}

SymbolInfo findAt(Module* mainModule, const std::string& filePath, LspPosition pos) {
    SymbolInfo empty;
    if (!mainModule) return empty;
    Finder finder{filePath, pos.line, pos.character, {}, -1};
    // Also search imported modules.
    for (auto* module : allModules(mainModule)) {
        for (auto& sourceFile : module->sourceFiles) {
            for (auto* decl : sourceFile.topLevelDecls) {
                finder.visitDecl(decl, 0);
            }
        }
    }
    return finder.best;
}

std::string hoverAt(Module* mainModule, const std::string& filePath, LspPosition pos) {
    SymbolInfo found = findAt(mainModule, filePath, pos);
    if (!found.decl) return "";
    std::string signature = hoverForDecl(*found.decl);
    if (signature.empty()) return "";
    std::ostringstream out;
    out << "```cx\n" << signature << "\n```\n";
    out << "*" << declKindLabel(*found.decl) << "*";
    Location defLoc = found.decl->getLocation();
    if (defLoc.isValid() && defLoc.file) {
        out << " - defined at " << defLoc.file << ":" << defLoc.line << ":" << defLoc.column;
    }
    return out.str();
}

bool gotoDefinitionAt(Module* mainModule, const std::string& filePath, LspPosition pos, std::string& outFilePath, LspRange& outRange) {
    SymbolInfo found = findAt(mainModule, filePath, pos);
    if (!found.decl) return false;
    Location loc = found.decl->getLocation();
    if (!loc.isValid() || !loc.file) return false;
    outFilePath = loc.file;
    outRange = nameRange(loc, found.decl->getName().size());
    return true;
}

std::vector<CompletionItem> completeAt(Module* mainModule, const std::string& filePath, LspPosition pos) {
    std::vector<CompletionItem> items;
    static const char* keywords[] = {"break",  "case",   "const",  "continue", "default", "defer",     "else",      "enum",    "extern",
                                     "false",  "for",    "if",     "import",   "in",      "interface", "null",      "private", "public",
                                     "return", "sizeof", "struct", "switch",   "this",    "true",      "undefined", "var",     "while"};
    for (auto* kw : keywords)
        items.push_back({kw, "keyword", "keyword"});

    if (!mainModule) return items;

    // Collect visible top-level declarations (main + imports).
    auto addDecl = [&](Decl* decl) {
        if (!decl || decl->getName().empty()) return;
        std::string name = decl->getName().str();
        for (auto& existing : items) {
            if (existing.label == name) return;
        }
        CompletionItem item;
        item.label = name;
        if (decl->isFunctionDecl() || decl->isFunctionTemplate()) {
            item.kind = "function";
            if (auto* fn = llvm::dyn_cast<FunctionDecl>(decl))
                item.detail = formatFunctionSignature(*fn);
            else if (auto* tmpl = llvm::dyn_cast<FunctionTemplate>(decl))
                item.detail = formatFunctionSignature(*tmpl->functionDecl);
        } else if (decl->isTypeDecl() || decl->isTypeTemplate()) {
            item.kind = "type";
            item.detail = hoverForDecl(*decl);
        } else if (decl->isVariableDecl()) {
            item.kind = "variable";
            item.detail = hoverForDecl(*decl);
        } else {
            item.kind = "variable";
        }
        items.push_back(std::move(item));
    };

    for (auto* module : allModules(mainModule)) {
        for (auto& sourceFile : module->sourceFiles) {
            for (auto* decl : sourceFile.topLevelDecls)
                addDecl(decl);
        }
    }

    // Add enclosing locals/params: find innermost function containing the cursor
    // in this file and collect variable declarations before the cursor.
    // (Best-effort: walk all function bodies and collect VarDecls/ParamDecls
    // whose definition is in the same file on an earlier line.)
    struct LocalCollector {
        const std::string& file;
        int line;
        std::vector<Decl*> locals;
        void visitExpr(Expr* expr) {
            if (!expr) return;
            switch (expr->kind) {
            case ExprKind::CallExpr:
            case ExprKind::UnaryExpr:
            case ExprKind::BinaryExpr:
            case ExprKind::IndexExpr:
            case ExprKind::IndexAssignmentExpr: {
                auto* call = llvm::cast<CallExpr>(expr);
                visitExpr(call->callee);
                for (auto& arg : call->args)
                    visitExpr(arg.value);
                return;
            }
            case ExprKind::ArrayLiteralExpr:
                for (auto* el : llvm::cast<ArrayLiteralExpr>(expr)->elements)
                    visitExpr(el);
                return;
            case ExprKind::TupleExpr:
                for (auto& el : llvm::cast<TupleExpr>(expr)->elements)
                    visitExpr(el.value);
                return;
            case ExprKind::UnwrapExpr:
                visitExpr(llvm::cast<UnwrapExpr>(expr)->operand);
                return;
            case ExprKind::LambdaExpr:
                if (auto* fn = llvm::cast<LambdaExpr>(expr)->functionDecl) visitDecl(fn);
                return;
            case ExprKind::IfExpr: {
                auto* ifExpr = llvm::cast<IfExpr>(expr);
                visitExpr(ifExpr->condition);
                visitExpr(ifExpr->thenExpr);
                visitExpr(ifExpr->elseExpr);
                return;
            }
            case ExprKind::ImplicitCastExpr:
                visitExpr(llvm::cast<ImplicitCastExpr>(expr)->operand);
                return;
            case ExprKind::VarDeclExpr:
                visitDecl(llvm::cast<VarDeclExpr>(expr)->varDecl);
                return;
            case ExprKind::MemberExpr:
                visitExpr(llvm::cast<MemberExpr>(expr)->base);
                return;
            default:
                return;
            }
        }
        void visitStmt(Stmt* stmt) {
            if (!stmt) return;
            switch (stmt->kind) {
            case StmtKind::ReturnStmt:
                visitExpr(llvm::cast<ReturnStmt>(stmt)->value);
                return;
            case StmtKind::VarStmt:
                visitDecl(llvm::cast<VarStmt>(stmt)->decl);
                return;
            case StmtKind::ExprStmt:
                visitExpr(llvm::cast<ExprStmt>(stmt)->expr);
                return;
            case StmtKind::DeferStmt:
                visitExpr(llvm::cast<DeferStmt>(stmt)->expr);
                return;
            case StmtKind::IfStmt: {
                auto* ifStmt = llvm::cast<IfStmt>(stmt);
                visitExpr(ifStmt->condition);
                for (auto* s : ifStmt->thenBody)
                    visitStmt(s);
                for (auto* s : ifStmt->elseBody)
                    visitStmt(s);
                return;
            }
            case StmtKind::SwitchStmt: {
                auto* switchStmt = llvm::cast<SwitchStmt>(stmt);
                visitExpr(switchStmt->condition);
                for (auto& c : switchStmt->cases) {
                    visitExpr(c.value);
                    if (c.associatedValue) visitDecl(c.associatedValue);
                    for (auto* s : c.stmts)
                        visitStmt(s);
                }
                for (auto* s : switchStmt->defaultStmts)
                    visitStmt(s);
                return;
            }
            case StmtKind::WhileStmt: {
                auto* whileStmt = llvm::cast<WhileStmt>(stmt);
                visitExpr(whileStmt->condition);
                for (auto* s : whileStmt->body)
                    visitStmt(s);
                return;
            }
            case StmtKind::ForStmt: {
                auto* forStmt = llvm::cast<ForStmt>(stmt);
                if (forStmt->variable) visitStmt(forStmt->variable);
                visitExpr(forStmt->condition);
                visitExpr(forStmt->increment);
                for (auto* s : forStmt->body)
                    visitStmt(s);
                return;
            }
            case StmtKind::ForEachStmt: {
                auto* forEach = llvm::cast<ForEachStmt>(stmt);
                if (forEach->variable) visitDecl(forEach->variable);
                visitExpr(forEach->range);
                for (auto* s : forEach->body)
                    visitStmt(s);
                return;
            }
            case StmtKind::CompoundStmt:
                for (auto* s : llvm::cast<CompoundStmt>(stmt)->body)
                    visitStmt(s);
                return;
            default:
                return;
            }
        }
        void visitDecl(Decl* decl) {
            if (!decl) return;
            if (auto* var = llvm::dyn_cast<VarDecl>(decl)) {
                Location loc = var->getLocation();
                if (loc.file && file == loc.file && loc.isValid() && loc.line - 1 <= line) locals.push_back(decl);
                if (var->initializer) visitExpr(var->initializer);
                return;
            }
            if (auto* fn = llvm::dyn_cast<FunctionDecl>(decl)) {
                for (auto& param : fn->getParams()) {
                    Location loc = param.getLocation();
                    if (loc.file && file == loc.file && loc.isValid() && loc.line - 1 <= line) locals.push_back(&param);
                }
                if (fn->body) {
                    for (auto* s : *fn->body)
                        visitStmt(s);
                }
                return;
            }
            if (auto* tmpl = llvm::dyn_cast<FunctionTemplate>(decl)) {
                visitDecl(tmpl->functionDecl);
                return;
            }
            // Recurse into type methods so completion inside a method body
            // sees its params and locals.
            if (auto* typeDecl = llvm::dyn_cast<TypeDecl>(decl)) {
                for (auto* method : typeDecl->methods)
                    visitDecl(method);
                return;
            }
            if (auto* tmpl = llvm::dyn_cast<TypeTemplate>(decl)) {
                visitDecl(tmpl->typeDecl);
                return;
            }
        }
    };

    // Only collect from functions in the main module to avoid duplicates.
    // (Imported locals are out of scope by definition.)
    if (mainModule) {
        // Find functions whose range plausibly contains the cursor is complex
        // without end locations; instead collect all file-local variables
        // defined on earlier lines (over-approximation, deduped below).
        LocalCollector collector{filePath, pos.line, {}};
        for (auto& sourceFile : mainModule->sourceFiles) {
            for (auto* decl : sourceFile.topLevelDecls)
                collector.visitDecl(decl);
        }
        for (auto* local : collector.locals) {
            std::string name = local->getName().str();
            if (name.empty()) continue;
            bool exists = false;
            for (auto& existing : items) {
                if (existing.label == name) {
                    exists = true;
                    break;
                }
            }
            if (!exists) items.push_back({name, "variable", hoverForDecl(*local)});
        }
    }

    llvm::sort(items, [](const CompletionItem& a, const CompletionItem& b) { return a.label < b.label; });
    return items;
}

std::vector<DocumentSymbol> documentSymbolsIn(Module* mainModule, const std::string& filePath) {
    std::vector<DocumentSymbol> symbols;
    if (!mainModule) return symbols;
    for (auto& sourceFile : mainModule->sourceFiles) {
        for (auto* decl : sourceFile.topLevelDecls) {
            Location loc = decl->getLocation();
            if (!loc.isValid() || !loc.file || filePath != loc.file) continue;
            DocumentSymbol symbol;
            symbol.name = decl->getName().str();
            if (symbol.name.empty()) continue;
            if (decl->isFunctionDecl() || decl->isFunctionTemplate())
                symbol.kind = "function";
            else if (decl->isEnumDecl())
                symbol.kind = "enum";
            else if (decl->isTypeDecl() || decl->isTypeTemplate())
                symbol.kind = "struct";
            else if (decl->isVariableDecl())
                symbol.kind = "variable";
            else
                symbol.kind = "variable";
            symbol.selectionRange = nameRange(loc, symbol.name.size());
            symbol.range = symbol.selectionRange;
            symbols.push_back(std::move(symbol));
            // Add members for types.
            if (auto* typeDecl = llvm::dyn_cast<TypeDecl>(decl)) {
                for (auto& field : typeDecl->fields) {
                    Location fieldLoc = field.getLocation();
                    if (!fieldLoc.isValid() || !fieldLoc.file || filePath != fieldLoc.file) continue;
                    DocumentSymbol member;
                    member.name = field.getName().str();
                    member.kind = "field";
                    member.selectionRange = nameRange(fieldLoc, member.name.size());
                    member.range = member.selectionRange;
                    symbols.push_back(std::move(member));
                }
                for (auto* method : typeDecl->methods) {
                    Location methodLoc = method->getLocation();
                    if (!methodLoc.isValid() || !methodLoc.file || filePath != methodLoc.file) continue;
                    DocumentSymbol member;
                    member.name = method->getName().str();
                    member.kind = "method";
                    member.selectionRange = nameRange(methodLoc, member.name.size());
                    member.range = member.selectionRange;
                    symbols.push_back(std::move(member));
                }
                if (auto* enumDecl = llvm::dyn_cast<EnumDecl>(decl)) {
                    for (auto& enumCase : enumDecl->cases) {
                        Location caseLoc = enumCase.getLocation();
                        if (!caseLoc.isValid() || !caseLoc.file || filePath != caseLoc.file) continue;
                        DocumentSymbol member;
                        member.name = enumCase.getName().str();
                        member.kind = "enumMember";
                        member.selectionRange = nameRange(caseLoc, member.name.size());
                        member.range = member.selectionRange;
                        symbols.push_back(std::move(member));
                    }
                }
            }
        }
    }
    return symbols;
}

std::vector<std::pair<std::string, LspRange>> referencesTo(Module* mainModule, const std::string& filePath, LspPosition pos) {
    std::vector<std::pair<std::string, LspRange>> result;
    SymbolInfo found = findAt(mainModule, filePath, pos);
    if (!found.decl) return result;
    ReferenceCollector collector{found.decl, {}};
    auto collectFromModule = [&](Module& module) {
        for (auto& sourceFile : module.sourceFiles) {
            for (auto* decl : sourceFile.topLevelDecls)
                collector.visitDecl(decl);
        }
    };
    if (mainModule) collectFromModule(*mainModule);
    for (auto* module : allModules(mainModule))
        if (module != mainModule) collectFromModule(*module);
    for (auto& [loc, len] : collector.locations) {
        if (!loc.isValid() || !loc.file) continue;
        result.emplace_back(loc.file, nameRange(loc, len));
    }
    return result;
}

const std::vector<std::string>& semanticTokenTypes() {
    static const std::vector<std::string> types = {
        "comment",   "string",        "number",    "keyword",  "macro",    "type",       "struct",   "enum",
        "interface", "typeParameter", "parameter", "variable", "property", "enumMember", "function", "method",
    };
    return types;
}

const std::vector<std::string>& semanticTokenModifiers() {
    static const std::vector<std::string> modifiers = {"definition"};
    return modifiers;
}

std::vector<SemanticToken> semanticTokensIn(Module* mainModule, const std::string& filePath, const std::string& content) {
    std::vector<SemanticToken> tokens;
    collectSyntaxTokens(content, tokens);
    if (mainModule) {
        std::vector<std::string> lines;
        std::string current;
        for (char ch : content) {
            if (ch == '\n') {
                lines.push_back(current);
                current.clear();
            } else if (ch != '\r') {
                current += ch;
            }
        }
        lines.push_back(current);
        SemanticCollector collector{filePath, lines, tokens, {}};
        for (auto& sourceFile : mainModule->sourceFiles) {
            for (auto* decl : sourceFile.topLevelDecls)
                collector.visitDecl(decl);
        }
    }
    // Stable by (line, start): definitions precede references at the same span,
    // otherwise insertion order wins (real decls precede synthesized ones).
    std::stable_sort(tokens.begin(), tokens.end(), [](const SemanticToken& a, const SemanticToken& b) {
        if (a.line != b.line) return a.line < b.line;
        if (a.start != b.start) return a.start < b.start;
        return a.definition > b.definition;
    });
    // Templates visit the inner decl at the same span as the wrapper, and
    // synthesized nodes share spans with real ones; keep the first token per
    // span so the output never overlaps.
    std::vector<SemanticToken> kept;
    kept.reserve(tokens.size());
    int keptLine = -1;
    int keptEnd = 0;
    for (auto& token : tokens) {
        if (token.line == keptLine && token.start < keptEnd) continue;
        kept.push_back(token);
        keptLine = token.line;
        keptEnd = token.start + token.length;
    }
    return kept;
}

LspQuery parseLspQuery(const JsonValue& queryJson) {
    LspQuery query;
    if (!queryJson.getAsObject()) throw JsonParseError("query must be a JSON object");
    query.method = getJsonString(queryJson, "method");
    if (query.method.empty()) throw JsonParseError("query is missing \"method\"");
    query.filePath = getJsonString(queryJson, "file");
    if (query.filePath.empty()) throw JsonParseError("query is missing \"file\"");
    query.content = getJsonString(queryJson, "content");
    if (auto* docs = findJson(queryJson, "openDocs")) {
        auto* docsObj = docs->getAsObject();
        if (!docsObj) throw JsonParseError("\"openDocs\" must be an object");
        for (auto& [path, text] : *docsObj) {
            auto textStr = text.getAsString();
            if (!textStr) throw JsonParseError("\"openDocs\" values must be strings");
            query.openDocs[path] = textStr->str();
        }
    }
    auto getStringArray = [](const JsonValue* value, const char* name) {
        std::vector<std::string> out;
        if (!value) return out;
        auto* array = value->getAsArray();
        if (!array) throw JsonParseError(std::string("\"") + name + "\" must be an array");
        for (auto& item : *array) {
            auto str = item.getAsString();
            if (!str) throw JsonParseError(std::string("\"") + name + "\" entries must be strings");
            out.push_back(str->str());
        }
        return out;
    };
    query.workspaceFolders = getStringArray(findJson(queryJson, "workspaceFolders"), "workspaceFolders");
    query.importSearchPaths = getStringArray(findJson(queryJson, "importSearchPaths"), "importSearchPaths");
    query.defines = getStringArray(findJson(queryJson, "defines"), "defines");
    if (auto* pos = findJson(queryJson, "position")) {
        if (!pos->getAsObject()) throw JsonParseError("\"position\" must be an object");
        query.position.line = static_cast<int>(getJsonInt(*pos, "line"));
        query.position.character = static_cast<int>(getJsonInt(*pos, "character"));
    }
    return query;
}

namespace {

JsonValue rangeToJson(const LspRange& range) {
    JsonObject start;
    start["line"] = range.start.line;
    start["character"] = range.start.character;
    JsonObject end;
    end["line"] = range.end.line;
    end["character"] = range.end.character;
    JsonObject out;
    out["start"] = std::move(start);
    out["end"] = std::move(end);
    return JsonValue(std::move(out));
}

JsonValue diagnosticsToJson(const std::vector<LspDiagnostic>& diagnostics) {
    JsonArray items;
    for (auto& diagnostic : diagnostics) {
        JsonObject item;
        item["file"] = diagnostic.filePath;
        item["range"] = rangeToJson(diagnostic.range);
        item["severity"] = diagnostic.severity;
        item["message"] = diagnostic.message;
        JsonArray notes;
        for (auto& note : diagnostic.relatedNotes) {
            JsonObject noteJson;
            if (note.location.isValid() && note.location.file) {
                JsonObject location;
                location["uri"] = pathToUri(note.location.file);
                location["range"] = rangeToJson(nameRange(note.location, 1));
                noteJson["location"] = std::move(location);
            }
            noteJson["message"] = note.message;
            notes.push_back(std::move(noteJson));
        }
        item["relatedInformation"] = std::move(notes);
        items.push_back(std::move(item));
    }
    return JsonValue(std::move(items));
}

} // namespace

JsonValue handleQuery(const JsonValue& queryJson) {
    LspQuery query = parseLspQuery(queryJson);
    FrontendResult frontend = runFrontendOnce(query);
    std::vector<LspDiagnostic> diagnostics = toLspDiagnostics(frontend.diagnostics, frontend.filePath, frontend.content);

    JsonObject result;
    result["diagnostics"] = diagnosticsToJson(diagnostics);

    if (query.method == "check") {
        return JsonValue(std::move(result));
    } else if (query.method == "hover") {
        std::string text = hoverAt(frontend.mainModule, query.filePath, query.position);
        result["hover"] = text;
        return JsonValue(std::move(result));
    } else if (query.method == "definition") {
        std::string targetFile;
        LspRange targetRange;
        if (gotoDefinitionAt(frontend.mainModule, query.filePath, query.position, targetFile, targetRange)) {
            result["found"] = true;
            result["file"] = targetFile;
            result["range"] = rangeToJson(targetRange);
        } else {
            result["found"] = false;
        }
        return JsonValue(std::move(result));
    } else if (query.method == "completion") {
        JsonArray items;
        for (auto& item : completeAt(frontend.mainModule, query.filePath, query.position)) {
            JsonObject entry;
            entry["label"] = item.label;
            entry["kind"] = item.kind;
            entry["detail"] = item.detail;
            items.push_back(std::move(entry));
        }
        result["items"] = std::move(items);
        return JsonValue(std::move(result));
    } else if (query.method == "documentSymbol") {
        JsonArray items;
        for (auto& symbol : documentSymbolsIn(frontend.mainModule, query.filePath)) {
            JsonObject entry;
            entry["name"] = symbol.name;
            entry["kind"] = symbol.kind;
            entry["range"] = rangeToJson(symbol.range);
            entry["selectionRange"] = rangeToJson(symbol.selectionRange);
            items.push_back(std::move(entry));
        }
        result["symbols"] = std::move(items);
        return JsonValue(std::move(result));
    } else if (query.method == "references") {
        JsonArray items;
        for (auto& [file, range] : referencesTo(frontend.mainModule, query.filePath, query.position)) {
            JsonObject entry;
            entry["file"] = file;
            entry["range"] = rangeToJson(range);
            items.push_back(std::move(entry));
        }
        result["references"] = std::move(items);
        return JsonValue(std::move(result));
    } else if (query.method == "semanticTokens") {
        JsonArray items;
        for (auto& token : semanticTokensIn(frontend.mainModule, query.filePath, frontend.content)) {
            JsonObject entry;
            entry["line"] = token.line;
            entry["start"] = token.start;
            entry["length"] = token.length;
            entry["type"] = token.type;
            JsonArray modifiers;
            if (token.definition) modifiers.push_back("definition");
            entry["modifiers"] = std::move(modifiers);
            items.push_back(std::move(entry));
        }
        result["tokens"] = std::move(items);
        return JsonValue(std::move(result));
    }
    throw JsonParseError("unknown query method: " + query.method);
}

} // namespace cx::lsp
