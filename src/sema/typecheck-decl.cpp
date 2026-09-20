#include "typecheck.h"
#include <algorithm>
#pragma warning(push, 0)
#include <llvm/ADT/SmallPtrSet.h>
#include <llvm/Support/SaveAndRestore.h>
#pragma warning(pop)
#include "../ast/arena.h"
#include "../ast/module.h"
#include "../driver/driver.h"
#include "c-import.h"

using namespace cx;

static std::vector<Note> getTypeCandidateNotes(llvm::ArrayRef<Decl*> candidates) {
    bool multipleModules = candidates.size() > 1 && llvm::any_of(candidates, [&](Decl* c) { return c->getModule() != candidates[0]->getModule(); });

    return map(candidates, [&](Decl* c) {
        auto message = "candidate type" + (multipleModules && c->getModule() ? " in module '" + c->getModule()->name + "'" : "") + ":";
        return Note{c->getLocation(), std::move(message)};
    });
}

// Finds the type template to instantiate for a generic type name. A same-named function
// doesn't prevent using the type in type position.
static TypeTemplate* findTypeTemplateForGenericArgs(Type type, std::vector<Decl*> decls) {
    decls.erase(std::remove_if(decls.begin(), decls.end(), [](Decl* d) { return !d->isTypeTemplate() && !d->isTypeDecl(); }), decls.end());

    if (decls.empty()) {
        ERROR(type.location, "'" << type << "' is not a type");
    }

    if (!decls[0]->isTypeTemplate()) {
        ERROR(type.location, "too many generic arguments to '" << type.getName() << "', expected 0");
    }

    if (decls.size() > 1) {
        ERROR_WITH_NOTES(type.location, getTypeCandidateNotes(decls), "ambiguous reference to '" << type.getName() << "'");
    }

    return llvm::cast<TypeTemplate>(decls[0]);
}

// Returns true if values of the given type transitively contain the target type declaration without pointer indirection,
// meaning the target type would have infinite size. `visiting` holds the declarations on the current search path.
static bool containsItselfByValue(Type type, const TypeDecl& target, llvm::SmallPtrSetImpl<const TypeDecl*>& visiting) {
    // Pointers, unsized arrays, and functions are pointer-sized (see getIRType), as are builtins.
    if (!type || type.isBuiltinType() || type.isFunctionType() || type.isImplementedAsPointer()) return false;

    if (type.isArrayType()) {
        if (type.getArraySize() == 0) return false; // Zero-sized arrays occupy no storage.
        return containsItselfByValue(type.getElementType(), target, visiting);
    }

    if (type.isTupleType()) {
        return llvm::any_of(type.getTupleElements(), [&](const TupleElement& element) { return containsItselfByValue(element.type, target, visiting); });
    }

    if (type.isOptionalType()) {
        return containsItselfByValue(type.getWrappedType(), target, visiting);
    }

    auto* typeDecl = type.getDecl();
    if (!typeDecl) return false;
    if (typeDecl == &target) return true;
    if (!visiting.insert(typeDecl).second) return true; // Found a by-value cycle; anything on it is infinitely sized.
    bool result = false;
    if (auto* enumDecl = llvm::dyn_cast<EnumDecl>(typeDecl)) {
        result = llvm::any_of(enumDecl->cases, [&](const EnumCase& enumCase) { return containsItselfByValue(enumCase.associatedType, target, visiting); });
    } else {
        result = llvm::any_of(typeDecl->fields, [&](const FieldDecl& field) { return containsItselfByValue(field.type, target, visiting); });
    }
    visiting.erase(typeDecl);
    return result;
}

static void checkForInfiniteSize(const TypeDecl& target, llvm::ArrayRef<Type> memberTypes) {
    llvm::SmallPtrSet<const TypeDecl*, 8> visiting;
    visiting.insert(&target);
    if (llvm::any_of(memberTypes, [&](Type type) { return containsItselfByValue(type, target, visiting); })) {
        ERROR(target.getLocation(), "'" << target.getName() << "' has infinite size because it contains itself");
    }
}

void Typechecker::typecheckType(Type type, AccessLevel userAccessLevel, bool recheckGenericArgs) {
    switch (type.getKind()) {
    case TypeKind::BasicType: {
        Decl* decl;
        auto* basicType = llvm::cast<BasicType>(type.typeBase);
        if (basicType->decl) {
            decl = basicType->decl;

            // Check generic arguments on repeat uses too: resolving basicType->decl above
            // skips the lookup below, which would otherwise silence access warnings after
            // the first use. Type nodes are interned, so the stored arguments carry the
            // first use's locations; relocate them to the current use. This is exact when
            // the nested types start where the outer type starts (e.g. 'A' in 'A*?').
            if (recheckGenericArgs) {
                for (auto genericArg : basicType->genericArgs) {
                    typecheckType(genericArg.withLocation(type.location), userAccessLevel);
                }
            }
        } else {
            if (basicType->name.empty()) break; // Nothing to type-check.

            if (!type.isOptionalType() && type.isBuiltinType()) {
                validateGenericArgCount(0, type.getGenericArgs(), type.getName(), type.location);
                break;
            }

            for (auto genericArg : basicType->genericArgs) {
                typecheckType(genericArg, userAccessLevel);
            }

            auto decls = findDecls(basicType->getQualifiedName());

            if (decls.empty()) {
                // For generic types, search again with the base name (without generic args).
                auto decls = findDecls(basicType->name);

                if (decls.empty()) {
                    ERROR(type.location, "unknown type '" << type << "'");
                }
                auto* typeTemplate = findTypeTemplateForGenericArgs(type, std::move(decls));
                decl = typeTemplate;
                ASSERT(!basicType->genericArgs.empty());
                auto instantiation = typeTemplate->instantiate(basicType->genericArgs);
                currentModule->addToSymbolTable(*instantiation);
                deferTypechecking(instantiation);
                checkHasAccess(*decl, type.location, userAccessLevel);
                break;
            } else if (decls.size() > 1) {
                ERROR_WITH_NOTES(type.location, getTypeCandidateNotes(decls), "ambiguous reference to '" << type.getName() << "'");
            } else {
                decl = decls.front();
            }
        }

        if (decl->isTypeTemplate()) {
            validateGenericArgCount(llvm::cast<TypeTemplate>(decl)->genericParams.size(), basicType->genericArgs, basicType->name, type.location);
        } else if (!decl->isTypeDecl()) {
            ERROR(type.location, "'" << type << "' is not a type");
        }

        checkHasAccess(*decl, type.location, userAccessLevel);
        break;
    }
    case TypeKind::ArrayType:
        typecheckType(type.getElementType(), userAccessLevel, recheckGenericArgs);
        break;
    case TypeKind::TupleType:
        for (auto& element : type.getTupleElements()) {
            typecheckType(element.type, userAccessLevel, recheckGenericArgs);
        }
        break;
    case TypeKind::FunctionType:
        for (auto paramType : type.getParamTypes()) {
            typecheckType(paramType, userAccessLevel, recheckGenericArgs);
        }
        typecheckType(type.getReturnType(), userAccessLevel, recheckGenericArgs);
        break;
    case TypeKind::PointerType: {
        typecheckType(type.getPointee(), userAccessLevel, recheckGenericArgs);
        break;
    }
    case TypeKind::UnresolvedType:
        llvm_unreachable("invalid unresolved type");
    }
}

void Typechecker::typecheckParamDecl(ParamDecl& decl, AccessLevel userAccessLevel) {
    if (!decl.getName().empty()) {
        if (auto existing = currentModule->symbolTable.findInCurrentScope(decl.getName()); !existing.empty()) {
            ERROR_WITH_NOTES(decl.getLocation(), getPreviousDefinitionNotes(existing), "redefinition of '" << decl.getName() << "'");
        }
    }

    typecheckType(decl.type, userAccessLevel);
    if (!decl.getName().empty()) {
        currentModule->symbolTable.add(decl.getName(), &decl);
    }
}

static bool allPathsReturn(llvm::ArrayRef<Stmt*> block) {
    if (block.empty()) return false;

    switch (block.back()->kind) {
    case StmtKind::ReturnStmt:
        return true;
    case StmtKind::ExprStmt: {
        auto& exprStmt = llvm::cast<ExprStmt>(*block.back());
        auto call = llvm::dyn_cast<CallExpr>(exprStmt.expr);
        if (!call) return false;
        // The call can lack a type if it failed to typecheck (the error was
        // already reported by typecheckStmt, which continues with the rest of
        // the function body instead of bailing out).
        if (call->type && call->type.isNeverType()) return true;
        // Builtin `assert(false)` branches to `assertFail`, which aborts, so it terminates all paths.
        if (!call->isMethodCall() && call->getFunctionName() == "assert" && !call->args.empty()) {
            if (auto* condition = llvm::dyn_cast<BoolLiteralExpr>(call->args[0].value)) {
                return !condition->value;
            }
        }
        return false;
    }
    case StmtKind::IfStmt: {
        auto& ifStmt = llvm::cast<IfStmt>(*block.back());
        return allPathsReturn(ifStmt.thenBody) && allPathsReturn(ifStmt.elseBody);
    }
    case StmtKind::SwitchStmt: {
        auto& switchStmt = llvm::cast<SwitchStmt>(*block.back());
        if (!llvm::all_of(switchStmt.cases, [](SwitchCase& c) { return allPathsReturn(c.stmts); })) return false;
        if (switchStmt.defaultStmts.empty()) return switchStmt.coversAllEnumCases;
        return allPathsReturn(switchStmt.defaultStmts);
    }
    default:
        return false;
    }
}

void Typechecker::typecheckGenericParamDecls(llvm::ArrayRef<GenericParamDecl> genericParams, AccessLevel userAccessLevel) {
    for (auto& genericParam : genericParams) {
        if (auto existing = currentModule->symbolTable.findFirst(genericParam.getName()); !existing.empty()) {
            ERROR_WITH_NOTES(genericParam.getLocation(), getPreviousDefinitionNotes(existing), "redefinition of '" << genericParam.getName() << "'");
        }

        for (Type constraint : genericParam.constraints) {
            try {
                const int errorsBefore = errors;
                typecheckType(constraint, userAccessLevel);

                if (errors == errorsBefore && !constraint.getDecl()->isInterface()) {
                    ERROR(constraint.location, "only interface types can be used as generic constraints");
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
    int errorsBefore = errors;
    llvm::SaveAndRestore saveNarrowings(narrowedTypes, NarrowMap{});
    // Lambda bodies are checked inline within the enclosing function; moves they record
    // must not clobber the enclosing move state, which is restored when the body is done.
    llvm::SaveAndRestore saveMovedDecls(movedDecls, movedDecls);
    // 'break' and 'continue' must not cross function boundaries into enclosing loops or switches.
    llvm::SaveAndRestore saveControlStmts(currentControlStmts, std::vector<Stmt*>());
    llvm::SaveAndRestore saveLocalVarDecls(localVarDecls, std::vector<VarDecl*>());

    if (decl.hasPack()) {
        ERROR(decl.getPackParam()->getLocation(), "variadic parameter requires a generic function");
    }

    if (decl.isExtern()) {
        Scope scope(&decl, &currentModule->symbolTable);
        llvm::SaveAndRestore setCurrentFunction(currentFunction, &decl);

        typecheckParams(decl.getParams(), decl.accessLevel);

        if (!decl.isConstructorDecl() && !decl.isDestructorDecl() && decl.getReturnType()) {
            typecheckType(decl.getReturnType(), decl.accessLevel);
        }

        decl.typechecked = true;
        return;
    }

    TypeDecl* receiverTypeDecl = decl.getTypeDecl();

    Scope scope(&decl, &currentModule->symbolTable);
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
            auto* varDecl = makeAST<VarDecl>(thisType, "this", nullptr, &decl, AccessLevel::None, *currentModule, decl.getLocation());
            currentModule->addToSymbolTable(varDecl);
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
                            if (auto* constructorDecl = llvm::dyn_cast_or_null<ConstructorDecl>(callExpr->calleeDecl)) {
                                if (constructorDecl->getTypeDecl() == receiverTypeDecl || receiverTypeDecl->hasInterface(*constructorDecl->getTypeDecl())) {
                                    delegatedInit = true;
                                }
                            }
                        }
                    }
                }
            }

            if (!decl.getReturnType()) {
                ASSERT(decl.isLambda());
                decl.proto.returnType = Type::getVoid();
            }

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

    // Don't warn about unused variables in the standard library or after errors.
    if (errors == errorsBefore && decl.getModule()->name != "std" && !options.noUnusedWarnings) {
        for (auto* varDecl : localVarDecls) {
            if (!varDecl->isReferenced() && !varDecl->getName().starts_with("_")) {
                WARN(varDecl->getLocation(), "unused variable '" << varDecl->getName() << "'");
            }
        }
    }

    decl.typechecked = true;
}

void Typechecker::typecheckFunctionTemplate(FunctionTemplate& decl) {
    typecheckGenericParamDecls(decl.genericParams, decl.accessLevel);

    FunctionDecl* functionDecl = decl.functionDecl;
    if (!functionDecl->hasPack()) return;

    auto params = functionDecl->getParams();
    Type packType = params.back().type;

    for (const GenericParamDecl& genericParam : decl.genericParams) {
        bool inPack = packType && containsGenericParam(packType, genericParam.getName());
        bool inFixed = false;
        for (const ParamDecl& param : params.drop_back()) {
            if (param.type && containsGenericParam(param.type, genericParam.getName())) {
                inFixed = true;
                break;
            }
        }
        if (inPack && inFixed) {
            ERROR(params.back().getLocation(), "generic parameter '" << genericParam.getName() << "' cannot be used in both fixed and variadic parameters");
        }
        if (inPack && functionDecl->getReturnType() && containsGenericParam(functionDecl->getReturnType(), genericParam.getName())) {
            ERROR(functionDecl->getReturnType().location, "variadic generic parameter '" << genericParam.getName() << "' cannot be used in return type");
        }
    }
}

void Typechecker::typecheckTypeDecl(TypeDecl& decl) {
    for (Type interface : decl.interfaces) {
        typecheckType(interface, decl.accessLevel);
        auto* interfaceDecl = interface.getDecl();

        if (!interfaceDecl->isInterface()) {
            REPORT_ERROR(interface.location, "'" << interface << "' is not an interface");
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

    checkForInfiniteSize(decl, map(realDecl->fields, [](const FieldDecl& field) { return field.type; }));
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

    checkForInfiniteSize(decl, map(decl.cases, [](const EnumCase& enumCase) { return enumCase.associatedType; }));
}

// Global initializers are emitted as constant expressions; anything needing runtime
// evaluation would emit instructions outside any function and corrupt codegen.
static bool isSupportedGlobalInitializer(const Expr& expr, llvm::SmallPtrSetImpl<const VarDecl*>& seen) {
    switch (expr.kind) {
    case ExprKind::IntLiteralExpr:
    case ExprKind::FloatLiteralExpr:
    case ExprKind::BoolLiteralExpr:
    case ExprKind::CharacterLiteralExpr:
    case ExprKind::NullLiteralExpr:
    case ExprKind::UndefinedLiteralExpr:
    case ExprKind::SizeofExpr:
        return true;
    case ExprKind::VarExpr: {
        auto* decl = llvm::cast<VarExpr>(expr).decl;
        if (!decl) return true; // Unresolved references are diagnosed elsewhere, don't cascade.
        if (llvm::isa<FunctionDecl>(decl)) return true;
        auto* varDecl = llvm::dyn_cast<VarDecl>(decl);
        // Immutable globals inline their initializer; mutable ones need a runtime load.
        return varDecl && !varDecl->type.isMutable() && varDecl->initializer && seen.insert(varDecl).second
            && isSupportedGlobalInitializer(*varDecl->initializer, seen);
    }
    case ExprKind::MemberExpr:
        return llvm::isa_and_nonnull<EnumCase>(llvm::cast<MemberExpr>(expr).decl);
    case ExprKind::UnaryExpr: {
        auto& unary = llvm::cast<UnaryExpr>(expr);
        switch (unary.op) {
        case Token::Plus:
        case Token::Minus:
        case Token::Tilde:
            return isSupportedGlobalInitializer(unary.getOperand(), seen);
        case Token::Not:
            return unary.getOperand().type.isBool() && isSupportedGlobalInitializer(unary.getOperand(), seen);
        case Token::And: {
            // Addresses of globals and functions are constants, but const globals have no storage.
            auto* varExpr = llvm::dyn_cast<VarExpr>(&unary.getOperand());
            if (!varExpr) return false;
            if (!varExpr->decl) return true;
            if (auto* varDecl = llvm::dyn_cast<VarDecl>(varExpr->decl)) {
                return varDecl->isGlobal() && varDecl->type.isMutable();
            }
            return llvm::isa<FunctionDecl>(varExpr->decl);
        }
        default:
            return false;
        }
    }
    case ExprKind::BinaryExpr: {
        auto& binary = llvm::cast<BinaryExpr>(expr);
        if (!isSupportedGlobalInitializer(binary.getLHS(), seen) || !isSupportedGlobalInitializer(binary.getRHS(), seen)) {
            return false;
        }
        switch (binary.op) {
        case Token::Plus:
        case Token::Minus:
        case Token::Star:
        case Token::Slash:
        case Token::Modulo:
        case Token::PositiveModulo:
        case Token::And:
        case Token::Or:
        case Token::Xor:
        case Token::LeftShift:
        case Token::RightShift:
            return expr.type.isInteger() || expr.type.isFloatingPoint() || expr.type.isChar();
        case Token::Equal:
        case Token::NotEqual:
        case Token::Less:
        case Token::LessOrEqual:
        case Token::Greater:
        case Token::GreaterOrEqual:
            return expr.type.isBool();
        default:
            // Assignments, &&, ||, ranges, and overloaded operators need runtime code.
            return false;
        }
    }
    case ExprKind::ImplicitCastExpr: {
        auto& cast = llvm::cast<ImplicitCastExpr>(expr);
        // Pointer-implemented optionals need no construction, other casts need loads or branches.
        return cast.castKind == ImplicitCastExpr::OptionalWrap && expr.type.isOptionalType() && expr.type.getWrappedType().isImplementedAsPointer()
            && isSupportedGlobalInitializer(*cast.operand, seen);
    }
    default:
        return false;
    }
}

void Typechecker::typecheckVarDecl(VarDecl& decl) {
    if (!decl.isGlobal()) {
        localVarDecls.push_back(&decl);
    }

    Type declaredType = decl.type;
    if (declaredType) {
        typecheckType(declaredType, !decl.isGlobal() ? AccessLevel::None : decl.accessLevel);
    }

    if (decl.initializer) {
        try {
            typecheckExpr(*decl.initializer, false, declaredType);
        } catch (const CompileError&) {
            if (!decl.isGlobal()) currentModule->addToSymbolTable(decl);
            throw;
        }
    }

    if (!decl.isGlobal()) currentModule->addToSymbolTable(decl);
    if (!decl.initializer) {
        if (!declaredType) {
            ERROR(decl.getLocation(), "couldn't infer type of '" << decl.getName() << "', add a type annotation or initializer");
        }
        return;
    }
    Type initializerType = decl.initializer->type;
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

            diagnoseClosureConversion(initializerType, declaredType, decl.initializer->location);
            ERROR(decl.initializer->location, "cannot assign '" << initializerType << "' to '" << declaredType << "'" << hint);
        }
    } else {
        if (initializerType.isNull()) {
            ERROR(decl.getLocation(), "couldn't infer type of '" << decl.getName() << "', add a type annotation");
        }

        decl.type = NOTNULL(initializerType.withMutability(decl.type.mutability));
    }

    if (!decl.type.isImplicitlyCopyable()) {
        setMoved(decl.initializer, true);
    }

    if (decl.isGlobal() && decl.initializer) {
        llvm::SmallPtrSet<const VarDecl*, 8> seen;
        if (!isSupportedGlobalInitializer(*decl.initializer, seen)) {
            ERROR(decl.initializer->location, "global variable initializer must be a constant expression");
        }
    }
}

void Typechecker::typecheckFieldDecl(FieldDecl& decl) {
    typecheckType(decl.type, std::min(decl.accessLevel, decl.getParentDecl()->accessLevel));

    if (decl.defaultValue) {
        typecheckExpr(*decl.defaultValue, false, decl.type);
        if (Expr* converted = convert(decl.defaultValue, decl.type)) {
            decl.defaultValue = converted;
        } else {
            ERROR(decl.defaultValue->location, "cannot assign '" << decl.defaultValue->type << "' to '" << decl.type << "'");
        }
    }
}

void Typechecker::typecheckImportDecl(ImportDecl& decl, const BuildConfig* config) {
    if (decl.target.ends_with(".h")) {
        const int errorsBefore = errors;
        if (!importCHeader(*currentSourceFile, decl, *this) && errors == errorsBefore) {
            REPORT_ERROR(decl.getLocation(), "couldn't import C header file '" << decl.target << "'");
        }
    } else {
        auto module = importModule(currentSourceFile, config, decl.target);
        if (!module) {
            if (module.getError() == std::make_error_code(std::errc::no_such_file_or_directory)) {
                REPORT_ERROR(decl.getLocation(), "couldn't find module '" << decl.target << "' in the following locations:\n"
                                                                          << llvm::join(options.importSearchPaths, "\n"));
            } else {
                REPORT_ERROR(decl.getLocation(), "couldn't import module '" << decl.target << "': " << module.getError().message());
            }
        }
    }
}

void Typechecker::typecheckTopLevelDecl(Decl& decl, const BuildConfig* config) {
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
        typecheckImportDecl(llvm::cast<ImportDecl>(decl), config);
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
