#include "typecheck.h"
#pragma warning(push, 0)
#include <llvm/ADT/SmallPtrSet.h>
#include <llvm/Support/SaveAndRestore.h>
#pragma warning(pop)
#include "c-import.h"
#include "../ast/module.h"

using namespace delta;

void Typechecker::typecheckType(Type type, AccessLevel userAccessLevel) {
    switch (type.getKind()) {
        case TypeKind::BasicType: {
            if (type.isBuiltinType()) {
                validateGenericArgCount(0, type.getGenericArgs(), type.getName(), type.getLocation());
                break;
            }

            auto* basicType = llvm::cast<BasicType>(type.getBase());

            for (auto genericArg : basicType->getGenericArgs()) {
                typecheckType(genericArg, userAccessLevel);
            }

            auto decls = findDecls(basicType->getQualifiedName());
            Decl* decl;

            if (decls.empty()) {
                auto decls = findDecls(basicType->getName());

                if (decls.empty()) {
                    ERROR(type.getLocation(), "unknown type '" << type << "'");
                }

                ASSERT(decls.size() == 1);
                decl = decls[0];
                auto typeTemplate = llvm::cast<TypeTemplate>(decl)->instantiate(basicType->getGenericArgs());
                getCurrentModule()->addToSymbolTable(*typeTemplate);
                typecheckTypeDecl(*typeTemplate);
            } else {
                ASSERT(decls.size() == 1);
                decl = decls[0];

                switch (decl->getKind()) {
                    case DeclKind::TypeDecl:
                    case DeclKind::EnumDecl:
                        break;
                    case DeclKind::TypeTemplate:
                        validateGenericArgCount(llvm::cast<TypeTemplate>(decl)->getGenericParams().size(), basicType->getGenericArgs(),
                                                basicType->getName(), type.getLocation());
                        break;
                    default:
                        ERROR(type.getLocation(), "'" << type << "' is not a type");
                }
            }

            checkHasAccess(*decl, type.getLocation(), userAccessLevel);
            break;
        }
        case TypeKind::ArrayType:
            if (type.isArrayWithRuntimeSize()) {
                auto qualifiedTypeName = getQualifiedTypeName("ArrayRef", type.getElementType());
                if (findDecls(qualifiedTypeName).empty()) {
                    auto* arrayRef = llvm::cast<TypeTemplate>(findDecl("ArrayRef", SourceLocation()));
                    auto* instantiation = arrayRef->instantiate({ type.getElementType() });
                    getCurrentModule()->addToSymbolTable(*instantiation);
                    declsToTypecheck.push_back(instantiation);
                }
            }
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
            if (type.getPointee().isArrayWithRuntimeSize()) {
                ERROR(type.getLocation(), "pointer to array reference is not yet implemented");
            } else {
                typecheckType(type.getPointee(), userAccessLevel);
            }
            break;
        }
        case TypeKind::OptionalType:
            typecheckType(type.getWrappedType(), userAccessLevel);
            break;
    }
}

void Typechecker::typecheckParamDecl(ParamDecl& decl, AccessLevel userAccessLevel) {
    if (getCurrentModule()->getSymbolTable().containsInCurrentScope(decl.getName())) {
        ERROR(decl.getLocation(), "redefinition of '" << decl.getName() << "'");
    }

    typecheckType(decl.getType(), userAccessLevel);
    getCurrentModule()->getSymbolTable().add(decl.getName(), &decl);
}

static bool allPathsReturn(llvm::ArrayRef<Stmt*> block) {
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
            return llvm::all_of(switchStmt.getCases(), [](auto& c) { return allPathsReturn(c.getStmts()); }) &&
                   allPathsReturn(switchStmt.getDefaultStmts());
        }
        default:
            return false;
    }
}

void Typechecker::typecheckGenericParamDecls(llvm::ArrayRef<GenericParamDecl> genericParams, AccessLevel userAccessLevel) {
    for (auto& genericParam : genericParams) {
        if (getCurrentModule()->getSymbolTable().contains(genericParam.getName())) {
            REPORT_ERROR(genericParam.getLocation(), "redefinition of '" << genericParam.getName() << "'");
        }

        for (Type constraint : genericParam.getConstraints()) {
            try {
                typecheckType(constraint, userAccessLevel);

                if (!constraint.getDecl()->isInterface()) {
                    ERROR(constraint.getLocation(), "only interface types can be used as generic constraints");
                }
            } catch (const CompileError& error) {
                error.print();
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
    if (decl.isTypechecked()) return;
    if (decl.isExtern()) return; // TODO: Typecheck parameters and return type of extern functions.

    TypeDecl* receiverTypeDecl = decl.getTypeDecl();

    Scope scope(&decl, &currentModule->getSymbolTable());
    llvm::SaveAndRestore setCurrentFunction(currentFunction, &decl);

    typecheckParams(decl.getParams(), decl.getAccessLevel());

    if (decl.isLambda()) {
        ASSERT(decl.getBody().size() == 1);
        decl.getProto().setReturnType(typecheckExpr(*llvm::cast<ReturnStmt>(decl.getBody().front())->getReturnValue()));
    }

    if (!decl.isConstructorDecl() && !decl.isDestructorDecl()) {
        typecheckType(decl.getReturnType(), decl.getAccessLevel());
    }

    if (!decl.isExtern()) {
        llvm::SmallPtrSet<FieldDecl*, 32> initializedFields;
        llvm::SaveAndRestore setReturnType(functionReturnType, decl.getReturnType());
        llvm::SaveAndRestore setInitializedFields(currentInitializedFields, &initializedFields);

        if (receiverTypeDecl) {
            Type thisType = receiverTypeDecl->getTypeForPassing();
            auto* varDecl = new VarDecl(thisType, "this", nullptr, &decl, AccessLevel::None, *getCurrentModule(), decl.getLocation());
            getCurrentModule()->addToSymbolTable(varDecl);
        }

        bool delegatedInit = false;

        if (decl.hasBody()) {
            for (auto& stmt : decl.getBody()) {
                {
                    llvm::SaveAndRestore setCurrentStmt(currentStmt, stmt);
                    typecheckStmt(stmt);
                }

                if (decl.isConstructorDecl()) {
                    if (auto* exprStmt = llvm::dyn_cast<ExprStmt>(stmt)) {
                        if (auto* callExpr = llvm::dyn_cast<CallExpr>(&exprStmt->getExpr())) {
                            if (auto* constructorDecl = llvm::dyn_cast_or_null<ConstructorDecl>(callExpr->getCalleeDecl())) {
                                if (constructorDecl->getTypeDecl() == receiverTypeDecl ||
                                    receiverTypeDecl->hasInterface(*constructorDecl->getTypeDecl())) {
                                    delegatedInit = true;
                                }
                            }
                        }
                    }
                }
            }

            // This prevents creating destructors calls during codegen.
            for (auto* movedDecl : movedDecls) {
                switch (movedDecl->getKind()) {
                    case DeclKind::ParamDecl:
                        llvm::cast<ParamDecl>(movedDecl)->setMoved(true);
                        break;
                    case DeclKind::VarDecl:
                        llvm::cast<VarDecl>(movedDecl)->setMoved(true);
                        break;
                    default:
                        break;
                }
            }
            movedDecls.clear();
        }

        if (decl.isConstructorDecl() && !delegatedInit) {
            for (auto& field : decl.getTypeDecl()->getFields()) {
                if (!field.getDefaultValue() && initializedFields.count(&field) == 0) {
                    WARN(decl.getLocation(), "constructor doesn't initialize member variable '" << field.getName() << "'");
                }
            }
        }
    }

    if ((!receiverTypeDecl || !receiverTypeDecl->isInterface()) && !decl.getReturnType().isVoid() && !allPathsReturn(decl.getBody())) {
        REPORT_ERROR(decl.getLocation(), "'" << decl.getName() << "' is missing a return statement");
    }

    decl.setTypechecked(true);
}

void Typechecker::typecheckFunctionTemplate(FunctionTemplate& decl) {
    typecheckGenericParamDecls(decl.getGenericParams(), decl.getAccessLevel());
}

void Typechecker::typecheckTypeDecl(TypeDecl& decl) {
    for (Type interface : decl.getInterfaces()) {
        typecheckType(interface, decl.getAccessLevel());
        auto* interfaceDecl = interface.getDecl();

        if (!interfaceDecl->isInterface()) {
            REPORT_ERROR(interface.getLocation(), "'" << interface << "' is not an interface");
            continue;
        }

        std::string errorReason;
        if (!providesInterfaceRequirements(decl, *interfaceDecl, &errorReason)) {
            REPORT_ERROR(decl.getLocation(),
                         "'" << decl.getName() << "' " << errorReason << " required by interface '" << interfaceDecl->getName() << "'");
        }
    }

    TypeDecl* realDecl;

    if (decl.isInterface()) {
        // TODO: Move this to typecheckModule to the pre-typechecking phase?
        realDecl = llvm::cast<TypeDecl>(decl.instantiate({ { "This", decl.getType() } }, {}));
    } else {
        realDecl = &decl;
    }

    for (auto& fieldDecl : realDecl->getFields()) {
        typecheckFieldDecl(fieldDecl);
    }

    for (auto& methodDecl : realDecl->getMethods()) {
        typecheckMethodDecl(*methodDecl);
    }
}

void Typechecker::typecheckTypeTemplate(TypeTemplate& decl) {
    typecheckGenericParamDecls(decl.getGenericParams(), decl.getAccessLevel());
}

void Typechecker::typecheckEnumDecl(EnumDecl& decl) {
    std::vector<const EnumCase*> cases = map(decl.getCases(), [](const EnumCase& c) { return &c; });
    std::sort(cases.begin(), cases.end(), [](auto* a, auto* b) { return a->getName() < b->getName(); });
    auto it = std::adjacent_find(cases.begin(), cases.end(), [](auto* a, auto* b) { return a->getName() == b->getName(); });

    if (it != cases.end()) {
        ERROR((*it)->getLocation(), "duplicate enum case '" << (*it)->getName() << "'");
    }

    for (auto& enumCase : decl.getCases()) {
        typecheckExpr(*enumCase.getValue());

        if (enumCase.getAssociatedType()) {
            typecheckType(enumCase.getAssociatedType(), enumCase.getAccessLevel());
        }
    }
}

void Typechecker::typecheckVarDecl(VarDecl& decl, bool isGlobal) {
    if (!isGlobal && getCurrentModule()->getSymbolTable().contains(decl.getName())) {
        ERROR(decl.getLocation(), "redefinition of '" << decl.getName() << "'");
    }

    if (isGlobal && decl.getInitializer()->isUndefinedLiteralExpr()) {
        ERROR(decl.getLocation(), "global variables cannot be uninitialized");
    }

    Type declaredType = decl.getType();

    try {
        typecheckExpr(*decl.getInitializer(), false, declaredType);
    } catch (const CompileError&) {
        if (!isGlobal) getCurrentModule()->addToSymbolTable(decl, false);
        throw;
    }

    if (!isGlobal) getCurrentModule()->addToSymbolTable(decl, false);

    Type initializerType = decl.getInitializer()->getType();
    if (!initializerType) return;

    if (declaredType) {
        bool isLocalVariable = decl.getParent() && decl.getParent()->isFunctionDecl();
        typecheckType(declaredType, isLocalVariable ? AccessLevel::None : decl.getAccessLevel());

        if (!convert(decl.getInitializer(), declaredType)) {
            const char* hint = "";

            if (initializerType.isNull()) {
                ASSERT(!declaredType.isOptionalType());
                hint = " (add '?' to the type to make it nullable)";
            }

            ERROR(decl.getInitializer()->getLocation(), "cannot assign '" << initializerType << "' to '" << declaredType << "'" << hint);
        }
    } else {
        if (initializerType.isNull()) {
            ERROR(decl.getLocation(), "couldn't infer type of '" << decl.getName() << "', add a type annotation");
        }

        decl.setType(initializerType.withMutability(decl.getType().getMutability()));
    }

    if (!decl.getType().isImplicitlyCopyable()) {
        setMoved(decl.getInitializer(), true);
    }
}

void Typechecker::typecheckFieldDecl(FieldDecl& decl) {
    typecheckType(decl.getType(), std::min(decl.getAccessLevel(), decl.getParent()->getAccessLevel()));
}

void Typechecker::typecheckImportDecl(ImportDecl& decl, const PackageManifest* manifest) {
    // TODO: Print import search paths as part of the below error messages.

    if (decl.getTarget().endswith(".h")) {
        if (!importCHeader(*currentSourceFile, decl.getTarget(), options)) {
            REPORT_ERROR(decl.getLocation(), "couldn't find C header file '" << decl.getTarget() << "'");
        }
    } else {
        auto module = importDeltaModule(currentSourceFile, manifest, decl.getTarget());
        if (!module) {
            REPORT_ERROR(decl.getLocation(), "couldn't import module '" << decl.getTarget() << "': " << module.getError().message());
        }
    }
}

void Typechecker::typecheckTopLevelDecl(Decl& decl, const PackageManifest* manifest) {
    switch (decl.getKind()) {
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
            typecheckVarDecl(llvm::cast<VarDecl>(decl), true);
            break;
        case DeclKind::FieldDecl:
            llvm_unreachable("no top-level field declarations");
        case DeclKind::ImportDecl:
            typecheckImportDecl(llvm::cast<ImportDecl>(decl), manifest);
            break;
    }
}

void Typechecker::typecheckMethodDecl(Decl& decl) {
    switch (decl.getKind()) {
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
