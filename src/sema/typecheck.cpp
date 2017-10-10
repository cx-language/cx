#include <iterator>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <cstdlib>
#include <system_error>
#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/Support/ErrorHandling.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/ErrorOr.h>
#include "typecheck.h"
#include "c-import.h"
#include "../ast/type.h"
#include "../ast/expr.h"
#include "../ast/decl.h"
#include "../ast/module.h"
#include "../ast/mangle.h"
#include "../package-manager/manifest.h"
#include "../support/utility.h"

using namespace delta;

namespace delta {
std::unordered_map<std::string, std::shared_ptr<Module>> allImportedModules;
}

std::vector<Module*> delta::getAllImportedModules() {
    return map(allImportedModules,
               [](const std::pair<std::string, std::shared_ptr<Module>>& p) { return p.second.get(); });
}

namespace {

/// Storage for Decls that are not in the AST but are referenced by the symbol table.
std::vector<std::unique_ptr<Decl>> nonASTDecls;

llvm::MutableArrayRef<FieldDecl> currentFieldDecls;
Type functionReturnType = nullptr;
bool inInitializer = false;
int breakableBlocks = 0;

}

void TypeChecker::typecheckReturnStmt(ReturnStmt& stmt) const {
    if (stmt.getValues().empty()) {
        if (!functionReturnType.isVoid()) {
            error(stmt.getLocation(), "expected return statement to return a value of type '",
                  functionReturnType, "'");
        }
        return;
    }

    auto returnValueTypes = map(stmt.getValues(),
                                [&](const std::unique_ptr<Expr>& value) { return typecheckExpr(*value); });

    Type returnType = returnValueTypes.size() > 1
                      ? TupleType::get(std::move(returnValueTypes)) : returnValueTypes[0];
    if (!isValidConversion(stmt.getValues(), returnType, functionReturnType)) {
        error(stmt.getLocation(), "mismatching return type '", returnType, "', expected '",
              functionReturnType, "'");
    }
}

void TypeChecker::typecheckVarStmt(VarStmt& stmt) const {
    typecheckVarDecl(stmt.getDecl(), false);
}

void TypeChecker::typecheckIncrementStmt(IncrementStmt& stmt) const {
    auto type = typecheckExpr(stmt.getOperand());
    if (!type.isMutable()) {
        error(stmt.getLocation(), "cannot increment immutable value");
    }
    // TODO: check that operand supports increment operation.
}

void TypeChecker::typecheckDecrementStmt(DecrementStmt& stmt) const {
    auto type = typecheckExpr(stmt.getOperand());
    if (!type.isMutable()) {
        error(stmt.getLocation(), "cannot decrement immutable value");
    }
    // TODO: check that operand supports decrement operation.
}

void TypeChecker::typecheckIfStmt(IfStmt& ifStmt) const {
    Type conditionType = typecheckExpr(ifStmt.getCondition());
    if (!conditionType.isBool()) {
        error(ifStmt.getCondition().getLocation(), "'if' condition must have type 'bool'");
    }
    for (auto& stmt : ifStmt.getThenBody()) {
        typecheckStmt(*stmt);
    }
    for (auto& stmt : ifStmt.getElseBody()) {
        typecheckStmt(*stmt);
    }
}

void TypeChecker::typecheckSwitchStmt(SwitchStmt& stmt) const {
    Type conditionType = typecheckExpr(stmt.getCondition());
    breakableBlocks++;
    for (auto& switchCase : stmt.getCases()) {
        Type caseType = typecheckExpr(*switchCase.getValue());
        if (!caseType.isImplicitlyConvertibleTo(conditionType)) {
            error(switchCase.getValue()->getLocation(), "case value type '", caseType,
                  "' doesn't match switch condition type '", conditionType, "'");
        }
        for (auto& caseStmt : switchCase.getStmts()) {
            typecheckStmt(*caseStmt);
        }
    }
    for (auto& defaultStmt : stmt.getDefaultStmts()) {
        typecheckStmt(*defaultStmt);
    }
    breakableBlocks--;
}

void TypeChecker::typecheckWhileStmt(WhileStmt& whileStmt) const {
    Type conditionType = typecheckExpr(whileStmt.getCondition());
    if (!conditionType.isBool()) {
        error(whileStmt.getCondition().getLocation(), "'while' condition must have type 'bool'");
    }
    breakableBlocks++;
    for (auto& stmt : whileStmt.getBody()) {
        typecheckStmt(*stmt);
    }
    breakableBlocks--;
}

void TypeChecker::typecheckForStmt(ForStmt& forStmt) const {
    if (getCurrentModule()->getSymbolTable().contains(forStmt.getLoopVariableName())) {
        error(forStmt.getLocation(), "redefinition of '", forStmt.getLoopVariableName(), "'");
    }

    Type rangeType = typecheckExpr(forStmt.getRangeExpr());
    if (!rangeType.isIterable()) {
        error(forStmt.getRangeExpr().getLocation(), "'for' range expression is not an 'Iterable'");
    }

    getCurrentModule()->getSymbolTable().pushScope();
    addToSymbolTable(VarDecl(rangeType.getIterableElementType(), std::string(forStmt.getLoopVariableName()),
                             nullptr, *getCurrentModule(), forStmt.getLocation()));
    breakableBlocks++;
    for (auto& stmt : forStmt.getBody()) {
        typecheckStmt(*stmt);
    }
    breakableBlocks--;
    getCurrentModule()->getSymbolTable().popScope();
}

void TypeChecker::typecheckBreakStmt(BreakStmt& breakStmt) const {
    if (breakableBlocks == 0) {
        error(breakStmt.getLocation(), "'break' is only allowed inside 'while' and 'switch' statements");
    }
}

void TypeChecker::typecheckAssignStmt(AssignStmt& stmt) const {
    Type lhsType = typecheckExpr(*stmt.getLHS(), true);
    if (lhsType.isFunctionType()) error(stmt.getLocation(), "cannot assign to function");
    Type rhsType = typecheckExpr(*stmt.getRHS());

    if (!isValidConversion(*stmt.getRHS(), rhsType, lhsType)) {
        error(stmt.getRHS()->getLocation(), "cannot assign '", rhsType, "' to variable of type '", lhsType, "'");
    }

    if (!lhsType.isMutable() && !inInitializer) {
        if (auto* varExpr = llvm::dyn_cast<VarExpr>(stmt.getLHS())) {
            error(stmt.getLocation(), "cannot assign to immutable variable '", varExpr->getIdentifier(), "'");
        } else {
            error(stmt.getLocation(), "cannot assign to immutable expression");
        }
    }

    if (!isImplicitlyCopyable(rhsType)) {
        if (auto* varExpr = llvm::dyn_cast<VarExpr>(stmt.getRHS())) {
            varExpr->getDecl()->markAsMoved();
        }
    }
}

void TypeChecker::typecheckStmt(Stmt& stmt) const {
    switch (stmt.getKind()) {
        case StmtKind::ReturnStmt: typecheckReturnStmt(llvm::cast<ReturnStmt>(stmt)); break;
        case StmtKind::VarStmt: typecheckVarStmt(llvm::cast<VarStmt>(stmt)); break;
        case StmtKind::IncrementStmt: typecheckIncrementStmt(llvm::cast<IncrementStmt>(stmt)); break;
        case StmtKind::DecrementStmt: typecheckDecrementStmt(llvm::cast<DecrementStmt>(stmt)); break;
        case StmtKind::ExprStmt: typecheckExpr(llvm::cast<ExprStmt>(stmt).getExpr()); break;
        case StmtKind::DeferStmt: typecheckExpr(llvm::cast<DeferStmt>(stmt).getExpr()); break;
        case StmtKind::IfStmt: typecheckIfStmt(llvm::cast<IfStmt>(stmt)); break;
        case StmtKind::SwitchStmt: typecheckSwitchStmt(llvm::cast<SwitchStmt>(stmt)); break;
        case StmtKind::WhileStmt: typecheckWhileStmt(llvm::cast<WhileStmt>(stmt)); break;
        case StmtKind::ForStmt: typecheckForStmt(llvm::cast<ForStmt>(stmt)); break;
        case StmtKind::BreakStmt: typecheckBreakStmt(llvm::cast<BreakStmt>(stmt)); break;
        case StmtKind::AssignStmt: typecheckAssignStmt(llvm::cast<AssignStmt>(stmt)); break;
    }
}

void TypeChecker::typecheckParamDecl(ParamDecl& decl) const {
    if (getCurrentModule()->getSymbolTable().contains(decl.getName())) {
        error(decl.getLocation(), "redefinition of '", decl.getName(), "'");
    }

    if (auto* basicType = llvm::dyn_cast<BasicType>(decl.getType().get())) {
        auto decls = findDecls(basicType->getName());
        if (!decls.empty()) {
            auto& typeDecl = llvm::cast<TypeDecl>(*decls[0]);
            if (auto* deinitDecl = typeDecl.getDeinitializer()) {
                SAVE_STATE(currentGenericArgs);
                ASSERT(basicType->getGenericArgs().size() == typeDecl.getGenericParams().size());
                for (auto t : llvm::zip_first(typeDecl.getGenericParams(), basicType->getGenericArgs())) {
                    currentGenericArgs.emplace(std::get<0>(t).getName(), std::get<1>(t));
                }
                SAVE_STATE(typecheckingGenericFunction);
                typecheckingGenericFunction = true;
                typecheckFunctionLikeDecl(*deinitDecl);
            }
        }
    }

    getCurrentModule()->getSymbolTable().add(decl.getName(), &decl);
}

void TypeChecker::addToSymbolTableWithName(Decl& decl, llvm::StringRef name) const {
    if (getCurrentModule()->getSymbolTable().contains(name)) {
        error(decl.getLocation(), "redefinition of '", name, "'");
    }
    getCurrentModule()->getSymbolTable().add(name, &decl);
}

template<typename DeclT>
void TypeChecker::addToSymbolTableCheckParams(DeclT& decl) const {
    if (getCurrentModule()->getSymbolTable().findWithMatchingParams(decl)) {
        error(decl.getLocation(), "redefinition of '", mangle(decl), "'");
    }
    getCurrentModule()->getSymbolTable().add(mangle(decl), &decl);
}

void TypeChecker::addToSymbolTable(FunctionDecl& decl) const {
    addToSymbolTableCheckParams(decl);
}

void TypeChecker::addToSymbolTable(InitDecl& decl) const {
    addToSymbolTableCheckParams(decl);
}

void TypeChecker::addToSymbolTable(DeinitDecl& decl) const {
    addToSymbolTableWithName(decl, mangle(decl));
}

void TypeChecker::addToSymbolTable(TypeDecl& decl) const {
    addToSymbolTableWithName(decl, decl.getName());

    for (auto& memberDecl : decl.getMemberDecls()) {
        switch (memberDecl->getKind()) {
            case DeclKind::MethodDecl:
                addToSymbolTable(llvm::cast<MethodDecl>(*memberDecl));
                break;
            case DeclKind::InitDecl:
                addToSymbolTable(llvm::cast<InitDecl>(*memberDecl));
                break;
            case DeclKind::DeinitDecl:
                addToSymbolTable(llvm::cast<DeinitDecl>(*memberDecl));
                break;
            default:
                llvm_unreachable("invalid member declaration kind");
        }
    }
}

void TypeChecker::addToSymbolTable(VarDecl& decl) const {
    addToSymbolTableWithName(decl, decl.getName());
}

template<typename DeclT>
void TypeChecker::addToSymbolTableNonAST(DeclT& decl) const {
    std::string name = decl.getName();
    nonASTDecls.push_back(llvm::make_unique<DeclT>(std::move(decl)));
    getCurrentModule()->getSymbolTable().add(std::move(name), nonASTDecls.back().get());
}

void TypeChecker::addToSymbolTable(FunctionDecl&& decl) const {
    addToSymbolTableNonAST(decl);
}

void TypeChecker::addToSymbolTable(TypeDecl&& decl) const {
    addToSymbolTableNonAST(decl);
}

void TypeChecker::addToSymbolTable(VarDecl&& decl) const {
    addToSymbolTableNonAST(decl);
}

void TypeChecker::addIdentifierReplacement(llvm::StringRef source, llvm::StringRef target) const {
    getCurrentModule()->getSymbolTable().addIdentifierReplacement(source, target);
}

template<typename ModuleContainer>
static llvm::SmallVector<Decl*, 1> findDeclsInModules(llvm::StringRef name,
                                                      const ModuleContainer& modules) {
    llvm::SmallVector<Decl*, 1> decls;

    for (auto& module : modules) {
        llvm::ArrayRef<Decl*> matches = module->getSymbolTable().find(name);
        decls.append(matches.begin(), matches.end());
    }

    return decls;
}

template<typename ModuleContainer>
static Decl* findDeclInModules(llvm::StringRef name, SourceLocation location, const ModuleContainer& modules) {
    auto decls = findDeclsInModules(name, modules);

    switch (decls.size()) {
        case 1: return decls[0];
        case 0: return nullptr;
        default: error(location, "ambiguous reference to '", name, "'");
    }
}

llvm::ArrayRef<std::shared_ptr<Module>> getStdlibModules() {
    auto it = allImportedModules.find("std");
    if (it == allImportedModules.end()) return {};
    return it->second;
}

Decl& TypeChecker::findDecl(llvm::StringRef name, SourceLocation location, bool everywhere) const {
    ASSERT(!name.empty());

    if (Decl* match = findDeclInModules(name, location, llvm::makeArrayRef(getCurrentModule()))) {
        return *match;
    }

    for (auto& field : currentFieldDecls) {
        if (field.getName() == name) {
            return field;
        }
    }

    if (Decl* match = findDeclInModules(name, location, getStdlibModules())) {
        return *match;
    }

    if (everywhere) {
        if (Decl* match = findDeclInModules(name, location, getAllImportedModules())) {
            return *match;
        }
    } else {
        if (Decl* match = findDeclInModules(name, location, getCurrentSourceFile()->getImportedModules())) {
            return *match;
        }
    }

    error(location, "unknown identifier '", name, "'");
}

std::vector<Decl*> TypeChecker::findDecls(llvm::StringRef name, bool everywhere) const {
    std::vector<Decl*> decls;

    if (auto* typeDecl = currentFunction ? currentFunction->getTypeDecl() : nullptr) {
        for (auto& decl : typeDecl->getMemberDecls()) {
            if (auto* functionDecl = llvm::dyn_cast<FunctionDecl>(decl.get())) {
                if (functionDecl->getName() == name) {
                    decls.emplace_back(decl.get());
                }
            }
        }
    }

    if (getCurrentModule()->getName() != "std") {
        append(decls, findDeclsInModules(name, llvm::makeArrayRef(getCurrentModule())));
    }
    append(decls, findDeclsInModules(name, getStdlibModules()));
    append(decls, everywhere ? findDeclsInModules(name, getAllImportedModules())
                             : findDeclsInModules(name, getCurrentSourceFile()->getImportedModules()));
    return decls;
}

bool allPathsReturn(llvm::ArrayRef<std::unique_ptr<Stmt>> block) {
    if (block.empty()) return false;

    switch (block.back()->getKind()) {
        case StmtKind::ReturnStmt:
            return true;
        case StmtKind::IfStmt: {
            auto& ifStmt = llvm::cast<IfStmt>(*block.back());
            return allPathsReturn(ifStmt.getThenBody()) && allPathsReturn(ifStmt.getElseBody());
        }
        case StmtKind::SwitchStmt: {
            auto& switchStmt = llvm::cast<SwitchStmt>(*block.back());
            return llvm::all_of(switchStmt.getCases(),
                                [](const SwitchCase& c) { return allPathsReturn(c.getStmts()); })
                   && allPathsReturn(switchStmt.getDefaultStmts());
        }
        default:
            return false;
    }
}

void TypeChecker::typecheckGenericParamDecls(llvm::ArrayRef<GenericParamDecl> genericParams) const {
    for (auto& genericParam : genericParams) {
        if (getCurrentModule()->getSymbolTable().contains(genericParam.getName())) {
            error(genericParam.getLocation(), "redefinition of '", genericParam.getName(), "'");
        }
    }
}

std::vector<Type> TypeChecker::getGenericArgsAsArray() const {
    return map(currentGenericArgs, [](const std::pair<std::string, Type>& p) { return p.second; });
}

std::vector<Type> TypeChecker::getUnresolvedGenericArgs() const {
    return map(currentGenericArgs, [](const std::pair<std::string, Type>& p) {
        return BasicType::get(p.first, {});
    });
}

void TypeChecker::typecheckFunctionLikeDecl(FunctionLikeDecl& decl) const {
    if (decl.isExtern()) return;

    if (decl.isGeneric() && currentGenericArgs.empty()) {
        typecheckGenericParamDecls(decl.getGenericParams());
        return; // Partial type-checking of uninstantiated generic functions not implemented yet.
    }

    TypeDecl* receiverTypeDecl = decl.getTypeDecl();
    if (receiverTypeDecl && receiverTypeDecl->isGeneric() && currentGenericArgs.empty()) {
        return; // Partial type-checking of uninstantiated generic functions not implemented yet.
    }

    getCurrentModule()->getSymbolTable().pushScope();
    SAVE_STATE(currentFunction);
    currentFunction = &decl;

    for (ParamDecl& param : decl.getParams()) {
        if (param.getType().isMutable()) {
            error(param.getLocation(), "parameter types cannot be 'mutable'");
        }
        typecheckParamDecl(param);
    }
    if (decl.getReturnType().isMutable()) error(decl.getLocation(), "return types cannot be 'mutable'");

    if (decl.getBody()) {
        SAVE_STATE(functionReturnType);
        functionReturnType = decl.getReturnType();
        SAVE_STATE(currentFieldDecls);
        if (receiverTypeDecl) {
            currentFieldDecls = receiverTypeDecl->getFields();
            Type thisType = receiverTypeDecl->getTypeForPassing(getUnresolvedGenericArgs(), decl.isMutating());
            addToSymbolTable(VarDecl(thisType, "this", nullptr, *getCurrentModule(), SourceLocation::invalid()));
        }

        for (auto& stmt : *decl.getBody()) {
            typecheckStmt(*stmt);
        }
    }

    getCurrentModule()->getSymbolTable().popScope();

    if (!decl.getReturnType().isVoid() && !allPathsReturn(*decl.getBody())) {
        error(decl.getLocation(), "'", decl.getName(), "' is missing a return statement");
    }
}

void TypeChecker::typecheckInitDecl(InitDecl& decl) const {
    getCurrentModule()->getSymbolTable().pushScope();
    SAVE_STATE(currentFunction);
    currentFunction = &decl;

    if (decl.getTypeDecl()->isGeneric() && currentGenericArgs.empty()) {
        return; // Partial type-checking of uninstantiated generic functions not implemented yet.
    }

    addToSymbolTable(VarDecl(decl.getTypeDecl()->getType(getGenericArgsAsArray(), true),
                             "this", nullptr, *getCurrentModule(), SourceLocation::invalid()));
    for (ParamDecl& param : decl.getParams()) typecheckParamDecl(param);

    SAVE_STATE(inInitializer);
    inInitializer = true;
    SAVE_STATE(currentFieldDecls);
    currentFieldDecls = decl.getTypeDecl()->getFields();
    for (auto& stmt : *decl.getBody()) {
        typecheckStmt(*stmt);
    }

    getCurrentModule()->getSymbolTable().popScope();
}

void TypeChecker::typecheckTypeDecl(TypeDecl& decl) const {
    for (auto& memberDecl : decl.getMemberDecls()) {
        typecheckMemberDecl(*memberDecl);
    }
}

TypeDecl* TypeChecker::getTypeDecl(const BasicType& type) const {
    auto decls = findDecls(type.getName());
    if (decls.empty()) return nullptr;
    ASSERT(decls.size() == 1);
    return llvm::cast<TypeDecl>(decls[0]);
}

void TypeChecker::typecheckVarDecl(VarDecl& decl, bool isGlobal) const {
    if (!isGlobal && getCurrentModule()->getSymbolTable().contains(decl.getName())) {
        error(decl.getLocation(), "redefinition of '", decl.getName(), "'");
    }

    Type declaredType = decl.getType();
    Type initType = nullptr;

    if (decl.getInitializer()) {
        initType = typecheckExpr(*decl.getInitializer());

        if (initType.isFunctionType()) {
            error(decl.getInitializer()->getLocation(), "function pointers not implemented yet");
        }
    } else if (isGlobal) {
        error(decl.getLocation(), "global variables cannot be uninitialized");
    }

    if (declaredType) {
        if (initType && !isValidConversion(*decl.getInitializer(), initType, declaredType)) {
            error(decl.getInitializer()->getLocation(), "cannot initialize variable of type '", declaredType,
                  "' with '", initType, "'");
        }
    } else {
        if (initType.isNull()) {
            error(decl.getLocation(), "couldn't infer type of '", decl.getName(), "', add a type annotation");
        }

        initType.setMutable(decl.getType().isMutable());
        decl.setType(initType);
    }

    if (!isGlobal) addToSymbolTable(decl);

    if (decl.getInitializer() && !isImplicitlyCopyable(decl.getType())) {
        decl.getInitializer()->markAsMoved();
    }
}

void typecheckFieldDecl(FieldDecl&) {}

std::error_code parseSourcesInDirectoryRecursively(llvm::StringRef directoryPath, Module& module,
                                                   ParserFunction& parse) {
    std::error_code error;
    llvm::sys::fs::recursive_directory_iterator it(directoryPath, error), end;

    for (; it != end; it.increment(error)) {
        if (error) break;

        if (llvm::sys::path::extension(it->path()) == ".delta") {
            parse(it->path(), module);
        }
    }

    return error;
}

llvm::ErrorOr<const Module&> importDeltaModule(SourceFile* importer,
                                               const PackageManifest* manifest,
                                               llvm::ArrayRef<std::string> importSearchPaths,
                                               ParserFunction& parse,
                                               llvm::StringRef moduleExternalName,
                                               llvm::StringRef moduleInternalName = "") {
    if (moduleInternalName.empty()) moduleInternalName = moduleExternalName;

    auto it = allImportedModules.find(moduleInternalName);
    if (it != allImportedModules.end()) {
        if (importer) importer->addImportedModule(it->second);
        return *it->second;
    }

    auto module = std::make_shared<Module>(moduleInternalName);
    std::error_code error;

    if (manifest) {
        for (auto& dependency : manifest->getDeclaredDependencies()) {
            if (dependency.getPackageIdentifier() == moduleInternalName) {
                error = parseSourcesInDirectoryRecursively(dependency.getFileSystemPath(), *module, parse);
                goto done;
            }
        }
    }

    for (llvm::StringRef importPath : importSearchPaths) {
        llvm::sys::fs::directory_iterator it(importPath, error), end;
        for (; it != end; it.increment(error)) {
            if (error) goto done;
            if (!llvm::sys::fs::is_directory(it->path())) continue;
            if (llvm::sys::path::filename(it->path()) != moduleExternalName) continue;

            error = parseSourcesInDirectoryRecursively(it->path(), *module, parse);
            goto done;
        }
    }

done:
    if (error || module->getSourceFiles().empty()) {
        return error;
    }

    if (importer) importer->addImportedModule(module);
    allImportedModules[module->getName()] = module;
    typecheckModule(*module, /* TODO: Pass the package manifest of `module` here. */ nullptr,
                    importSearchPaths, parse);
    return *module;
}

void TypeChecker::typecheckImportDecl(ImportDecl& decl, const PackageManifest* manifest,
                                      llvm::ArrayRef<std::string> importSearchPaths,
                                      ParserFunction& parse) const {
    if (importDeltaModule(currentSourceFile, manifest, importSearchPaths, parse, decl.getTarget())) {
        return;
    }

    if (!importCHeader(*currentSourceFile, decl.getTarget(), importSearchPaths)) {
        llvm::errs() << "error: couldn't find module or C header '" << decl.getTarget() << "'\n";
        abort();
    }
}

void TypeChecker::typecheckTopLevelDecl(Decl& decl, const PackageManifest* manifest,
                                        llvm::ArrayRef<std::string> importSearchPaths,
                                        ParserFunction& parse) const {
    switch (decl.getKind()) {
        case DeclKind::ParamDecl: llvm_unreachable("no top-level parameter declarations");
        case DeclKind::FunctionDecl: typecheckFunctionLikeDecl(llvm::cast<FunctionDecl>(decl)); break;
        case DeclKind::MethodDecl: llvm_unreachable("no top-level method declarations");
        case DeclKind::GenericParamDecl: llvm_unreachable("no top-level parameter declarations");
        case DeclKind::InitDecl: llvm_unreachable("no top-level initializer declarations");
        case DeclKind::DeinitDecl: llvm_unreachable("no top-level deinitializer declarations");
        case DeclKind::TypeDecl: typecheckTypeDecl(llvm::cast<TypeDecl>(decl)); break;
        case DeclKind::VarDecl: typecheckVarDecl(llvm::cast<VarDecl>(decl), true); break;
        case DeclKind::FieldDecl: llvm_unreachable("no top-level field declarations");
        case DeclKind::ImportDecl: typecheckImportDecl(llvm::cast<ImportDecl>(decl), manifest,
                                                       importSearchPaths, parse); break;
    }
}

void TypeChecker::typecheckMemberDecl(Decl& decl) const {
    switch (decl.getKind()) {
        case DeclKind::MethodDecl: typecheckFunctionLikeDecl(llvm::cast<MethodDecl>(decl)); break;
        case DeclKind::InitDecl: typecheckInitDecl(llvm::cast<InitDecl>(decl)); break;
        case DeclKind::DeinitDecl: typecheckFunctionLikeDecl(llvm::cast<DeinitDecl>(decl)); break;
        case DeclKind::FieldDecl: typecheckFieldDecl(llvm::cast<FieldDecl>(decl)); break;
        default: llvm_unreachable("invalid member declaration kind");
    }
}

void TypeChecker::postProcess() {
    SAVE_STATE(currentGenericArgs);
    SAVE_STATE(typecheckingGenericFunction);
    typecheckingGenericFunction = true;

    while (!genericFunctionInstantiationsToTypecheck.empty()) {
        auto genericFunctionInstantiations = std::move(genericFunctionInstantiationsToTypecheck);
        for (auto& functionDeclAndGenericArgs : genericFunctionInstantiations) {
            currentGenericArgs = functionDeclAndGenericArgs.second;
            // TODO: Don't typecheck more than once with the same generic arguments.
            typecheckFunctionLikeDecl(functionDeclAndGenericArgs.first);
        }
    }
}

void delta::typecheckModule(Module& module, const PackageManifest* manifest,
                            llvm::ArrayRef<std::string> importSearchPaths,
                            ParserFunction& parse) {
    auto stdlibModule = importDeltaModule(nullptr, nullptr, importSearchPaths, parse, "stdlib", "std");
    if (!stdlibModule) {
        printErrorAndExit("couldn't import the standard library: ", stdlibModule.getError().message());
    }

    // Infer the types of global variables for use before their declaration.
    for (auto& sourceFile : module.getSourceFiles()) {
        TypeChecker typeChecker(&module, &sourceFile);

        for (auto& decl : sourceFile.getTopLevelDecls()) {
            if (auto* varDecl = llvm::dyn_cast<VarDecl>(decl.get())) {
                typeChecker.typecheckVarDecl(*varDecl, true);
            }
        }

        typeChecker.postProcess();
    }

    for (auto& sourceFile : module.getSourceFiles()) {
        TypeChecker typeChecker(&module, &sourceFile);

        for (auto& decl : sourceFile.getTopLevelDecls()) {
            if (!decl->isVarDecl()) {
                typeChecker.typecheckTopLevelDecl(*decl, manifest, importSearchPaths, parse);
            }
        }

        typeChecker.postProcess();
    }
}
