#include "typecheck.h"
#pragma warning(push, 0)
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/SaveAndRestore.h>
#pragma warning(pop)
#include "../ast/module.h"
#include "../driver/driver.h"
#include "../package-manager/manifest.h"
#include "../parser/parse.h"

using namespace delta;

static const Expr& getIfOrWhileCondition(const Stmt& ifOrWhileStmt) {
    switch (ifOrWhileStmt.getKind()) {
        case StmtKind::IfStmt:
            return llvm::cast<IfStmt>(ifOrWhileStmt).getCondition();
        case StmtKind::WhileStmt:
            return llvm::cast<WhileStmt>(ifOrWhileStmt).getCondition();
        default:
            llvm_unreachable("should be IfStmt or WhileStmt");
    }
}

static llvm::ArrayRef<Stmt*> getIfOrWhileThenBody(const Stmt& ifOrWhileStmt) {
    switch (ifOrWhileStmt.getKind()) {
        case StmtKind::IfStmt:
            return llvm::cast<IfStmt>(ifOrWhileStmt).getThenBody();
        case StmtKind::WhileStmt:
            return llvm::cast<WhileStmt>(ifOrWhileStmt).getBody();
        default:
            llvm_unreachable("should be IfStmt or WhileStmt");
    }
}

static llvm::Optional<bool> memberExprChainsMatch(const MemberExpr& a, const MemberExpr& b) {
    if (a.getMemberName() != b.getMemberName()) return false;

    switch (a.getBaseExpr()->getKind()) {
        case ExprKind::VarExpr: {
            auto* bBaseVarExpr = llvm::dyn_cast<VarExpr>(b.getBaseExpr());
            ASSERT(llvm::cast<VarExpr>(a.getBaseExpr())->getDecl() && bBaseVarExpr->getDecl());
            return bBaseVarExpr && llvm::cast<VarExpr>(a.getBaseExpr())->getDecl() == bBaseVarExpr->getDecl();
        }
        case ExprKind::MemberExpr: {
            auto* bBaseMemberExpr = llvm::dyn_cast<MemberExpr>(b.getBaseExpr());
            return bBaseMemberExpr && memberExprChainsMatch(*llvm::cast<MemberExpr>(a.getBaseExpr()), *bBaseMemberExpr);
        }
        default:
            return llvm::None;
    }
}

bool Typechecker::isGuaranteedNonNull(const Expr& expr) const {
    for (auto* currentControlStmt : llvm::reverse(currentControlStmts)) {
        if (isGuaranteedNonNull(expr, *currentControlStmt)) return true;
    }
    return false;
}

namespace {
struct NullCheckInfo {
    const Expr* nullableValue = nullptr;
    Token::Kind op = Token::None;
    bool isNullCheck() const { return nullableValue != nullptr; }
};
} // namespace

static NullCheckInfo analyzeNullCheck(const Expr& condition) {
    NullCheckInfo nullCheckInfo;

    if (condition.getType().isOptionalType()) {
        nullCheckInfo.nullableValue = &condition;
        nullCheckInfo.op = Token::NotEqual;
    } else if (auto* binaryExpr = llvm::dyn_cast<BinaryExpr>(&condition)) {
        if (binaryExpr->getRHS().isNullLiteralExpr()) {
            nullCheckInfo.nullableValue = &binaryExpr->getLHS();
            nullCheckInfo.op = binaryExpr->getOperator();
        }
    } else if (auto* unaryExpr = llvm::dyn_cast<UnaryExpr>(&condition)) {
        if (unaryExpr->getOperator() == Token::Not) {
            nullCheckInfo = analyzeNullCheck(unaryExpr->getOperand());
            nullCheckInfo.op = nullCheckInfo.op == Token::Equal ? Token::NotEqual : Token::Equal;
        }
    }

    if (nullCheckInfo.isNullCheck()) {
        ASSERT(nullCheckInfo.op == Token::NotEqual || nullCheckInfo.op == Token::Equal);
    }

    return nullCheckInfo;
}

bool Typechecker::isGuaranteedNonNull(const Expr& expr, const Stmt& currentControlStmt) const {
    if (!currentControlStmt.isIfStmt() && !currentControlStmt.isWhileStmt()) return false;
    auto& condition = getIfOrWhileCondition(currentControlStmt);
    NullCheckInfo nullCheckInfo = analyzeNullCheck(condition);
    if (!nullCheckInfo.isNullCheck()) return false;

    switch (expr.getKind()) {
        case ExprKind::VarExpr: {
            auto* decl = llvm::cast<VarExpr>(expr).getDecl();
            if (!decl) return false;
            auto* lhs = llvm::dyn_cast<VarExpr>(nullCheckInfo.nullableValue);
            if (!lhs || lhs->getDecl() != decl) return false;
            break;
        }
        case ExprKind::MemberExpr: {
            auto* lhs = llvm::dyn_cast<MemberExpr>(nullCheckInfo.nullableValue);
            if (!lhs) return false;
            if (memberExprChainsMatch(llvm::cast<MemberExpr>(expr), *lhs) != true) return false;
            break;
        }
        default:
            return false;
    }

    llvm::ArrayRef<Stmt*> stmts;
    switch (nullCheckInfo.op) {
        case Token::NotEqual:
            stmts = getIfOrWhileThenBody(currentControlStmt);
            break;
        case Token::Equal:
            if (auto* ifStmt = llvm::dyn_cast<IfStmt>(&currentControlStmt)) {
                stmts = ifStmt->getElseBody();
            } else {
                return false;
            }
            break;
        default:
            return false;
    }

    for (auto& stmt : stmts) {
        if (auto result = maySetToNullBeforeEvaluating(expr, *stmt)) return !*result;
    }

    return false;
}

llvm::Optional<bool> Typechecker::maySetToNullBeforeEvaluating(const Expr& var, const Stmt& stmt) const {
    switch (stmt.getKind()) {
        case StmtKind::ReturnStmt:
            if (auto* returnValue = llvm::cast<ReturnStmt>(stmt).getReturnValue()) {
                return maySetToNullBeforeEvaluating(var, *returnValue);
            }
            return llvm::None;

        case StmtKind::VarStmt:
            return maySetToNullBeforeEvaluating(var, *llvm::cast<VarStmt>(stmt).getDecl().getInitializer());

        case StmtKind::ExprStmt:
            return maySetToNullBeforeEvaluating(var, llvm::cast<ExprStmt>(stmt).getExpr());

        case StmtKind::DeferStmt:
            return true;

        case StmtKind::IfStmt: {
            auto& ifStmt = llvm::cast<IfStmt>(stmt);
            if (auto result = maySetToNullBeforeEvaluating(var, ifStmt.getCondition())) return *result;
            if (auto result = maySetToNullBeforeEvaluating(var, ifStmt.getThenBody())) return *result;
            if (auto result = maySetToNullBeforeEvaluating(var, ifStmt.getElseBody())) return *result;
            return llvm::None;
        }
        case StmtKind::SwitchStmt:
            return true;

        case StmtKind::WhileStmt: {
            auto& whileStmt = llvm::cast<WhileStmt>(stmt);
            if (auto result = maySetToNullBeforeEvaluating(var, whileStmt.getCondition())) return *result;
            if (auto result = maySetToNullBeforeEvaluating(var, whileStmt.getBody())) return *result;
            return llvm::None;
        }
        case StmtKind::ForStmt:
            llvm_unreachable("ForStmt should be lowered into a WhileStmt");

        case StmtKind::BreakStmt:
        case StmtKind::ContinueStmt:
            return false;

        case StmtKind::CompoundStmt:
            return maySetToNullBeforeEvaluating(var, llvm::cast<CompoundStmt>(stmt).getBody());
    }

    llvm_unreachable("all cases handled");
}

llvm::Optional<bool> Typechecker::subExprMaySetToNullBeforeEvaluating(const Expr& var, const Expr& expr) const {
    for (auto* subExpr : expr.getSubExprs()) {
        if (auto result = maySetToNullBeforeEvaluating(var, *subExpr)) return *result;
    }
    return llvm::None;
}

llvm::Optional<bool> Typechecker::maySetToNullBeforeEvaluating(const Expr& var, const Expr& expr) const {
    if (&expr == &var) return false;
    if (auto result = subExprMaySetToNullBeforeEvaluating(var, expr)) return *result;
    if (expr.isCallExpr()) return true;
    if (expr.isAssignment()) {
        auto* lhs = &llvm::cast<BinaryExpr>(expr).getLHS();
        auto* rhs = &llvm::cast<BinaryExpr>(expr).getRHS();

        if (auto result = maySetToNullBeforeEvaluating(var, *lhs)) return *result;
        if (auto result = maySetToNullBeforeEvaluating(var, *rhs)) return *result;

        switch (var.getKind()) {
            case ExprKind::VarExpr:
                if (auto* lhsVarExpr = llvm::dyn_cast<VarExpr>(lhs)) {
                    return lhsVarExpr->getDecl() == llvm::cast<VarExpr>(var).getDecl() && rhs->getType().isOptionalType();
                }
                return llvm::None;

            case ExprKind::MemberExpr:
                if (auto* lhsMemberExpr = llvm::dyn_cast<MemberExpr>(lhs)) {
                    if (auto result = memberExprChainsMatch(*lhsMemberExpr, llvm::cast<MemberExpr>(var))) {
                        return *result && rhs->getType().isOptionalType();
                    }
                }
                return llvm::None;

            default:
                llvm_unreachable("unsupported variable expression kind");
        }
    }
    return llvm::None;
}

llvm::Optional<bool> Typechecker::maySetToNullBeforeEvaluating(const Expr& var, llvm::ArrayRef<Stmt*> block) const {
    for (auto& stmt : block) {
        if (auto result = maySetToNullBeforeEvaluating(var, *stmt)) return *result;
    }
    return llvm::None;
}

TypeDecl* Typechecker::getTypeDecl(const BasicType& type) {
    if (auto* typeDecl = type.getDecl()) {
        return typeDecl;
    }

    auto decls = findDecls(type.getQualifiedName());

    if (!decls.empty()) {
        ASSERT(decls.size() == 1);
        return llvm::dyn_cast_or_null<TypeDecl>(decls[0]);
    }

    decls = findDecls(type.getName());
    if (decls.empty()) return nullptr;
    ASSERT(decls.size() == 1);
    auto instantiation = llvm::cast<TypeTemplate>(decls[0])->instantiate(type.getGenericArgs());
    getCurrentModule()->addToSymbolTable(*instantiation);
    declsToTypecheck.push_back(instantiation);
    return instantiation;
}

static std::error_code parseSourcesInDirectoryRecursively(const llvm::Twine& directoryPath, Module& module, const CompileOptions& options) {
    std::error_code error;
    llvm::sys::fs::recursive_directory_iterator it(directoryPath, error), end;

    for (; it != end; it.increment(error)) {
        if (error) break;
        if (llvm::sys::path::extension(it->path()) == ".delta") {
            Parser parser(it->path(), module, options);
            parser.parse();
        }
    }

    return error;
}

llvm::ErrorOr<const Module&> Typechecker::importDeltaModule(SourceFile* importer, const PackageManifest* manifest, llvm::StringRef moduleName) {
    auto it = Module::getAllImportedModulesMap().find(moduleName);
    if (it != Module::getAllImportedModulesMap().end()) {
        if (importer) importer->addImportedModule(it->second);
        return *it->second;
    }

    auto module = new Module(moduleName);
    std::error_code error;

    if (manifest) {
        for (auto& dependency : manifest->getDeclaredDependencies()) {
            if (dependency.getPackageIdentifier() == moduleName) {
                error = parseSourcesInDirectoryRecursively(dependency.getFileSystemPath(), *module, options);
                goto done;
            }
        }
    }

    for (llvm::StringRef importPath : options.importSearchPaths) {
        auto modulePath = importPath + "/" + moduleName;
        if (llvm::sys::fs::is_directory(modulePath)) {
            error = parseSourcesInDirectoryRecursively(modulePath, *module, options);
            goto done;
        }
    }

done:
    if (error || module->getSourceFiles().empty()) {
        return error;
    }

    if (importer) importer->addImportedModule(module);
    Module::getAllImportedModulesMap()[module->getName()] = module;
    typecheckModule(*module, nullptr);
    return *module;
}

void Typechecker::postProcess() {
    llvm::SaveAndRestore setPostProcessing(isPostProcessing, true);

    while (!declsToTypecheck.empty()) {
        auto currentDeclsToTypecheck = std::move(declsToTypecheck);

        for (auto* decl : currentDeclsToTypecheck) {
            switch (decl->getKind()) {
                case DeclKind::FunctionDecl:
                case DeclKind::MethodDecl:
                case DeclKind::InitDecl:
                case DeclKind::DeinitDecl:
                    typecheckFunctionDecl(*llvm::cast<FunctionDecl>(decl));
                    break;
                case DeclKind::FunctionTemplate:
                    typecheckFunctionTemplate(*llvm::cast<FunctionTemplate>(decl));
                    break;
                case DeclKind::TypeDecl:
                    typecheckTypeDecl(*llvm::cast<TypeDecl>(decl));
                    break;
                default:
                    llvm_unreachable("invalid deferred decl");
            }
        }
    }
}

static void checkUnusedDecls(const Module& module) {
    for (auto& sourceFile : module.getSourceFiles()) {
        for (auto& decl : sourceFile.getTopLevelDecls()) {
            if (decl->isReferenced()) continue;

            if (decl->isFunctionDecl() || decl->isFunctionTemplate()) {
                if (decl->isMain()) continue;
                WARN(decl->getLocation(), "unused declaration '" << decl->getName() << "'");
            }
        }
    }
}

void Typechecker::typecheckModule(Module& module, const PackageManifest* manifest) {
    auto stdModule = importDeltaModule(nullptr, nullptr, "std");
    if (!stdModule) {
        ABORT("couldn't import the standard library: " << stdModule.getError().message());
    }

    // Infer the types of global variables for use before their declaration.
    for (auto& sourceFile : module.getSourceFiles()) {
        currentModule = &module;
        currentSourceFile = &sourceFile;

        for (auto& decl : sourceFile.getTopLevelDecls()) {
            if (auto* varDecl = llvm::dyn_cast<VarDecl>(decl)) {
                try {
                    typecheckVarDecl(*varDecl, true);
                } catch (const CompileError& error) {
                    error.print();
                }
            }
        }

        postProcess();
    }

    for (auto& sourceFile : module.getSourceFiles()) {
        for (auto& decl : sourceFile.getTopLevelDecls()) {
            currentModule = &module;
            currentSourceFile = &sourceFile;

            if (!decl->isVarDecl()) {
                typecheckTopLevelDecl(*decl, manifest);
                postProcess();
            }
        }
    }

    if (module.getName() != "std" && isWarningEnabled("unused")) {
        checkUnusedDecls(module);
    }

    currentModule = nullptr;
    currentSourceFile = nullptr;
}

bool Typechecker::isWarningEnabled(llvm::StringRef warning) const {
    return !llvm::is_contained(options.disabledWarnings, warning);
}

static llvm::SmallVector<Decl*, 1> findDeclsInModules(llvm::StringRef name, llvm::ArrayRef<Module*> modules) {
    llvm::SmallVector<Decl*, 1> decls;

    for (auto& module : modules) {
        llvm::ArrayRef<Decl*> matches = module->getSymbolTable().find(name);
        decls.append(matches.begin(), matches.end());
    }

    return decls;
}

static Decl* findDeclInModules(llvm::StringRef name, SourceLocation location, llvm::ArrayRef<Module*> modules) {
    auto decls = findDeclsInModules(name, modules);

    if (decls.empty()) {
        return nullptr;
    } else {
        if (decls.size() > 1) ERROR(location, "ambiguous reference to '" << name << "'");
        return decls[0];
    }
}

Decl* Typechecker::findDecl(llvm::StringRef name, SourceLocation location) const {
    ASSERT(!name.empty());

    if (Decl* match = findDeclInModules(name, location, currentModule)) {
        return match;
    }

    if (currentFunction) {
        if (auto* typeDecl = currentFunction->getTypeDecl()) {
            for (auto& field : typeDecl->getFields()) {
                if (field.getName() == name) {
                    return &field;
                }
            }
        }
    }

    if (Decl* match = findDeclInModules(name, location, currentModule->getStdlibModules())) {
        return match;
    }

    if (currentSourceFile) {
        if (Decl* match = findDeclInModules(name, location, currentSourceFile->getImportedModules())) {
            return match;
        }
    } else {
        if (Decl* match = findDeclInModules(name, location, currentModule->getAllImportedModules())) {
            return match;
        }
    }

    ERROR(location, "unknown identifier '" << name << "'");
}

std::vector<Decl*> Typechecker::findDecls(llvm::StringRef name, TypeDecl* receiverTypeDecl, bool inAllImportedModules) const {
    std::vector<Decl*> decls;

    if (!receiverTypeDecl && currentFunction) {
        receiverTypeDecl = currentFunction->getTypeDecl();
    }

    if (receiverTypeDecl) {
        for (auto& decl : receiverTypeDecl->getMemberDecls()) {
            if (auto* functionDecl = llvm::dyn_cast<FunctionDecl>(decl)) {
                if (functionDecl->getName() == name) {
                    decls.emplace_back(decl);
                }
            } else if (auto* functionTemplate = llvm::dyn_cast<FunctionTemplate>(decl)) {
                if (functionTemplate->getQualifiedName() == name) {
                    decls.emplace_back(decl);
                }
            }
        }

        for (auto& field : receiverTypeDecl->getFields()) {
            // TODO: Only one comparison should be needed.
            if (field.getName() == name || field.getQualifiedName() == name) {
                decls.emplace_back(&field);
            }
        }
    }

    if (currentModule->getName() != "std") {
        append(decls, findDeclsInModules(name, currentModule));
    }

    append(decls, findDeclsInModules(name, currentModule->getStdlibModules()));

    if (currentSourceFile && !inAllImportedModules) {
        append(decls, findDeclsInModules(name, currentSourceFile->getImportedModules()));
    } else {
        append(decls, findDeclsInModules(name, currentModule->getAllImportedModules()));
    }

    return decls;
}
