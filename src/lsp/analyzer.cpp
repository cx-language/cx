#include "analyzer.h"
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <memory>
#include <optional>
#include <sstream>
#pragma warning(push, 0)
#include <llvm/ADT/STLExtras.h>
#include <llvm/ADT/ScopeExit.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/Process.h>
#include <llvm/Support/SaveAndRestore.h>
#pragma warning(pop)
#include "../ast/arena.h"
#include "../ast/decl.h"
#include "../ast/expr.h"
#include "../ast/module.h"
#include "../ast/stmt.h"
#include "../ast/type.h"
#include "../backend/irgen.h"
#include "../build/config.h"
#include "../build/dependencies.h"
#include "../driver/driver.h"
#include "../parser/parse.h"
#include "../sema/c-import.h"
#include "../sema/null-analyzer.h"
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

/// Every build.cx met walking up from parentDir, innermost first. Stat-only:
/// safe to call without a diagnostic collector (unlike BuildConfig, which
/// parses and aborts the process on malformed files without one).
std::vector<std::string> collectUpwardBuildFiles(const std::string& parentDir) {
    std::vector<std::string> paths;
    if (parentDir.empty()) return paths;
    std::string dir = parentDir;
    while (true) {
        std::string buildFilePath = dir + "/" + BuildConfig::buildFileName;
        bool isFile = false;
        if (!llvm::sys::fs::is_regular_file(buildFilePath, isFile) && isFile) {
            paths.push_back(buildFilePath);
        }
        llvm::StringRef parent = llvm::sys::path::parent_path(dir);
        if (parent == dir) return paths; // Filesystem root reached.
        dir = parent.str();
    }
}

/// Finds the build root governing filePath by walking up from parentDir to the
/// outermost directory whose build.cx target roots contain the file, or nullopt
/// if the file stands alone. Outermost wins to mirror `cx build`, which runs at
/// the project root and compiles nested build.cx files as ordinary sources;
/// dependencies are still resolved via import search paths, never fetched.
/// When found, buildDir receives the directory holding that build.cx file.
/// checkedBuildFiles (when given) receives every build.cx met on the way up,
/// innermost first, for cache validation.
std::optional<std::string> findBuildRoot(const std::string& filePath, const std::string& parentDir, std::string* buildDir = nullptr,
                                         const std::vector<std::string>& defines = {}, std::vector<std::string>* checkedBuildFiles = nullptr) {
    std::optional<std::string> outermost;
    for (auto& buildFilePath : collectUpwardBuildFiles(parentDir)) {
        if (checkedBuildFiles) checkedBuildFiles->push_back(buildFilePath);
        std::string dir = llvm::sys::path::parent_path(buildFilePath).str();
        BuildConfig config{std::string(dir), defines};
        for (auto& root : config.getTargetRootDirectories()) {
            // Either separator: file paths may use backslashes on Windows
            // while roots built from URIs use forward slashes.
            if (filePath == root || llvm::StringRef(filePath).starts_with(root + "/") || llvm::StringRef(filePath).starts_with(root + "\\")) {
                outermost = root;
                if (buildDir) *buildDir = dir;
                break;
            }
        }
    }
    return outermost;
}

/// True when parentDir is an importable `<searchPath>/std` directory (editing
/// inside the standard library itself). Stat-only, like collectUpwardBuildFiles.
bool isStdDirectory(const std::string& parentDir, const std::vector<std::string>& importSearchPaths) {
    for (llvm::StringRef searchPath : importSearchPaths) {
        auto candidate = (searchPath + "/std").str();
        if (llvm::sys::fs::is_directory(candidate) && llvm::sys::fs::equivalent(candidate, parentDir)) {
            return true;
        }
    }
    return false;
}

/// Search paths package detection runs against: the base paths plus the
/// vendor/ directories runFrontendOnce adds below (a package typically
/// resolves through the build root's vendor/). Header-search and
/// dependency paths need parsed build configs, so they stay out: detection
/// is stat-only for cache validation. Must mirror runFrontendOnce's
/// vendor appends.
std::vector<std::string> withVendorPaths(const std::vector<std::string>& basePaths, const std::optional<std::string>& moduleDir, const std::string& buildDir) {
    std::vector<std::string> paths = basePaths;
    if (!buildDir.empty()) paths.push_back(buildDir + "/vendor");
    if (moduleDir) {
        std::string moduleVendor = *moduleDir + "/vendor";
        if (buildDir.empty() || moduleVendor != buildDir + "/vendor") paths.push_back(std::move(moduleVendor));
    }
    return paths;
}

/// A directory importable as `<searchPath>/<name>`.
struct ImportablePackage {
    std::string dir;
    std::string name;
};

/// Nearest importable ancestor of parentDir (starting with itself),
/// mirroring importModule's first-hit-wins lookup: a match loses when an
/// earlier search path holds a different directory under the same name.
/// Generalizes isStdDirectory: "std" is just the package found this way
/// inside the standard library. Stat-only for cache validation.
std::optional<ImportablePackage> findImportablePackage(const std::string& parentDir, const std::vector<std::string>& importSearchPaths) {
    for (std::string dir = parentDir; !dir.empty(); dir = llvm::sys::path::parent_path(dir).str()) {
        std::string parent = llvm::sys::path::parent_path(dir).str();
        if (parent.empty() || parent == dir) break;
        for (size_t i = 0; i < importSearchPaths.size(); ++i) {
            if (!llvm::sys::fs::equivalent(parent, importSearchPaths[i])) continue;
            std::string name = llvm::sys::path::filename(dir).str();
            for (size_t j = 0; j < i; ++j) {
                std::string shadowed = importSearchPaths[j] + "/" + name;
                if (llvm::sys::fs::is_directory(shadowed) && !llvm::sys::fs::equivalent(shadowed, dir)) {
                    return std::nullopt;
                }
            }
            return ImportablePackage{dir, name};
        }
    }
    return std::nullopt;
}

/// Sorted .cx siblings under moduleDir, excluding filePath itself, vendored
/// packages (which join via `import`), and the project root's build.cx (which
/// is config, not source; a build.cx anywhere else is an ordinary source).
/// Stat-only directory walk, no parsing.
std::vector<std::string> enumerateSiblingPaths(const std::string& moduleDir, const std::string& exclusionRoot, const std::string& filePath) {
    std::vector<std::string> siblingPaths;
    std::error_code ec;
    for (llvm::sys::fs::recursive_directory_iterator it(moduleDir, ec), end; it != end && !ec; it.increment(ec)) {
        if (llvm::sys::path::extension(it->path()) == ".cx" && it->path() != filePath && !isVendoredPath(it->path())
            && !isRootBuildFile(it->path(), exclusionRoot)) {
            siblingPaths.push_back(it->path());
        }
    }
    llvm::sort(siblingPaths);
    return siblingPaths;
}

/// Appends the .cx files an `import` of packageDir would load (mirroring
/// importModuleSourcesInDirectoryRecursively's file set), minus the open
/// file, which joins from its buffer instead.
void appendPackagePaths(std::vector<std::string>& out, const std::string& packageDir, const std::string& filePath) {
    std::error_code ec;
    for (llvm::sys::fs::recursive_directory_iterator it(packageDir, ec), end; it != end && !ec; it.increment(ec)) {
        if (llvm::sys::path::extension(it->path()) == ".cx" && it->path() != filePath && !isRootBuildFile(it->path(), packageDir)) {
            out.push_back(it->path());
        }
    }
}

/// Sorted .cx siblings for the main module: the moduleDir files plus, when
/// the open file sits in an importable package, that package's own files
/// (which join here instead of via `import`, so the import resolves to
/// this module). Deduplicated: the two sets can overlap.
std::vector<std::string> buildSiblingPaths(const std::optional<std::string>& moduleDir, const std::string& buildDir, const std::string& filePath,
                                           const std::optional<std::string>& packageDir) {
    std::vector<std::string> siblings;
    if (moduleDir) {
        std::string exclusionRoot = buildDir.empty() ? *moduleDir : buildDir;
        siblings = enumerateSiblingPaths(*moduleDir, exclusionRoot, filePath);
    }
    if (packageDir) appendPackagePaths(siblings, *packageDir, filePath);
    llvm::sort(siblings);
    siblings.erase(llvm::unique(siblings), siblings.end());
    return siblings;
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
    case DeclKind::TypeAliasDecl:
        return "type alias";
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

static std::string displayName(const FunctionDecl& decl) {
    std::string result;
    if (decl.getTypeDecl()) {
        result = decl.getTypeDecl()->getType().getDisplayName();
        result += '.';
    }
    result += decl.getName();
    if (!decl.genericArgs.empty()) {
        result = getDisplayTypeName(result, decl.genericArgs);
    }
    return result;
}

static std::string displayName(const TypeDecl& decl) {
    return getDisplayTypeName(decl.getName(), decl.genericArgs);
}

static std::string displayName(const FieldDecl& decl) {
    return (displayName(llvm::cast<TypeDecl>(*decl.getParentDecl())) + "." + decl.getName()).str();
}

std::string formatFunctionSignature(const FunctionDecl& decl) {
    std::ostringstream out;
    out << decl.getReturnType().toString() << " " << displayName(decl) << "(";
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
        out << displayName(typeDecl);
        break;
    }
    case DeclKind::TypeTemplate: {
        auto& tmpl = llvm::cast<TypeTemplate>(decl);
        out << (tmpl.typeDecl->isUnion() ? "union " : "struct ") << tmpl.getName().str() << "<...>";
        break;
    }
    case DeclKind::TypeAliasDecl: {
        auto& alias = llvm::cast<TypeAliasDecl>(decl);
        out << "using " << alias.getName() << " = " << alias.aliasedType;
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
        out << (field.type ? field.type.toString() + " " : "") << displayName(field);
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

template<typename Visitor>
concept HasOnType = requires(Visitor& visitor, Type type) { visitor.onType(type); };
template<typename Visitor>
concept HasOnVarExpr = requires(Visitor& visitor, VarExpr* expr) { visitor.onVarExpr(expr); };
template<typename Visitor>
concept HasOnMemberExpr = requires(Visitor& visitor, MemberExpr* expr) { visitor.onMemberExpr(expr); };
template<typename Visitor>
concept HasOnCallExpr = requires(Visitor& visitor, CallExpr* expr) { visitor.onCallExpr(expr); };
template<typename Visitor>
concept HasOnCallGenericArgs = requires(Visitor& visitor, CallExpr* expr) { visitor.onCallGenericArgs(expr); };
template<typename Visitor>
concept HasOnUnwrapExpr = requires(Visitor& visitor, CallExpr* expr) { visitor.onUnwrapExpr(expr); };
template<typename Visitor>
concept HasOnSizeof = requires(Visitor& visitor, SizeofExpr* expr) { visitor.onSizeof(expr); };

// Shared by the cursor, reference, highlight, and completion walks. Each
// visitor supplies visitExpr/visitStmt/visitDecl (and visitType, for types)
// plus the hooks it cares about; everything else is the tree shape.
template<typename Visitor> void walkType(Visitor& visitor, Type type) {
    if (!type) return;
    if constexpr (HasOnType<Visitor>) visitor.onType(type);
    switch (type.getKind()) {
    case TypeKind::BasicType:
        // Covers Array<T, N>: element is a generic arg.
        for (GenericArg arg : type.getGenericArgs())
            if (arg.isType()) visitor.visitType(arg.getType());
        break;
    case TypeKind::ArrayPointerType:
        visitor.visitType(type.getElementType());
        break;
    case TypeKind::AnonymousStructType:
        for (auto& element : type.getAnonymousStructElements())
            visitor.visitType(element.type);
        break;
    case TypeKind::FunctionType:
        visitor.visitType(type.getReturnType());
        for (Type param : type.getParamTypes())
            visitor.visitType(param);
        break;
    case TypeKind::PointerType:
        visitor.visitType(type.getPointee());
        break;
    case TypeKind::UnresolvedType:
        break;
    }
}

template<typename Visitor> void walkExpr(Visitor& visitor, Expr* expr) {
    if (!expr) return;
    switch (expr->kind) {
    case ExprKind::VarExpr:
        if constexpr (HasOnVarExpr<Visitor>) visitor.onVarExpr(llvm::cast<VarExpr>(expr));
        return;
    case ExprKind::MemberExpr: {
        auto* member = llvm::cast<MemberExpr>(expr);
        visitor.visitExpr(member->base);
        if constexpr (HasOnMemberExpr<Visitor>) visitor.onMemberExpr(member);
        return;
    }
    case ExprKind::CallExpr:
    case ExprKind::UnaryExpr:
    case ExprKind::BinaryExpr:
    case ExprKind::IndexExpr:
    case ExprKind::IndexAssignmentExpr: {
        // Unary, binary, and index expressions are CallExpr subclasses.
        auto* call = llvm::cast<CallExpr>(expr);
        visitor.visitExpr(call->callee);
        if constexpr (HasOnCallExpr<Visitor>) visitor.onCallExpr(call);
        for (auto& arg : call->args)
            visitor.visitExpr(arg.value);
        if constexpr (HasOnCallGenericArgs<Visitor>) visitor.onCallGenericArgs(call);
        return;
    }
    case ExprKind::ArrayLiteralExpr:
        for (auto* element : llvm::cast<ArrayLiteralExpr>(expr)->elements)
            visitor.visitExpr(element);
        return;
    case ExprKind::AnonymousStructExpr:
        for (auto& element : llvm::cast<AnonymousStructExpr>(expr)->elements)
            visitor.visitExpr(element.value);
        return;
    case ExprKind::UnwrapExpr: {
        auto* call = llvm::cast<CallExpr>(expr);
        // The callee is a synthesized 'unwrap' member; only the receiver is source.
        visitor.visitExpr(call->getReceiver());
        if constexpr (HasOnUnwrapExpr<Visitor>) visitor.onUnwrapExpr(call);
        return;
    }
    case ExprKind::LambdaExpr:
        if (auto* function = llvm::cast<LambdaExpr>(expr)->functionDecl) visitor.visitDecl(function);
        return;
    case ExprKind::IfExpr: {
        auto* ifExpr = llvm::cast<IfExpr>(expr);
        visitor.visitExpr(ifExpr->condition);
        visitor.visitExpr(ifExpr->thenExpr);
        visitor.visitExpr(ifExpr->elseExpr);
        return;
    }
    case ExprKind::SwitchExpr: {
        auto* switchExpr = llvm::cast<SwitchExpr>(expr);
        visitor.visitExpr(switchExpr->condition);
        for (auto& arm : switchExpr->arms) {
            visitor.visitExpr(arm.value);
            if (arm.associatedValue) visitor.visitDecl(arm.associatedValue);
            visitor.visitExpr(arm.expr);
        }
        if (switchExpr->defaultExpr) visitor.visitExpr(switchExpr->defaultExpr);
        return;
    }
    case ExprKind::ImplicitCastExpr:
        visitor.visitExpr(llvm::cast<ImplicitCastExpr>(expr)->operand);
        return;
    case ExprKind::VarDeclExpr:
        visitor.visitDecl(llvm::cast<VarDeclExpr>(expr)->varDecl);
        return;
    case ExprKind::SizeofExpr:
        if constexpr (HasOnSizeof<Visitor>) visitor.onSizeof(llvm::cast<SizeofExpr>(expr));
        return;
    case ExprKind::StringLiteralExpr:
    case ExprKind::CharacterLiteralExpr:
    case ExprKind::IntLiteralExpr:
    case ExprKind::FloatLiteralExpr:
    case ExprKind::BoolLiteralExpr:
    case ExprKind::NullLiteralExpr:
    case ExprKind::UndefinedLiteralExpr:
        return;
    }
}

template<typename Visitor> void walkStmt(Visitor& visitor, Stmt* stmt) {
    if (!stmt) return;
    switch (stmt->kind) {
    case StmtKind::ReturnStmt:
        visitor.visitExpr(llvm::cast<ReturnStmt>(stmt)->value);
        return;
    case StmtKind::VarStmt:
        for (auto* decl : llvm::cast<VarStmt>(stmt)->decls)
            visitor.visitDecl(decl);
        return;
    case StmtKind::ExprStmt:
        visitor.visitExpr(llvm::cast<ExprStmt>(stmt)->expr);
        return;
    case StmtKind::DeferStmt:
        visitor.visitExpr(llvm::cast<DeferStmt>(stmt)->expr);
        return;
    case StmtKind::IfStmt: {
        auto* ifStmt = llvm::cast<IfStmt>(stmt);
        visitor.visitExpr(ifStmt->condition);
        for (auto* child : ifStmt->thenBody)
            visitor.visitStmt(child);
        for (auto* child : ifStmt->elseBody)
            visitor.visitStmt(child);
        return;
    }
    case StmtKind::SwitchStmt: {
        auto* switchStmt = llvm::cast<SwitchStmt>(stmt);
        visitor.visitExpr(switchStmt->condition);
        for (auto& switchCase : switchStmt->cases) {
            visitor.visitExpr(switchCase.value);
            if (switchCase.associatedValue) visitor.visitDecl(switchCase.associatedValue);
            for (auto* child : switchCase.stmts)
                visitor.visitStmt(child);
        }
        for (auto* child : switchStmt->defaultStmts)
            visitor.visitStmt(child);
        return;
    }
    case StmtKind::WhileStmt: {
        auto* whileStmt = llvm::cast<WhileStmt>(stmt);
        visitor.visitExpr(whileStmt->condition);
        for (auto* child : whileStmt->body)
            visitor.visitStmt(child);
        return;
    }
    case StmtKind::DoWhileStmt: {
        auto* doWhileStmt = llvm::cast<DoWhileStmt>(stmt);
        visitor.visitExpr(doWhileStmt->condition);
        for (auto* child : doWhileStmt->body)
            visitor.visitStmt(child);
        return;
    }
    case StmtKind::ForStmt: {
        auto* forStmt = llvm::cast<ForStmt>(stmt);
        if (forStmt->variable) visitor.visitStmt(forStmt->variable);
        visitor.visitExpr(forStmt->condition);
        for (auto* increment : forStmt->increments)
            visitor.visitExpr(increment);
        for (auto* child : forStmt->body)
            visitor.visitStmt(child);
        return;
    }
    case StmtKind::ForEachStmt: {
        auto* forEach = llvm::cast<ForEachStmt>(stmt);
        if (forEach->variable) visitor.visitDecl(forEach->variable);
        if (forEach->indexVariable) visitor.visitDecl(forEach->indexVariable);
        visitor.visitExpr(forEach->range);
        for (auto* child : forEach->body)
            visitor.visitStmt(child);
        return;
    }
    case StmtKind::BreakStmt:
    case StmtKind::ContinueStmt:
        return;
    case StmtKind::CompoundStmt:
        for (auto* child : llvm::cast<CompoundStmt>(stmt)->body)
            visitor.visitStmt(child);
        return;
    }
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
        struct Walker {
            Finder& finder;
            int depth;
            void visitType(Type nested) { finder.visitType(nested, depth + 1); }
            void onType(Type type) {
                if (TypeDecl* typeDecl = type.getDecl()) {
                    // Approximate the source span with the printed spelling; it
                    // matches the source in the common cases (`Point`, `const Point`,
                    // `Point?`, `int[10]`). Generic arguments are visited below at a
                    // deeper level so they win over the outer name.
                    finder.consider(typeDecl, type.location, type.toString().size(), nullptr, false, depth);
                }
            }
        } walker{*this, depth};
        walkType(walker, type);
    }
};

void Finder::visitExpr(Expr* expr, int depth) {
    struct Walker {
        Finder& finder;
        int depth;
        void visitExpr(Expr* nested) { finder.visitExpr(nested, depth + 1); }
        void visitDecl(Decl* decl) { finder.visitDecl(decl, depth + 1); }
        void visitType(Type type) { finder.visitType(type, depth + 1); }
        void onVarExpr(VarExpr* var) {
            if (var->decl) finder.consider(var->decl, var->location, var->identifier.size(), var, false, depth);
        }
        void onMemberExpr(MemberExpr* member) {
            if (member->decl) finder.consider(member->decl, member->location, member->member.size(), member, false, depth);
        }
        void onCallExpr(CallExpr* call) {
            // Plain function calls resolve through CallExpr::calleeDecl; the
            // callee VarExpr itself is never typechecked so its decl stays null.
            if (!call->calleeDecl) return;
            if (auto* var = llvm::dyn_cast<VarExpr>(call->callee)) {
                finder.consider(call->calleeDecl, var->location, var->identifier.size(), call->callee, false, depth + 1);
            } else if (auto* member = llvm::dyn_cast<MemberExpr>(call->callee)) {
                finder.consider(call->calleeDecl, member->location, member->member.size(), call->callee, false, depth + 1);
            }
        }
        void onUnwrapExpr(CallExpr* call) {
            // The callee is a synthesized 'unwrap' member at the '!' location; only the '!' itself is real source.
            if (call->calleeDecl) finder.consider(call->calleeDecl, call->location, 1, call, false, depth + 1);
        }
        void onSizeof(SizeofExpr* sizeofExpr) { visitType(sizeofExpr->operandType); }
    } walker{*this, depth};
    walkExpr(walker, expr);
}

void Finder::visitStmt(Stmt* stmt, int depth) {
    struct Walker {
        Finder& finder;
        int depth;
        void visitExpr(Expr* expr) { finder.visitExpr(expr, depth); }
        void visitStmt(Stmt* nested) { finder.visitStmt(nested, depth); }
        void visitDecl(Decl* decl) { finder.visitDecl(decl, depth); }
    } walker{*this, depth + 1};
    walkStmt(walker, stmt);
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
        // Bodies resolve per-instantiation; the clones keep the original
        // locations, so visit them to make definition and hover work inside
        // generic bodies. The template itself goes last so it wins ties.
        for (auto& [args, instantiation] : tmpl->instantiations)
            visitDecl(instantiation, depth + 1);
        visitDecl(tmpl->functionDecl, depth + 1);
        return;
    }
    case DeclKind::TypeDecl: {
        auto* typeDecl = llvm::cast<TypeDecl>(decl);
        for (Type interface : typeDecl->interfaces)
            visitType(interface, depth + 1);
        for (GenericArg arg : typeDecl->genericArgs)
            if (arg.isType()) visitType(arg.getType(), depth + 1);
        for (auto& field : typeDecl->fields) {
            consider(&field, field.getLocation(), field.getName().size(), nullptr, true, depth + 1);
            visitType(field.type, depth + 1);
            if (field.defaultValue) visitExpr(field.defaultValue, depth + 2);
        }
        for (auto* method : typeDecl->methods)
            visitDecl(method, depth + 1);
        return;
    }
    case DeclKind::TypeTemplate: {
        auto* tmpl = llvm::cast<TypeTemplate>(decl);
        for (auto& [args, instantiation] : tmpl->instantiations)
            visitDecl(instantiation, depth + 1);
        visitDecl(tmpl->typeDecl, depth + 1);
        return;
    }
    case DeclKind::TypeAliasDecl:
        visitType(llvm::cast<TypeAliasDecl>(decl)->aliasedType, depth + 1);
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
    void onType(Type type) {
        if (type.getDecl() == target) locations.emplace_back(type.location, type.toString().size());
    }
    void visitType(Type type) { walkType(*this, type); }
    void onVarExpr(VarExpr* var) {
        if (var->decl == target) locations.emplace_back(var->location, var->identifier.size());
    }
    void onMemberExpr(MemberExpr* member) {
        if (member->decl == target) locations.emplace_back(member->location, member->member.size());
    }
    void onCallExpr(CallExpr* call) {
        // See Finder: plain calls resolve through calleeDecl, leaving the
        // callee VarExpr's decl null.
        if (call->calleeDecl != target) return;
        if (auto* var = llvm::dyn_cast<VarExpr>(call->callee)) {
            locations.emplace_back(var->location, var->identifier.size());
        } else if (auto* member = llvm::dyn_cast<MemberExpr>(call->callee)) {
            locations.emplace_back(member->location, member->member.size());
        }
    }
    void onUnwrapExpr(CallExpr* call) {
        // The callee is a synthesized 'unwrap' member at the '!' location; only the '!' itself is real source.
        if (call->calleeDecl == target) locations.emplace_back(call->location, 1);
    }
    void onSizeof(SizeofExpr* expr) { visitType(expr->operandType); }
};

void ReferenceCollector::visitExpr(Expr* expr) {
    walkExpr(*this, expr);
}

void ReferenceCollector::visitStmt(Stmt* stmt) {
    walkStmt(*this, stmt);
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
        for (GenericArg arg : typeDecl->genericArgs)
            if (arg.isType()) visitType(arg.getType());
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
    case DeclKind::TypeAliasDecl:
        visitType(llvm::cast<TypeAliasDecl>(decl)->aliasedType);
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
    case DeclKind::TypeAliasDecl:
        return "type";
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

/// True for immutable local bindings (`const` variables and parameters), which
/// editors render without the mutable styling.
bool isReadonlyVariable(const Decl& decl) {
    if (auto* var = llvm::dyn_cast<VarDecl>(&decl)) return var->isConst;
    if (auto* param = llvm::dyn_cast<ParamDecl>(&decl)) return param->isConst;
    return false;
}

/// Tolerant scanner for syntax-only tokens (comments, strings, numbers,
/// keywords). Deliberately independent of Lexer: it never reports errors or
/// throws, so half-typed code still highlights something sane. Identifiers
/// are left to the AST pass, which knows their semantic type. Interpolated
/// `{...}` interiors recurse as code (nesting included).
struct SyntaxScanner {
    const std::string& content;
    std::vector<SemanticToken>& out;
    size_t i = 0;
    int line = 0;
    int col = 0;

    void emitToken(int tokenLine, int start, int length, const char* type) {
        if (length <= 0) return;
        SemanticToken token;
        token.line = tokenLine;
        token.start = start;
        token.length = length;
        token.type = type;
        out.push_back(std::move(token));
    }

    // Scans normal code. With stopAtBrace, `{` nests and the `}` closing the
    // interpolation ends the scan (left for the caller to consume). Braces
    // inside nested strings and comments never surface: those branches
    // consume through their terminators.
    void scanContent(bool stopAtBrace);

    // Scans a `"` string from its opening quote, splitting base chunks around
    // `{...}` interpolations. Delimiters highlight as keywords: the legend has
    // no punctuation type, and keywords render distinctly from strings in every
    // theme. Mirrors lex.cpp's trigger rule: every unescaped `{` interpolates.
    void scanString();
};

void SyntaxScanner::scanContent(bool stopAtBrace) {
    // Mirrors the keyword table in lex.cpp; hash-directives highlight as macros.
    static const llvm::StringMap<const char*> keywords = {
        {"break", "keyword"},    {"case", "keyword"},   {"const", "keyword"},     {"continue", "keyword"},  {"default", "keyword"}, {"defer", "keyword"},
        {"else", "keyword"},     {"enum", "keyword"},   {"extern", "keyword"},    {"false", "keyword"},     {"for", "keyword"},     {"if", "keyword"},
        {"implicit", "keyword"}, {"import", "keyword"}, {"in", "keyword"},        {"interface", "keyword"}, {"is", "keyword"},      {"null", "keyword"},
        {"private", "keyword"},  {"public", "keyword"}, {"return", "keyword"},    {"sizeof", "keyword"},    {"struct", "keyword"},  {"switch", "keyword"},
        {"this", "keyword"},     {"true", "keyword"},   {"undefined", "keyword"}, {"union", "keyword"},     {"using", "keyword"},   {"var", "keyword"},
        {"while", "keyword"},    {"#if", "macro"},      {"#else", "macro"},       {"#endif", "macro"},
    };

    int braceDepth = 0;
    while (i < content.size()) {
        char ch = content[i];
        if (stopAtBrace && ch == '{') {
            ++braceDepth;
            ++i;
            ++col;
            continue;
        }
        if (stopAtBrace && ch == '}') {
            if (braceDepth == 0) return;
            --braceDepth;
            ++i;
            ++col;
            continue;
        }
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
            emitToken(line, start, col - start, "comment");
            continue;
        }
        if (ch == '/' && i + 1 < content.size() && content[i + 1] == '*') {
            // Nested block comment, split per line for the single-line LSP encoding.
            int nest = 0;
            int segLine = line;
            int segStart = col;
            while (i < content.size()) {
                if (content[i] == '\n') {
                    emitToken(segLine, segStart, col - segStart, "comment");
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
            emitToken(segLine, segStart, col - segStart, "comment");
            continue;
        }
        if (ch == '"') {
            scanString();
            continue;
        }
        if (ch == '\'') {
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
                if (c == '\'') break;
            }
            emitToken(line, start, col - start, "string");
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
            emitToken(line, start, col - start, "number");
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
            if (it != keywords.end()) emitToken(line, start, col - start, it->second);
            continue;
        }
        ++i;
        ++col;
    }
}

void SyntaxScanner::scanString() {
    int segStart = col;
    int segLine = line;
    ++i; // Consume the opening quote.
    ++col;
    while (i < content.size()) {
        char c = content[i];
        if (c == '\n' || c == '\r') break; // Unterminated: highlight to end of line.
        if (c == '\\' && i + 1 < content.size() && content[i + 1] != '\n' && content[i + 1] != '\r') {
            i += 2;
            col += 2;
            continue;
        }
        if (c == '"') {
            ++i;
            ++col;
            break;
        }
        if (c == '{') {
            emitToken(segLine, segStart, col - segStart, "string");
            emitToken(line, col, 1, "keyword");
            ++i;
            ++col;
            scanContent(true);
            if (i < content.size() && content[i] == '}') {
                emitToken(line, col, 1, "keyword");
                ++i;
                ++col;
            }
            segStart = col;
            segLine = line;
            continue;
        }
        ++i;
        ++col;
    }
    emitToken(segLine, segStart, col - segStart, "string");
}

void collectSyntaxTokens(const std::string& content, std::vector<SemanticToken>& out) {
    SyntaxScanner scanner{content, out};
    scanner.scanContent(false);
}

/// Emits one highlight token per declaration and reference in the target file.
/// Traversal mirrors Finder; only filePath locations are kept.
struct SemanticCollector {
    const std::string& file;
    const std::vector<std::string>& lines;
    std::vector<SemanticToken>& out;
    std::vector<std::string> genericParamNames; // In-scope generic parameters, for typeParameter uses.
    const llvm::StringSet<>* aliasNames = nullptr; // Visible type aliases, for alias-spelled type uses.

    void emit(const Location& loc, llvm::StringRef name, const char* type, bool definition, bool readonly = false) {
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
        token.readonly = readonly;
        out.push_back(std::move(token));
    }

    void emitDecl(Decl* decl, bool definition) {
        if (!decl) return;
        if (const char* type = tokenTypeForDecl(*decl)) emit(decl->getLocation(), decl->getName(), type, definition, isReadonlyVariable(*decl));
    }

    // The source may spell an alias ("int") where the resolved type has the
    // canonical name ("int32"); highlight the spelled name then. Only known
    // alias spellings qualify, so synthesized types at reused locations still
    // need an exact canonical match (see emit()).
    void emitTypeName(Type type, const char* tokenType) {
        const Location& loc = type.location;
        llvm::StringRef spelling = type.getName();
        if (aliasNames && loc.isValid() && loc.file && file == loc.file && loc.line >= 1 && loc.line - 1 < static_cast<int>(lines.size()) && loc.column >= 1) {
            const std::string& lineText = lines[loc.line - 1];
            size_t start = static_cast<size_t>(loc.column - 1);
            if (start <= lineText.size()) {
                size_t end = start;
                while (end < lineText.size() && (std::isalnum((unsigned char)lineText[end]) || lineText[end] == '_'))
                    end++;
                llvm::StringRef ident(lineText.data() + start, end - start);
                if (!ident.empty() && ident != spelling && aliasNames->contains(ident)) spelling = ident;
            }
        }
        emit(loc, spelling, tokenType, false);
    }

    void visitExpr(Expr* expr);
    void visitStmt(Stmt* stmt);
    void visitDecl(Decl* decl);
    void onVarExpr(VarExpr* var) {
        if (!var->decl) return;
        if (const char* type = tokenTypeForDecl(*var->decl)) emit(var->location, var->identifier, type, false, isReadonlyVariable(*var->decl));
    }
    void onMemberExpr(MemberExpr* member) {
        if (!member->decl) return;
        if (const char* type = tokenTypeForDecl(*member->decl)) emit(member->location, member->member, type, false, isReadonlyVariable(*member->decl));
    }
    void onCallExpr(CallExpr* call) {
        // See Finder: plain calls resolve through calleeDecl, leaving the
        // callee VarExpr's decl null.
        if (!call->calleeDecl) return;
        if (const char* type = tokenTypeForDecl(*call->calleeDecl)) {
            bool readonly = isReadonlyVariable(*call->calleeDecl);
            if (auto* var = llvm::dyn_cast<VarExpr>(call->callee)) {
                emit(var->location, var->identifier, type, false, readonly);
            } else if (auto* member = llvm::dyn_cast<MemberExpr>(call->callee)) {
                emit(member->location, member->member, type, false, readonly);
            }
        }
    }
    void onCallGenericArgs(CallExpr* call) {
        for (GenericArg arg : call->genericArgs)
            if (arg.isType()) visitType(arg.getType());
    }
    void onUnwrapExpr(CallExpr* call) {
        // The callee is a synthesized 'unwrap' member at the '!' location; only the '!' itself is real source.
        if (!call->calleeDecl) return;
        if (const char* type = tokenTypeForDecl(*call->calleeDecl)) {
            emit(call->location, "!", type, false, isReadonlyVariable(*call->calleeDecl));
        }
    }
    void onSizeof(SizeofExpr* expr) { visitType(expr->operandType); }
    void visitType(Type type) {
        if (!type) return;
        switch (type.getKind()) {
        case TypeKind::BasicType: {
            // Optional (`T?`) and Slice (`T[]`) wrappers are synthesized: the
            // location points at the sugar or is invalid, so only the wrapped
            // type highlights. The direct generic args (not getWrappedType())
            // preserve the inner locations.
            if ((type.isOptionalType() || type.isSlice()) && !type.getGenericArgs().empty()) {
                for (GenericArg arg : type.getGenericArgs())
                    if (arg.isType()) visitType(arg.getType());
                return;
            }
            if (TypeDecl* typeDecl = type.getDecl()) {
                if (const char* tokenType = tokenTypeForDecl(*typeDecl)) emitTypeName(type, tokenType);
            } else if (llvm::is_contained(genericParamNames, type.getName())) {
                emit(type.location, type.getName(), "typeParameter", false);
            } else {
                // Builtins without decls (`void`) and unresolved names.
                emit(type.location, type.getName(), "type", false);
            }
            for (GenericArg arg : type.getGenericArgs())
                if (arg.isType()) visitType(arg.getType());
            return;
        }
        case TypeKind::ArrayPointerType:
            visitType(type.getElementType());
            return;
        case TypeKind::AnonymousStructType:
            for (auto& element : llvm::cast<AnonymousStructType>(type.typeBase)->elements)
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
    walkExpr(*this, expr);
}

void SemanticCollector::visitStmt(Stmt* stmt) {
    walkStmt(*this, stmt);
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
        for (GenericArg arg : typeDecl->genericArgs)
            if (arg.isType()) visitType(arg.getType());
        for (auto& field : typeDecl->fields) {
            emitDecl(&field, true);
            visitType(field.type);
            if (field.defaultValue) visitExpr(field.defaultValue);
        }
        for (auto* method : typeDecl->methods)
            visitDecl(method);
        return;
    }
    case DeclKind::TypeAliasDecl:
        visitType(llvm::cast<TypeAliasDecl>(decl)->aliasedType);
        return;
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

constexpr const char* kCompletionPlaceholder = "cxLspCompletionPlaceholder";

std::string getLineText(const std::string& content, int line0) {
    size_t start = 0;
    for (int i = 0; i < line0; ++i) {
        size_t nl = content.find('\n', start);
        if (nl == std::string::npos) return "";
        start = nl + 1;
    }
    size_t end = content.find('\n', start);
    std::string line = content.substr(start, end == std::string::npos ? end : end - start);
    if (!line.empty() && line.back() == '\r') line.pop_back();
    return line;
}

size_t getLineStartOffset(const std::string& content, int line0) {
    size_t start = 0;
    for (int i = 0; i < line0; ++i) {
        size_t nl = content.find('\n', start);
        if (nl == std::string::npos) return std::string::npos;
        start = nl + 1;
    }
    return start;
}

std::optional<int> findMemberDot(const std::string& line, int cursorChar) {
    if (cursorChar < 0 || cursorChar > static_cast<int>(line.size())) return std::nullopt;
    int i = cursorChar;
    while (i > 0 && (std::isalnum(static_cast<unsigned char>(line[i - 1])) || line[i - 1] == '_'))
        --i;
    int afterDot = i;
    while (afterDot > 0 && (line[afterDot - 1] == ' ' || line[afterDot - 1] == '\t'))
        --afterDot;
    if (afterDot <= 0 || line[afterDot - 1] != '.') return std::nullopt;
    int dot = afterDot - 1;
    if (dot > 0 && line[dot - 1] == '.') return std::nullopt;
    if (dot + 1 < static_cast<int>(line.size()) && line[dot + 1] == '.') return std::nullopt;
    if (dot > 0 && dot + 1 < static_cast<int>(line.size()) && std::isdigit(static_cast<unsigned char>(line[dot - 1]))
        && std::isdigit(static_cast<unsigned char>(line[dot + 1]))) {
        return std::nullopt;
    }
    return dot;
}

bool isMemberCompletionContext(const std::string& content, LspPosition pos) {
    return findMemberDot(getLineText(content, pos.line), pos.character).has_value();
}

bool needsCompletionPlaceholder(const std::string& content, LspPosition pos) {
    std::string line = getLineText(content, pos.line);
    if (!findMemberDot(line, pos.character)) return false;
    int i = pos.character;
    int identEnd = i;
    while (i > 0 && (std::isalnum(static_cast<unsigned char>(line[i - 1])) || line[i - 1] == '_'))
        --i;
    if (i < identEnd) return false;
    if (pos.character < static_cast<int>(line.size())) {
        char next = line[pos.character];
        if (std::isalpha(static_cast<unsigned char>(next)) || next == '_') return false;
    }
    return true;
}

std::string insertCompletionPlaceholder(const std::string& content, LspPosition pos) {
    size_t lineStart = getLineStartOffset(content, pos.line);
    if (lineStart == std::string::npos) return content;
    size_t lineEnd = content.find('\n', lineStart);
    size_t lineLen = (lineEnd == std::string::npos ? content.size() : lineEnd) - lineStart;
    if (lineLen > 0 && content[lineStart + lineLen - 1] == '\r') --lineLen;
    int ch = std::max(0, std::min(pos.character, static_cast<int>(lineLen)));
    std::string out = content;
    out.insert(lineStart + ch, kCompletionPlaceholder);
    return out;
}

struct MemberExprCollector {
    std::vector<MemberExpr*> members;
    void onMemberExpr(MemberExpr* member) { members.push_back(member); }
    void visitExpr(Expr* expr) { walkExpr(*this, expr); }
    void visitStmt(Stmt* stmt) { walkStmt(*this, stmt); }
    void visitDecl(Decl* decl) {
        if (!decl) return;
        switch (decl->kind) {
        case DeclKind::FunctionDecl:
        case DeclKind::MethodDecl:
        case DeclKind::ConstructorDecl:
        case DeclKind::DestructorDecl: {
            auto* fn = llvm::cast<FunctionDecl>(decl);
            for (auto& param : fn->getParams()) {
                if (param.defaultValue) visitExpr(param.defaultValue);
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
            for (auto& field : typeDecl->fields) {
                if (field.defaultValue) visitExpr(field.defaultValue);
            }
            for (auto* method : typeDecl->methods)
                visitDecl(method);
            return;
        }
        case DeclKind::TypeAliasDecl: {
            if (TypeDecl* typeDecl = llvm::cast<TypeAliasDecl>(decl)->aliasedType.getDecl()) {
                visitDecl(typeDecl);
            }
            return;
        }
        case DeclKind::TypeTemplate:
            visitDecl(llvm::cast<TypeTemplate>(decl)->typeDecl);
            return;
        case DeclKind::EnumDecl: {
            auto* enumDecl = llvm::cast<EnumDecl>(decl);
            for (auto& c : enumDecl->cases) {
                if (c.value) visitExpr(c.value);
            }
            return;
        }
        case DeclKind::VarDecl: {
            auto* var = llvm::cast<VarDecl>(decl);
            if (var->initializer) visitExpr(var->initializer);
            return;
        }
        case DeclKind::FieldDecl: {
            auto* field = llvm::cast<FieldDecl>(decl);
            if (field->defaultValue) visitExpr(field->defaultValue);
            return;
        }
        case DeclKind::ParamDecl: {
            auto* param = llvm::cast<ParamDecl>(decl);
            if (param->defaultValue) visitExpr(param->defaultValue);
            return;
        }
        default:
            return;
        }
    }
};

MemberExpr* findPlaceholderMember(Module* mainModule) {
    if (!mainModule) return nullptr;
    MemberExprCollector collector;
    for (auto* module : allModules(mainModule)) {
        for (auto& sourceFile : module->sourceFiles) {
            for (auto* decl : sourceFile.topLevelDecls)
                collector.visitDecl(decl);
        }
    }
    for (auto* member : collector.members) {
        if (member->member == kCompletionPlaceholder) return member;
    }
    return nullptr;
}

MemberExpr* findMemberAt(Module* mainModule, const std::string& filePath, LspPosition pos) {
    if (!mainModule) return nullptr;
    MemberExprCollector collector;
    for (auto* module : allModules(mainModule)) {
        for (auto& sourceFile : module->sourceFiles) {
            for (auto* decl : sourceFile.topLevelDecls)
                collector.visitDecl(decl);
        }
    }
    MemberExpr* best = nullptr;
    for (auto* member : collector.members) {
        Location loc = member->location;
        if (!loc.isValid() || !loc.file || filePath != loc.file) continue;
        if (loc.line - 1 != pos.line) continue;
        int start = loc.column - 1;
        int end = start + static_cast<int>(member->member.size());
        if (pos.character >= start && pos.character <= end) {
            if (!best || start > best->location.column - 1) best = member;
        }
    }
    return best;
}

// Splits file buffers into lines once and reuses the split for consecutive
// same-file doc lookups (completion visits declarations file by file).
struct DocLineCache {
    const llvm::MemoryBuffer* buffer = nullptr;
    llvm::SmallVector<llvm::StringRef, 128> lines;
};

// Returns the /// doc comment above the given 1-based line, or "". Only
// consecutive /// lines directly above the declaration count; a //// line
// is a plain comment (like Rust) and breaks the block, as does a blank line.
// Single newlines fold into spaces (soft wraps); blank /// lines separate
// paragraphs.
static std::string extractDocComment(llvm::ArrayRef<llvm::StringRef> lines, int declLine) {
    if (declLine < 1 || declLine - 1 > static_cast<int>(lines.size())) return "";
    std::vector<llvm::StringRef> docLines;
    for (int i = declLine - 2; i >= 0; --i) {
        llvm::StringRef line = lines[i].rtrim("\r");
        size_t indent = line.find_first_not_of(" \t");
        if (indent == llvm::StringRef::npos) break;
        line = line.drop_front(indent);
        if (!line.starts_with("///")) break;
        line = line.drop_front(3);
        if (line.starts_with("/")) break;
        if (line.starts_with(" ")) line = line.drop_front(1);
        line = line.rtrim(" \t");
        docLines.push_back(line);
    }
    std::string doc, paragraph;
    auto flushParagraph = [&] {
        if (paragraph.empty()) return;
        if (!doc.empty()) doc += "\n\n";
        doc += paragraph;
        paragraph.clear();
    };
    for (auto it = docLines.rbegin(); it != docLines.rend(); ++it) {
        if (it->empty()) {
            flushParagraph();
        } else {
            if (!paragraph.empty()) paragraph += ' ';
            paragraph += *it;
        }
    }
    flushParagraph();
    return doc;
}

// Returns the /// doc comment above the given declaration, or "" when it
// has none. Reads the compiled file buffer, so open-document overlays are
// reflected without disk I/O.
static std::string docCommentForDecl(const Decl& decl, DocLineCache& cache) {
    Location loc = decl.getLocation();
    if (!loc.isValid() || !loc.file) return "";
    Module* module = decl.getModule();
    if (!module) {
        // Parameters have no module of their own; their file is their function's.
        if (auto* var = llvm::dyn_cast<VariableDecl>(&decl); var && var->parent) module = var->parent->getModule();
    }
    if (!module) return "";
    for (auto& buffer : module->fileBuffers) {
        if (buffer->getBufferIdentifier() == loc.file) {
            if (cache.buffer != buffer.get()) {
                cache.buffer = buffer.get();
                cache.lines.clear();
                buffer->getBuffer().split(cache.lines, '\n');
            }
            return extractDocComment(cache.lines, loc.line);
        }
    }
    return "";
}

std::vector<CompletionItem> membersForEnumCases(EnumDecl* enumDecl, DocLineCache& cache) {
    std::vector<CompletionItem> out;
    if (!enumDecl) return out;
    for (auto& c : enumDecl->cases) {
        if (c.getName().empty()) continue;
        CompletionItem item;
        item.label = c.getName().str();
        item.kind = "enumMember";
        item.detail = hoverForDecl(c);
        item.documentation = docCommentForDecl(c, cache);
        out.push_back(std::move(item));
    }
    return out;
}

bool declHasParams(Decl* decl) {
    FunctionDecl* fn = nullptr;
    if (auto* function = llvm::dyn_cast<FunctionDecl>(decl))
        fn = function;
    else if (auto* tmpl = llvm::dyn_cast<FunctionTemplate>(decl))
        fn = tmpl->functionDecl;
    return fn && !fn->getParams().empty();
}

std::vector<CompletionItem> membersForType(Type type, DocLineCache& cache) {
    std::vector<CompletionItem> out;
    if (!type) return out;
    Type t = type.removeOptional().removePointer();
    if (t.isAnonymousStructType()) {
        for (auto& el : t.getAnonymousStructElements()) {
            CompletionItem item;
            item.label = el.name.str();
            item.kind = "field";
            item.detail = ((el.type ? el.type.toString() + " " : "") + el.name).str();
            out.push_back(std::move(item));
        }
        return out;
    }
    TypeDecl* decl = t.getDecl();
    if (!decl && t.isFixedArray()) {
        if (auto* std = Module::getStdlibModule()) {
            if (auto* array = llvm::dyn_cast_or_null<TypeTemplate>(std->symbolTable.findOne("Array"))) {
                decl = array->instantiate(t.getGenericArgs());
            }
        }
    }
    if (!decl && t.isArrayPointer()) {
        // Array pointers have no declaration; their only method is the
        // compiler-known data() identity.
        CompletionItem item;
        item.label = "data";
        item.kind = "method";
        out.push_back(std::move(item));
        return out;
    }
    if (!decl || decl->isEnumDecl()) return out;
    for (auto& field : decl->fields) {
        if (field.getName().empty()) continue;
        CompletionItem item;
        item.label = field.getName().str();
        item.kind = "field";
        item.detail = hoverForDecl(field);
        item.documentation = docCommentForDecl(field, cache);
        out.push_back(std::move(item));
    }
    for (auto* method : decl->methods) {
        if (!method || method->getName().empty()) continue;
        if (llvm::isa<ConstructorDecl>(method)) continue;
        if (method->getName().starts_with("[")) continue;
        CompletionItem item;
        item.label = method->getName().str();
        item.kind = "method";
        if (auto* fn = llvm::dyn_cast<FunctionDecl>(method)) {
            item.detail = formatFunctionSignature(*fn);
        } else if (auto* tmpl = llvm::dyn_cast<FunctionTemplate>(method)) {
            if (tmpl->functionDecl) item.detail = formatFunctionSignature(*tmpl->functionDecl);
        }
        item.hasParams = declHasParams(method);
        item.documentation = docCommentForDecl(*method, cache);
        // Overloads share the label, so dedupe by signature: identical
        // entries (e.g. an interface default plus an identical override)
        // collapse, distinct overloads are all listed.
        bool exists = false;
        for (auto& e : out) {
            if (e.label == item.label && e.detail == item.detail) {
                exists = true;
                break;
            }
        }
        if (!exists) out.push_back(std::move(item));
    }
    return out;
}

Decl* findTopLevelDecl(Module* mainModule, llvm::StringRef name) {
    for (auto* module : allModules(mainModule)) {
        for (auto& sourceFile : module->sourceFiles) {
            for (auto* decl : sourceFile.topLevelDecls) {
                if (decl->getName() == name) return decl;
            }
        }
    }
    return nullptr;
}

std::vector<CompletionItem> completeMembersFor(MemberExpr* memberExpr, Module* mainModule, DocLineCache& cache) {
    if (!memberExpr || !memberExpr->base) return {};
    Expr* base = memberExpr->base;
    if (auto* var = llvm::dyn_cast<VarExpr>(base)) {
        Decl* decl = var->decl ? var->decl : findTopLevelDecl(mainModule, var->identifier);
        if (decl) {
            if (auto* enumDecl = llvm::dyn_cast<EnumDecl>(decl)) return membersForEnumCases(enumDecl, cache);
            if (auto* tmpl = llvm::dyn_cast<TypeTemplate>(decl)) {
                if (auto* enumTemplate = llvm::dyn_cast<EnumDecl>(tmpl->typeDecl)) return membersForEnumCases(enumTemplate, cache);
                return {};
            }
            auto* alias = llvm::dyn_cast_or_null<TypeAliasDecl>(decl);
            if (!alias && llvm::isa<TypeDecl>(decl)) {
                alias = llvm::dyn_cast_or_null<TypeAliasDecl>(findTopLevelDecl(mainModule, var->identifier));
            }
            if (alias) {
                if (auto* enumDecl = llvm::dyn_cast_or_null<EnumDecl>(base->type.getDecl())) {
                    return membersForEnumCases(enumDecl, cache);
                }
                return base->type ? membersForType(base->type, cache) : membersForType(alias->aliasedType, cache);
            }
            if (llvm::isa<TypeDecl>(decl)) return {};
            if (auto* varDecl = llvm::dyn_cast<VariableDecl>(decl)) {
                Type t = base->type ? base->type : varDecl->type;
                return membersForType(t, cache);
            }
            return {};
        }
        if (base->type) return membersForType(base->type, cache);
        return {};
    }
    if (base->type) return membersForType(base->type, cache);
    return {};
}

// Routes pkg-config compile flags (headers, defines, cflags) into options,
// like the driver; link-only parts are irrelevant for analysis. Failures
// degrade to no pkg-config paths rather than failing the query.
void addPkgConfigFlags(CompileOptions& options, llvm::ArrayRef<std::string> packages, llvm::StringRef rootDirectory) {
    auto split = queryPkgConfigFlags(packages);
    if (!split) return;
    for (auto& define : split->defines) {
        options.defines.push_back(define);
    }
    for (auto& path : split->headerSearchPaths) {
        options.importSearchPaths.push_back(absolutizePackagePath(rootDirectory, path));
    }
    for (auto& path : split->frameworkSearchPaths) {
        options.frameworkSearchPaths.push_back(absolutizePackagePath(rootDirectory, path));
    }
    for (auto& flag : split->cflags) {
        options.cflags.push_back(flag);
    }
}

} // namespace

namespace {

// Query-specific option building and module discovery. Only runFrontendOnce
// and the session validation below use these.

std::vector<std::string> buildSharedImportSearchPaths(const LspQuery& query) {
    // Shared search paths: workspace folders, then explicit extras, then the
    // distribution root (for std/) and system paths. The file's directory
    // joins the main module's options below, not dependencies'.
    std::vector<std::string> paths;
    for (auto& folder : query.workspaceFolders)
        paths.push_back(folder);
    for (auto& path : query.importSearchPaths)
        paths.push_back(path);
    if (auto rootDir = getCxRootDir(); !rootDir.empty()) {
        paths.push_back(rootDir + "/vendor");
        paths.push_back(std::move(rootDir));
    }
#ifdef CLANG_BUILTIN_INCLUDE_PATH
    paths.push_back(CLANG_BUILTIN_INCLUDE_PATH);
#endif
    paths.push_back("/usr/include");
    paths.push_back("/usr/local/include");
    // Same bonus search paths as `cx build` (see driver.cpp). Unlike the
    // driver, queries don't probe the external C compiler for its header
    // paths - C-header imports relying on those need explicit
    // initializationOptions.importSearchPaths.
    for (const char* name : {"CPATH", "C_INCLUDE_PATH", "INCLUDE"}) {
        if (auto pathsEnv = llvm::sys::Process::GetEnv(name)) {
            llvm::SmallVector<llvm::StringRef, 16> split;
            llvm::StringRef(*pathsEnv).split(split, llvm::sys::EnvPathSeparator, -1, false);
            for (llvm::StringRef path : split)
                paths.push_back(path.str());
        }
    }
    return paths;
}

std::vector<std::string> buildMainImportSearchPaths(const LspQuery& query, const std::string& parentDir) {
    // Import search paths: file's directory first, then the shared paths.
    // Vendored packages are imported by name (see driver.cpp).
    std::vector<std::string> paths = buildSharedImportSearchPaths(query);
    if (!parentDir.empty()) {
        paths.insert(paths.begin(), {parentDir, (llvm::StringRef(parentDir) + "/vendor").str()});
    }
    return paths;
}

std::vector<std::string> buildBaseDefines(const LspQuery& query) {
    // The language server always analyzes as Debug (matching baseOptions'
    // default below), with platform defines like the driver.
    std::vector<std::string> defines = query.defines;
    defines.push_back("Debug");
    defines.push_back("LeakCheck");
    auto platformOptions = getPlatformCompileOptions();
    for (auto& define : platformOptions.defines)
        defines.push_back(define);
    return defines;
}

ModuleLayout discoverModuleLayout(const std::string& filePath, const std::vector<std::string>& importSearchPaths, const std::vector<std::string>& defines) {
    // Determine which files form the open file's module: an importable
    // package (registered below so the import resolves to it), a
    // build.cx target root, or just the file itself when standalone.
    ModuleLayout layout;
    std::string parentDir = llvm::sys::path::parent_path(filePath).str();
    std::optional<std::string> packageDir;
    if (!parentDir.empty()) {
        if (isStdDirectory(parentDir, importSearchPaths)) {
            layout.moduleDir = parentDir;
            layout.checkedBuildFiles = collectUpwardBuildFiles(parentDir);
            layout.registerAsPackage = "std";
        } else {
            layout.moduleDir = findBuildRoot(filePath, parentDir, &layout.buildDir, defines, &layout.checkedBuildFiles);
            if (auto package = findImportablePackage(parentDir, withVendorPaths(importSearchPaths, layout.moduleDir, layout.buildDir))) {
                packageDir = package->dir;
                layout.registerAsPackage = package->name;
            }
        }
        layout.siblingPaths = buildSiblingPaths(layout.moduleDir, layout.buildDir, filePath, packageDir);
    }
    return layout;
}

} // namespace

void resetCompilerGlobals() {
    // Drop references first, then free the memory they point into. IR nodes
    // reference AST memory (type-cache keys, expression pointers), so they go
    // before the AST itself.
    resetIRState();
    Module::resetImportedModules();
    resetTypeInterning();
    resetAstArena();
    resetDiagnosticsState();
    resetLambdaNameCounter();
    resetTypecheckerCounters();
    resetCImportState();
}

FrontendResult runFrontendOnce(const LspQuery& query) {
    FrontendResult result;
    result.filePath = query.filePath;
    result.content = query.content;
    const std::string& filePath = query.filePath;
    const std::string& content = query.content;

    diagnosticOptions.disableWarnings = false;
    diagnosticOptions.warningsAsErrors = false;
    diagnosticOptions.errorLimit = 0; // unlimited; the editor gets everything collected so far

    llvm::SaveAndRestore saveCollector(diagnosticCollector, &result.diagnostics);

    // The session owns the module (deleted on evict); the one-shot query
    // process leaks it and exits instead. Owned locally until the end so
    // a failed compilation frees it instead of leaking per red edit.
    auto ownedModule = std::make_unique<Module>("main");
    Module* module = ownedModule.get();
    bool moduleRegisteredAsPackage = false;

    try {
        CompileOptions baseOptions;
        baseOptions.recoverParseErrors = true;
        baseOptions.defines = buildBaseDefines(query);
        // Platform settings (defines, SDK sysroot/frameworks), matching the
        // driver so C-header imports and #if platform branches resolve identically.
        auto platformOptions = getPlatformCompileOptions();
        for (auto& cflag : platformOptions.cflags)
            baseOptions.cflags.push_back(cflag);
        for (auto& path : platformOptions.frameworkSearchPaths)
            baseOptions.frameworkSearchPaths.push_back(path);
        baseOptions.importSearchPaths = buildSharedImportSearchPaths(query);
        std::string parentDir = llvm::sys::path::parent_path(filePath).str();

        CompileOptions options = baseOptions;
        options.importSearchPaths = buildMainImportSearchPaths(query, parentDir);

        ModuleLayout layout = discoverModuleLayout(filePath, options.importSearchPaths, baseOptions.defines);
        const std::string& buildDir = layout.buildDir;

        // The project build file contributes its settings to the main module;
        // dependencies bring theirs through the closure. pkg-config is queried
        // like in the driver so C-header imports resolve the same headers.
        BuildConfig projectConfig{buildDir.empty() ? std::string() : std::string(buildDir), baseOptions.defines};
        if (!buildDir.empty()) {
            for (auto& define : projectConfig.defines) {
                options.defines.push_back(define);
            }
            applyWarningSettings(options, projectConfig.warnings);
            // The project root's vendor/ holds importable packages. The file's
            // own directory contributes its vendor/ above; a multitarget file
            // under src/foo needs both (see driver.cpp addConfigBuildFlags).
            // Keep in sync with withVendorPaths (package detection).
            options.importSearchPaths.push_back(buildDir + "/vendor");
            for (auto& path : projectConfig.headerSearchPaths) {
                options.importSearchPaths.push_back(absolutizePackagePath(buildDir, path));
            }
            resolveDependencyClosure(projectConfig, baseOptions, /*fetchMissing=*/false);
            addPkgConfigFlags(options, projectConfig.pkgConfigDependencies, buildDir);
            for (auto& record : projectConfig.resolvedDependencies) {
                addPkgConfigFlags(record.options, record.pkgConfigDependencies, record.rootDirectory);
            }
        }

        if (layout.moduleDir) {
            std::string moduleVendor = *layout.moduleDir + "/vendor";
            if (buildDir.empty() || moduleVendor != buildDir + "/vendor") {
                options.importSearchPaths.push_back(std::move(moduleVendor));
            }
        }

        {
            PhaseTimer timer("lsp-parse-main");
            auto mainBuffer = llvm::MemoryBuffer::getMemBufferCopy(content, filePath);
            module->fileBuffers.push_back(std::move(mainBuffer));
            for (auto& sibling : layout.siblingPaths) {
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
        }

        {
            PhaseTimer timer("lsp-typecheck-main");
            // When the open file lives inside an importable package (the
            // standard library itself, or a vendored package), a sibling's
            // `import` of that package would otherwise load the same sources
            // a second time as a separate module, drowning the file in
            // ambiguous-reference errors. Analyze them as that package
            // instead by resolving the import to the main module. This
            // mirrors importModule's lookup order, so it only triggers when
            // the import would actually find this directory.
            if (layout.registerAsPackage) {
                Module::registerImportedModule(*layout.registerAsPackage, module);
                moduleRegisteredAsPackage = true;
            }

            Typechecker typechecker(options, buildDir.empty() ? nullptr : &projectConfig.resolvedDependencies);
            // No pre-pass over imported modules: sourceFile.importedModules
            // is only populated during typechecking, so the list is empty
            // here; imports resolve (and typecheck) from inside
            // typecheckModule instead.
            typechecker.typecheckModule(*module, options, true);
            typechecker.checkUnusedDecls(*module);
        }

        if (errors == 0) {
            // Null-safety warnings come from the IR, like in the driver (which
            // likewise skips IRGen once errors exist). The main module is
            // skipped below when it also serves as an imported package.
            PhaseTimer timer("lsp-irgen-nullcheck");
            IRGenerator irGenerator(options);
            for (auto* imported : Module::getAllImportedModules()) {
                if (imported != module) irGenerator.emitModule(*imported);
            }
            irGenerator.emitModule(*module);
            NullAnalyzer nullAnalyzer;
            for (auto* irModule : irGenerator.generatedModules) {
                nullAnalyzer.analyze(irModule);
            }
            resetIRState();
        }

        result.mainModule = module;
        result.layout = std::move(layout);
        for (auto& record : projectConfig.resolvedDependencies) {
            std::string depBuildFile = record.rootDirectory + "/" + BuildConfig::buildFileName;
            bool isFile = false;
            if (!llvm::sys::fs::is_regular_file(depBuildFile, isFile) && isFile) {
                result.depBuildFiles.push_back(depBuildFile);
            }
        }
        ownedModule.release();
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

    if (moduleRegisteredAsPackage && ownedModule) {
        // Compilation failed after registering the main module as a package:
        // the registry owns it now (deleteModules frees it); releasing keeps
        // the unique_ptr from freeing it out from under the registry.
        ownedModule.release();
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
            lineText = readLineFromFile(diagFile, diagnostic.location.line);
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
    // consider() only matches nodes in the cursor's file; skip the rest.
    for (auto* module : allModules(mainModule)) {
        for (auto& sourceFile : module->sourceFiles) {
            if (sourceFile.filePath != filePath) continue;
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
    out << signature << "\n";
    DocLineCache docCache;
    if (std::string doc = docCommentForDecl(*found.decl, docCache); !doc.empty()) {
        out << doc << "\n\n";
    }
    out << declKindLabel(*found.decl);
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

std::vector<CompletionItem> completeAt(Module* mainModule, const std::string& filePath, LspPosition pos, const std::string& content) {
    DocLineCache docCache;
    if (mainModule) {
        if (MemberExpr* placeholder = findPlaceholderMember(mainModule)) {
            auto members = completeMembersFor(placeholder, mainModule, docCache);
            llvm::sort(members, [](const CompletionItem& a, const CompletionItem& b) { return a.label < b.label; });
            return members;
        }
    }
    if (isMemberCompletionContext(content, pos)) {
        if (!mainModule) return {};
        if (MemberExpr* atPos = findMemberAt(mainModule, filePath, pos)) {
            auto members = completeMembersFor(atPos, mainModule, docCache);
            llvm::sort(members, [](const CompletionItem& a, const CompletionItem& b) { return a.label < b.label; });
            return members;
        }
        return {};
    }

    std::vector<CompletionItem> items;
    static const char* keywords[] = {"break",  "case",     "const",  "continue", "default",   "defer", "else",  "enum",    "extern", "false",  "for",
                                     "if",     "implicit", "import", "in",       "interface", "is",    "null",  "private", "public", "return", "sizeof",
                                     "struct", "switch",   "this",   "true",     "undefined", "union", "using", "var",     "while"};
    for (auto* kw : keywords)
        items.push_back({kw, "keyword", "keyword"});

    if (!mainModule) return items;

    // Collect visible top-level declarations (main + imports).
    auto addDecl = [&](Decl* decl) {
        if (!decl || decl->getName().empty()) return;
        // Test functions are run by the test runner, never called directly.
        if (auto* fn = llvm::dyn_cast<FunctionDecl>(decl)) {
            if (fn->isTest) return;
        } else if (auto* tmpl = llvm::dyn_cast<FunctionTemplate>(decl)) {
            if (tmpl->functionDecl->isTest) return;
        }
        std::string name = decl->getName().str();
        CompletionItem item;
        item.label = name;
        if (decl->isFunctionDecl() || decl->isFunctionTemplate()) {
            item.kind = "function";
            if (auto* fn = llvm::dyn_cast<FunctionDecl>(decl))
                item.detail = formatFunctionSignature(*fn);
            else if (auto* tmpl = llvm::dyn_cast<FunctionTemplate>(decl))
                item.detail = formatFunctionSignature(*tmpl->functionDecl);
            item.hasParams = declHasParams(decl);
        } else if (decl->isTypeDecl() || decl->isTypeTemplate() || decl->isTypeAliasDecl()) {
            item.kind = "type";
            item.detail = hoverForDecl(*decl);
        } else if (decl->isVariableDecl()) {
            item.kind = "variable";
            item.detail = hoverForDecl(*decl);
        } else {
            item.kind = "variable";
        }
        // Functions dedupe by signature so every overload is listed;
        // other declarations dedupe by name (first scope wins).
        for (auto& existing : items) {
            if (existing.label != name) continue;
            if (item.kind != "function" || existing.detail == item.detail) return;
        }
        item.documentation = docCommentForDecl(*decl, docCache);
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
        void visitExpr(Expr* expr) { walkExpr(*this, expr); }
        void visitStmt(Stmt* stmt) { walkStmt(*this, stmt); }
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
            if (!exists) {
                CompletionItem item{name, "variable", hoverForDecl(*local)};
                item.documentation = docCommentForDecl(*local, docCache);
                items.push_back(std::move(item));
            }
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
            else if (decl->isTypeAliasDecl())
                symbol.kind = "type";
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
    static const std::vector<std::string> modifiers = {"definition", "readonly"};
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
        llvm::StringSet<> aliasNames;
        for (auto* module : allModules(mainModule)) {
            for (auto& sourceFile : module->sourceFiles) {
                for (auto* decl : sourceFile.topLevelDecls) {
                    if (auto* alias = llvm::dyn_cast<TypeAliasDecl>(decl)) aliasNames.insert(alias->getName());
                }
            }
        }
        SemanticCollector collector{filePath, lines, tokens, {}, &aliasNames};
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

/// Answers one method from an already-compiled frontend: the session calls
/// it on cache hits, handleQuery on every one-shot run. Diagnostics in other
/// files re-read their lines from disk, so this does disk I/O per call.
JsonValue answerFromFrontend(const LspQuery& query, const FrontendResult& frontend) {
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
        for (auto& item : completeAt(frontend.mainModule, query.filePath, query.position, frontend.content)) {
            JsonObject entry;
            entry["label"] = item.label;
            entry["kind"] = item.kind;
            entry["detail"] = item.detail;
            entry["documentation"] = item.documentation;
            entry["hasParams"] = item.hasParams;
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
            if (token.readonly) modifiers.push_back("readonly");
            entry["modifiers"] = std::move(modifiers);
            items.push_back(std::move(entry));
        }
        result["tokens"] = std::move(items);
        return JsonValue(std::move(result));
    }
    throw JsonParseError("unknown query method: " + query.method);
}

// Stats one on-disk dependency. Nullopt when the file is gone (a change like any other).
std::optional<FileStat> statDependency(const std::string& path) {
    llvm::sys::fs::file_status status;
    if (llvm::sys::fs::status(path, status)) return std::nullopt;
    FileStat stat;
    stat.path = path;
    stat.mtimeNanos = std::chrono::duration_cast<std::chrono::nanoseconds>(status.getLastModificationTime().time_since_epoch()).count();
    stat.size = status.getSize();
    return stat;
}

bool depStatsMatch(const std::vector<FileStat>& stats) {
    for (auto& stat : stats) {
        auto current = statDependency(stat.path);
        if (!current || *current != stat) return false;
    }
    return true;
}

// Every on-disk input the compilation read: build files plus main-module and
// import sources (whose file list comes from the compiled modules, so
// modified or removed inputs invalidate; only newly added import files and
// transitive C-header includes are missed - editing the open file refreshes
// those). Nullopt when an input vanished mid-compile; the entry is
// uncacheable then.
std::optional<std::vector<FileStat>> collectDepStats(const LspQuery& query, const ModuleLayout& layout, const std::vector<std::string>& depBuildFiles,
                                                     Module* mainModule) {
    std::vector<FileStat> stats;
    auto statOne = [&](const std::string& path) {
        if (auto stat = statDependency(path)) {
            stats.push_back(std::move(*stat));
            return true;
        }
        return false;
    };
    for (auto& path : layout.checkedBuildFiles) {
        if (!statOne(path)) return std::nullopt;
    }
    for (auto& path : depBuildFiles) {
        if (!statOne(path)) return std::nullopt;
    }
    // Siblings come from the layout, not the compiled modules: an unreadable
    // sibling is absent from sourceFiles but still affects compilation once
    // readable. Overlaid siblings come from the editor instead (compared via
    // relevantOpenDocs).
    for (auto& sibling : layout.siblingPaths) {
        if (!query.openDocs.count(sibling)) {
            if (!statOne(sibling)) return std::nullopt;
        }
    }
    for (Module* module : allModules(mainModule)) {
        for (auto& sourceFile : module->sourceFiles) {
            const std::string& path = sourceFile.filePath;
            if (path.empty() || path == query.filePath) continue;
            // Siblings are statted from the layout above; only they honor the
            // openDocs overlay (the open file itself comes from content),
            // anything else open in the editor is still compiled from disk.
            if (std::binary_search(layout.siblingPaths.begin(), layout.siblingPaths.end(), path)) continue;
            if (!statOne(path)) return std::nullopt;
        }
    }
    return stats;
}

// The openDocs entries that affect compilation: siblings read from the
// overlay (the open file itself comes from content, anything else is
// ignored). Already sorted via siblingPaths.
std::vector<std::pair<std::string, std::string>> relevantOpenDocs(const LspQuery& query, const ModuleLayout& layout) {
    std::vector<std::pair<std::string, std::string>> docs;
    for (auto& sibling : layout.siblingPaths) {
        if (auto overlay = query.openDocs.find(sibling); overlay != query.openDocs.end()) {
            docs.emplace_back(sibling, overlay->second);
        }
    }
    return docs;
}

// Deletes every module of one compilation (main plus imports), freeing the
// file buffers. Module destructors only release owned storage, never arena
// memory, so this is safe before or after resetAstArena. Null-safe: a failed
// compilation has no main module, but its imports still need freeing.
void deleteModules(Module* mainModule) {
    llvm::SmallPtrSet<Module*, 16> seen;
    auto drop = [&](Module* module) {
        if (module && seen.insert(module).second) delete module;
    };
    drop(mainModule);
    for (auto* imported : Module::getAllImportedModules())
        drop(imported);
}

// True when the cached compilation still describes the query. Stat-only
// probes (no parsing): BuildConfig would abort the process on malformed
// files without a diagnostic collector, and parsing here would also grow
// the arena on every cache hit.
bool cacheMatches(const LspCachedFrontend& entry, const LspQuery& query) {
    auto miss = [](const char* reason) {
        if (profilingEnabled()) llvm::errs() << "[profile] lsp-miss: " << reason << "\n";
        return false;
    };
    if (query.filePath != entry.filePath) return miss("filePath");
    if (query.content != entry.frontend.content) return miss("content");
    if (query.workspaceFolders != entry.workspaceFolders || query.importSearchPaths != entry.importSearchPaths || query.defines != entry.defines)
        return miss("config");
    std::string parentDir = llvm::sys::path::parent_path(query.filePath).str();
    // A new or deleted build.cx above the file can move the module root; a
    // changed one can move it too (caught by depStats below).
    if (collectUpwardBuildFiles(parentDir) != entry.layout.checkedBuildFiles) return miss("buildfiles");
    std::vector<std::string> basePaths = buildMainImportSearchPaths(query, parentDir);
    std::optional<std::string> expectedPackage;
    std::optional<std::string> packageDir;
    if (isStdDirectory(parentDir, basePaths)) {
        expectedPackage = "std";
    } else if (auto package = findImportablePackage(parentDir, withVendorPaths(basePaths, entry.layout.moduleDir, entry.layout.buildDir))) {
        expectedPackage = package->name;
        packageDir = package->dir;
    }
    if (expectedPackage != entry.layout.registerAsPackage) return miss("package");
    if (buildSiblingPaths(entry.layout.moduleDir, entry.layout.buildDir, query.filePath, packageDir) != entry.layout.siblingPaths) return miss("siblings");
    if (relevantOpenDocs(query, entry.layout) != entry.openDocs) return miss("openDocs");
    if (!depStatsMatch(entry.depStats)) return miss("depStats");
    if (profilingEnabled()) llvm::errs() << "[profile] lsp-hit\n";
    return true;
}

// Builds the cache entry for a fresh compilation, moving frontend ownership
// into it. Nullopt when an input vanished mid-compile (uncacheable), and when
// compilation threw partway (no main module): its inputs may be incompletely
// known (a malformed dependency build file aborts the closure before dep
// inputs are enumerated), so the failure must never cache.
std::optional<LspCachedFrontend> buildEntry(const LspQuery& query, FrontendResult& frontend) {
    if (!frontend.mainModule) return std::nullopt;
    LspCachedFrontend entry;
    entry.filePath = query.filePath;
    entry.workspaceFolders = query.workspaceFolders;
    entry.importSearchPaths = query.importSearchPaths;
    entry.defines = query.defines;
    entry.layout = std::move(frontend.layout);
    entry.openDocs = relevantOpenDocs(query, entry.layout);
    if (auto stats = collectDepStats(query, entry.layout, frontend.depBuildFiles, frontend.mainModule)) {
        // A file changed mid-compile records fresh stats for stale content;
        // re-verify so that entry stays uncacheable instead of sticking.
        if (!depStatsMatch(*stats)) return std::nullopt;
        entry.depStats = std::move(*stats);
    } else {
        return std::nullopt;
    }
    entry.frontend = std::move(frontend);
    return entry;
}

} // namespace

JsonValue handleQuery(const JsonValue& queryJson) {
    LspQuery query = parseLspQuery(queryJson);
    if (query.method == "completion" && needsCompletionPlaceholder(query.content, query.position)) {
        query.content = insertCompletionPlaceholder(query.content, query.position);
    }
    FrontendResult frontend = runFrontendOnce(query);
    return answerFromFrontend(query, frontend);
}

void LspSession::dropCache() {
    if (cached) {
        // Reset first: cached diagnostics hold locations into the file
        // buffers, so they must be gone before the modules are freed.
        Module* mainModule = cached->frontend.mainModule;
        cached.reset();
        deleteModules(mainModule);
    }
    resetCompilerGlobals();
}

std::optional<JsonValue> LspSession::handle(LspQuery query) {
    try {
        if (query.method == "completion" && needsCompletionPlaceholder(query.content, query.position)) {
            query.content = insertCompletionPlaceholder(query.content, query.position);
        }
        if (cached && cacheMatches(*cached, query)) {
            return answerFromFrontend(query, cached->frontend);
        }
        dropCache();
        FrontendResult frontend = runFrontendOnce(query);
        if (auto entry = buildEntry(query, frontend)) {
            if (profilingEnabled()) llvm::errs() << "[profile] lsp-stored\n";
            cached = std::move(*entry);
            return answerFromFrontend(query, cached->frontend);
        }
        if (profilingEnabled()) llvm::errs() << "[profile] lsp-uncacheable\n";
        // Uncacheable (inputs vanished mid-compile, or compilation threw
        // partway): answer ad hoc without keeping anything, so the next
        // query recompiles instead of reusing it.
        llvm::scope_exit drop([&] { deleteModules(frontend.mainModule); });
        return answerFromFrontend(query, frontend);
    } catch (const std::exception& error) {
        llvm::errs() << "[cx-lsp] query failed: " << error.what() << '\n';
        return std::nullopt;
    } catch (...) {
        llvm::errs() << "[cx-lsp] query failed: unknown error\n";
        return std::nullopt;
    }
}

} // namespace cx::lsp
