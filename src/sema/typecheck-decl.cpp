#include "typecheck.h"
#include <algorithm>
#pragma warning(push, 0)
#include <llvm/ADT/SmallPtrSet.h>
#include <llvm/Support/SaveAndRestore.h>
#pragma warning(pop)
#include "../ast/module.h"
#include "c-import.h"

using namespace cx;

// Finds the type template to instantiate for a generic type name. A same-named function
// doesn't prevent using the type in type position.
static TypeTemplate* findTypeTemplateForGenericArgs(Type type, std::vector<Decl*> decls) {
    decls.erase(std::remove_if(decls.begin(), decls.end(), [](Decl* d) { return !d->isTypeTemplate() && !d->isTypeDecl(); }), decls.end());

    if (decls.empty()) {
        ERROR(type.getLocation(), "'" << type << "' is not a type");
    }

    if (!decls[0]->isTypeTemplate()) {
        ERROR(type.getLocation(), "too many generic arguments to '" << type.getName() << "', expected 0");
    }

    ASSERT(decls.size() == 1);
    return llvm::cast<TypeTemplate>(decls[0]);
}

void Typechecker::typecheckType(Type type, AccessLevel userAccessLevel) {
    switch (type.getKind()) {
    case TypeKind::BasicType: {
        Decl* decl;
        auto* basicType = llvm::cast<BasicType>(type.getBase());
        if (basicType->decl) {
            decl = basicType->decl;
        } else {
            if (basicType->name.empty()) break; // Nothing to type-check.

            if (!type.isOptionalType() && type.isBuiltinType()) {
                validateGenericArgCount(0, type.getGenericArgs(), type.getName(), type.getLocation());
                break;
            }

            for (auto genericArg : basicType->getGenericArgs()) {
                typecheckType(genericArg, userAccessLevel);
            }

            auto decls = findDecls(basicType->getQualifiedName());

            if (decls.empty()) {
                // For generic types, search again with the base name (without generic args).
                auto decls = findDecls(basicType->getName());

                if (decls.empty()) {
                    ERROR(type.getLocation(), "unknown type '" << type << "'");
                }
                auto* typeTemplate = findTypeTemplateForGenericArgs(type, std::move(decls));
                decl = typeTemplate;
                ASSERT(!basicType->getGenericArgs().empty());
                auto instantiation = typeTemplate->instantiate(basicType->getGenericArgs());
                getCurrentModule()->addToSymbolTable(*instantiation);
                deferTypechecking(instantiation);
                checkHasAccess(*decl, type.getLocation(), userAccessLevel);
                break;
            } else if (decls.size() > 1) {
                ERROR(type.getLocation(), "ambiguous reference to '" << type.getName() << "'"); // TODO: add candidate notes
            } else {
                decl = decls.front();
            }
        }

        if (decl->isTypeTemplate()) {
            validateGenericArgCount(llvm::cast<TypeTemplate>(decl)->genericParams.size(), basicType->getGenericArgs(), basicType->getName(),
                                    type.getLocation());
        } else if (!decl->isTypeDecl()) {
            ERROR(type.getLocation(), "'" << type << "' is not a type");
        }

        checkHasAccess(*decl, type.getLocation(), userAccessLevel);
        break;
    }
    case TypeKind::ArrayType:
        typecheckType(type.getElementType(), userAccessLevel);
        break;
    case TypeKind::TupleType:
        for (auto& element : type.getTupleElements()) {
            typecheckType(element.type, userAccessLevel);
        }
        break;
    case TypeKind::FunctionType:
        for (auto paramType : type.getParamTypes()) {
            typecheckType(paramType, userAccessLevel);
        }
        typecheckType(type.getReturnType(), userAccessLevel);
        break;
    case TypeKind::PointerType: {
        typecheckType(type.getPointee(), userAccessLevel);
        break;
    }
    case TypeKind::UnresolvedType:
        llvm_unreachable("invalid unresolved type");
    }
}

void Typechecker::typecheckParamDecl(ParamDecl& decl, AccessLevel userAccessLevel) {
    if (auto existing = getCurrentModule()->getSymbolTable().findInCurrentScope(decl.getName()); !existing.empty()) {
        ERROR_WITH_NOTES(decl.getLocation(), getPreviousDefinitionNotes(existing), "redefinition of '" << decl.getName() << "'");
    }

    typecheckType(decl.type, userAccessLevel);
    getCurrentModule()->getSymbolTable().add(decl.getName(), &decl);
}

static bool allPathsReturn(llvm::ArrayRef<Stmt*> block) {
    if (block.empty()) return false;

    switch (block.back()->kind) {
    case StmtKind::ReturnStmt:
        return true;
    case StmtKind::ExprStmt: {
        auto& exprStmt = llvm::cast<ExprStmt>(*block.back());
        auto call = llvm::dyn_cast<CallExpr>(exprStmt.expr);
        return call && call->getType().isNeverType();
    }
    case StmtKind::IfStmt: {
        auto& ifStmt = llvm::cast<IfStmt>(*block.back());
        return allPathsReturn(ifStmt.thenBody) && allPathsReturn(ifStmt.elseBody);
    }
    case StmtKind::SwitchStmt: {
        auto& switchStmt = llvm::cast<SwitchStmt>(*block.back());
        return llvm::all_of(switchStmt.cases, [](SwitchCase& c) { return allPathsReturn(c.stmts); }) && allPathsReturn(switchStmt.defaultStmts);
    }
    default:
        return false;
    }
}

void Typechecker::typecheckGenericParamDecls(llvm::ArrayRef<GenericParamDecl> genericParams, AccessLevel userAccessLevel) {
    for (auto& genericParam : genericParams) {
        if (auto existing = getCurrentModule()->getSymbolTable().findFirst(genericParam.getName()); !existing.empty()) {
            ERROR_WITH_NOTES(genericParam.getLocation(), getPreviousDefinitionNotes(existing), "redefinition of '" << genericParam.getName() << "'");
        }

        for (Type constraint : genericParam.constraints) {
            try {
                typecheckType(constraint, userAccessLevel);

                if (!constraint.getDecl()->isInterface()) {
                    ERROR(constraint.getLocation(), "only interface types can be used as generic constraints");
                }
            } catch (const CompileError& error) {
                error.report();
            }
        }
    }
}

void Typechecker::typecheckParams(llvm::MutableArrayRef<ParamDecl> params, AccessLevel userAccessLevel) {
    for (auto& param : params) {
        typecheckParamDecl(param, userAccessLevel);
    }
}

void Typechecker::typecheckFunctionDecl(FunctionDecl& decl) {
    if (decl.typechecked) return;
    if (decl.isExtern()) return; // TODO: Typecheck parameters and return type of extern functions.

    TypeDecl* receiverTypeDecl = decl.getTypeDecl();

    Scope scope(&decl, &currentModule->getSymbolTable());
    llvm::SaveAndRestore setCurrentFunction(currentFunction, &decl);

    typecheckParams(decl.getParams(), decl.accessLevel);

    if (!decl.isConstructorDecl() && !decl.isDestructorDecl() && decl.getReturnType()) {
        typecheckType(decl.getReturnType(), decl.accessLevel);
    }

    if (!decl.isExtern()) {
        llvm::SmallPtrSet<FieldDecl*, 32> initializedFields;
        llvm::SaveAndRestore setInitializedFields(currentInitializedFields, &initializedFields);

        if (receiverTypeDecl) {
            Type thisType = receiverTypeDecl->getTypeForPassing();
            auto* varDecl = new VarDecl(thisType, "this", nullptr, &decl, AccessLevel::None, *getCurrentModule(), decl.getLocation());
            getCurrentModule()->addToSymbolTable(varDecl);
        }

        bool delegatedInit = false;

        if (decl.body) {
            for (auto& stmt : *decl.body) {
                {
                    llvm::SaveAndRestore setCurrentStmt(currentStmt, &stmt);

                    if (!typecheckStmt(stmt) && !decl.getReturnType()) {
                        ASSERT(decl.isLambda());
                        throw CompileError::dependentError();
                    }
                }

                if (decl.isConstructorDecl()) {
                    if (auto* exprStmt = llvm::dyn_cast<ExprStmt>(stmt)) {
                        if (auto* callExpr = llvm::dyn_cast<CallExpr>(exprStmt->expr)) {
                            if (auto* constructorDecl = llvm::dyn_cast_or_null<ConstructorDecl>(callExpr->getCalleeDecl())) {
                                if (constructorDecl->getTypeDecl() == receiverTypeDecl || receiverTypeDecl->hasInterface(*constructorDecl->getTypeDecl())) {
                                    delegatedInit = true;
                                }
                            }
                        }
                    }
                }
            }

            ASSERT(decl.getReturnType());

            // This prevents creating destructors calls during codegen.
            for (auto* movedDecl : movedDecls) {
                switch (movedDecl->kind) {
                case DeclKind::ParamDecl:
                    llvm::cast<ParamDecl>(movedDecl)->moved = true;
                    break;
                case DeclKind::VarDecl:
                    llvm::cast<VarDecl>(movedDecl)->moved = true;
                    break;
                default:
                    break;
                }
            }

            movedDecls.clear();
        }

        if (decl.isConstructorDecl() && !delegatedInit) {
            for (auto& field : decl.getTypeDecl()->fields) {
                if (!field.defaultValue && initializedFields.count(&field) == 0) {
                    WARN(decl.getLocation(), "constructor doesn't initialize member variable '" << field.getName() << "'");
                }
            }
        }
    }

    if ((!receiverTypeDecl || !receiverTypeDecl->isInterface()) && !decl.getReturnType().isVoid() && !allPathsReturn(*decl.body)) {
        if (decl.getReturnType().isNeverType()) {
            WARN(decl.getLocation(), "'" << decl.getName() << "' is declared to never return but it does return");
        } else {
            REPORT_ERROR(decl.getLocation(), "'" << decl.getName() << "' is missing a return statement");
        }
    }

    decl.typechecked = true;
}

void Typechecker::typecheckFunctionTemplate(FunctionTemplate& decl) {
    typecheckGenericParamDecls(decl.genericParams, decl.accessLevel);
}

void Typechecker::typecheckTypeDecl(TypeDecl& decl) {
    for (Type interface : decl.interfaces) {
        typecheckType(interface, decl.accessLevel);
        auto* interfaceDecl = interface.getDecl();

        if (!interfaceDecl->isInterface()) {
            REPORT_ERROR(interface.getLocation(), "'" << interface << "' is not an interface");
            continue;
        }

        std::string errorReason;
        if (!providesInterfaceRequirements(decl, *interfaceDecl, &errorReason)) {
            REPORT_ERROR(decl.getLocation(), "'" << decl.getName() << "' " << errorReason << " required by interface '" << interfaceDecl->getName() << "'");
        }
    }

    TypeDecl* realDecl;

    if (decl.isInterface()) {
        // TODO: Move this to typecheckModule to the pre-typechecking phase?
        realDecl = llvm::cast<TypeDecl>(decl.instantiate({{"This", decl.getType()}}, {}));
    } else {
        realDecl = &decl;
    }

    for (auto& fieldDecl : realDecl->fields) {
        typecheckFieldDecl(fieldDecl);
    }

    for (auto& methodDecl : realDecl->methods) {
        typecheckMethodDecl(*methodDecl);
    }
}

void Typechecker::typecheckTypeTemplate(TypeTemplate& decl) {
    typecheckGenericParamDecls(decl.genericParams, decl.accessLevel);
}

void Typechecker::typecheckEnumDecl(EnumDecl& decl) {
    std::vector<const EnumCase*> cases = map(decl.cases, [](const EnumCase& c) { return &c; });
    std::ranges::sort(cases, [](auto* a, auto* b) { return a->getName() < b->getName(); });
    auto it = std::ranges::adjacent_find(cases, [](auto* a, auto* b) { return a->getName() == b->getName(); });

    if (it != cases.end()) {
        ERROR((*it)->getLocation(), "duplicate enum case '" << (*it)->getName() << "'");
    }

    for (auto& enumCase : decl.cases) {
        typecheckExpr(*enumCase.value);

        if (enumCase.associatedType) {
            typecheckType(enumCase.associatedType, enumCase.accessLevel);
        }
    }
}

void Typechecker::typecheckVarDecl(VarDecl& decl) {
    Type declaredType = decl.type;
    if (declaredType) {
        typecheckType(declaredType, !decl.isGlobal() ? AccessLevel::None : decl.accessLevel);
    }

    if (decl.initializer) {
        try {
            typecheckExpr(*decl.initializer, false, declaredType);
        } catch (const CompileError&) {
            if (!decl.isGlobal()) getCurrentModule()->addToSymbolTable(decl);
            throw;
        }
    }

    if (!decl.isGlobal()) getCurrentModule()->addToSymbolTable(decl);
    if (!decl.initializer) return;
    Type initializerType = decl.initializer->getType();
    if (!initializerType) return;

    if (declaredType) {
        if (auto converted = convert(decl.initializer, declaredType)) {
            decl.initializer = converted;
        } else {
            const char* hint = "";

            if (initializerType.isNull()) {
                ASSERT(!declaredType.isOptionalType());
                hint = " (add '?' to the type to make it nullable)";
            }

            ERROR(decl.initializer->getLocation(), "cannot assign '" << initializerType << "' to '" << declaredType << "'" << hint);
        }
    } else {
        if (initializerType.isNull()) {
            ERROR(decl.getLocation(), "couldn't infer type of '" << decl.getName() << "', add a type annotation");
        }

        decl.type = NOTNULL(initializerType.withMutability(decl.type.getMutability()));
    }

    if (!decl.type.isImplicitlyCopyable()) {
        setMoved(decl.initializer, true);
    }
}

void Typechecker::typecheckFieldDecl(FieldDecl& decl) {
    typecheckType(decl.type, std::min(decl.accessLevel, decl.getParentDecl()->accessLevel));
}

void Typechecker::typecheckImportDecl(ImportDecl& decl, const PackageManifest* manifest) {
    // TODO: Print import search paths as part of the below error messages.

    if (decl.target.ends_with(".h")) {
        if (!importCHeader(*currentSourceFile, decl, *this)) {
            REPORT_ERROR(decl.getLocation(), "couldn't import C header file '" << decl.target << "'");
        }
    } else {
        auto module = importModule(currentSourceFile, manifest, decl.target);
        if (!module) {
            REPORT_ERROR(decl.getLocation(), "couldn't import module '" << decl.target << "': " << module.getError().message());
        }
    }
}

void Typechecker::typecheckTopLevelDecl(Decl& decl, const PackageManifest* manifest) {
    switch (decl.kind) {
    case DeclKind::ParamDecl:
        llvm_unreachable("no top-level parameter declarations");
    case DeclKind::FunctionDecl:
        typecheckFunctionDecl(llvm::cast<FunctionDecl>(decl));
        break;
    case DeclKind::MethodDecl:
        llvm_unreachable("no top-level method declarations");
    case DeclKind::GenericParamDecl:
        llvm_unreachable("no top-level parameter declarations");
    case DeclKind::ConstructorDecl:
        llvm_unreachable("no top-level constructor declarations");
    case DeclKind::DestructorDecl:
        llvm_unreachable("no top-level destructor declarations");
    case DeclKind::FunctionTemplate:
        typecheckFunctionTemplate(llvm::cast<FunctionTemplate>(decl));
        break;
    case DeclKind::TypeDecl:
        typecheckTypeDecl(llvm::cast<TypeDecl>(decl));
        break;
    case DeclKind::TypeTemplate:
        typecheckTypeTemplate(llvm::cast<TypeTemplate>(decl));
        break;
    case DeclKind::EnumDecl:
        typecheckEnumDecl(llvm::cast<EnumDecl>(decl));
        break;
    case DeclKind::EnumCase:
        llvm_unreachable("no top-level enum case declarations");
    case DeclKind::VarDecl:
        typecheckVarDecl(llvm::cast<VarDecl>(decl));
        break;
    case DeclKind::FieldDecl:
        llvm_unreachable("no top-level field declarations");
    case DeclKind::ImportDecl:
        typecheckImportDecl(llvm::cast<ImportDecl>(decl), manifest);
        break;
    }
}

void Typechecker::typecheckMethodDecl(Decl& decl) {
    switch (decl.kind) {
    case DeclKind::MethodDecl:
    case DeclKind::ConstructorDecl:
    case DeclKind::DestructorDecl:
        typecheckFunctionDecl(llvm::cast<MethodDecl>(decl));
        break;
    case DeclKind::FunctionTemplate:
        typecheckFunctionTemplate(llvm::cast<FunctionTemplate>(decl));
        break;
    default:
        llvm_unreachable("invalid method declaration kind");
    }
}
