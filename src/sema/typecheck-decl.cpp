#include "typecheck.h"
#include <algorithm>
#include <limits>
#pragma warning(push, 0)
#include <llvm/ADT/ScopeExit.h>
#include <llvm/ADT/SmallPtrSet.h>
#include <llvm/ADT/StringExtras.h>
#include <llvm/Support/SaveAndRestore.h>
#include <llvm/TargetParser/Host.h>
#include <llvm/TargetParser/Triple.h>
#pragma warning(pop)
#include "../ast/arena.h"
#include "../ast/ast.h"
#include "../ast/module.h"
#include "../build/dependencies.h"
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

// Interfaces constrain but never store, so borrows may appear in their generic arguments
// (e.g. an iterator conforming to Iterator<Element&>); Optional is likewise transparent.
static bool allowsBorrowArgs(Decl* decl) {
    if (auto* typeDecl = llvm::dyn_cast<TypeDecl>(decl)) return typeDecl->isInterface();
    if (auto* typeTemplate = llvm::dyn_cast<TypeTemplate>(decl)) return typeTemplate->typeDecl->isInterface();
    return false;
}

// Finds the type template to instantiate for a generic type name. A same-named function
// doesn't prevent using the type in type position.
static TypeTemplate* findTypeTemplateForGenericArgs(Type type, std::vector<Decl*> decls) {
    decls.erase(std::remove_if(decls.begin(), decls.end(), [](Decl* d) { return !d->isTypeTemplate() && !d->isTypeDecl(); }), decls.end());

    if (decls.empty()) {
        ERROR_RANGE(type.location, type.endLocation, "'" << type << "' is not a type");
    }

    if (!decls[0]->isTypeTemplate()) {
        ERROR_RANGE(type.location, type.endLocation, "too many generic arguments to '" << type.getName() << "', expected 0");
    }

    if (decls.size() > 1) {
        ERROR_WITH_NOTES_RANGE(type.location, type.endLocation, getTypeCandidateNotes(decls), "ambiguous reference to '" << type.getName() << "'");
    }

    return llvm::cast<TypeTemplate>(decls[0]);
}

// Returns true if values of the given type transitively contain the target type declaration without pointer indirection,
// meaning the target type would have infinite size. `visiting` holds the declarations on the current search path.
static bool containsItselfByValue(Type type, const TypeDecl& target, llvm::SmallPtrSetImpl<const TypeDecl*>& visiting) {
    // Pointers, array pointers, and functions are pointer-sized (see getIRType), as are builtins.
    if (!type || type.isBuiltinType() || type.isFunctionType() || type.isImplementedAsPointer()) return false;

    if (type.isArrayType()) {
        if (type.hasSizeofArraySize()) {
            // A sizeof size depends on its operand's layout; a cycle through it
            // leaves the size equation unsolvable.
            if (containsItselfByValue(type.getSizeofArrayOperand(), target, visiting)) return true;
        } else if (type.getArraySize() == 0) {
            return false; // Zero-sized arrays occupy no storage.
        }
        return containsItselfByValue(type.getElementType(), target, visiting);
    }

    if (type.isAnonymousStructType()) {
        return llvm::any_of(type.getAnonymousStructElements(),
                            [&](const AnonymousStructElement& element) { return containsItselfByValue(element.type, target, visiting); });
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

// Substituted references are valid in members of an instantiation whose generic arguments
// contain a reference: the use-site spelling was already validated, so the reference arrived
// through substitution rather than being written in a stored position.
static bool allowsSubstitutedReference(const TypeDecl& typeDecl) {
    return typeDecl.instantiatedFrom != nullptr
        && llvm::any_of(typeDecl.genericArgs, [](GenericArg arg) { return arg.isType() && arg.getType().containsReference(); });
}

static void checkForInfiniteSize(const TypeDecl& target, llvm::ArrayRef<Type> memberTypes) {
    llvm::SmallPtrSet<const TypeDecl*, 8> visiting;
    visiting.insert(&target);
    if (llvm::any_of(memberTypes, [&](Type type) { return containsItselfByValue(type, target, visiting); })) {
        ERROR_RANGE(target.getLocation(), getIdentifierEndLocation(target), "'" << target.getName() << "' has infinite size because it contains itself");
    }
}

TypeAliasDecl* Typechecker::findTypeAlias(Type type) {
    if (!type.isBasicType() || type.isBuiltinType()) return nullptr;
    // Anonymous C-imported types have empty names; nothing to resolve.
    if (type.getName().empty()) return nullptr;

    // Current-module type declarations shadow imported declarations. Keep the
    // first declaration at the winning scope; multiple imported type names stay
    // ambiguous and are diagnosed by the normal type lookup path.
    Decl* firstType = nullptr;
    for (Decl* decl : findDecls(type.getName())) {
        if (!decl->isTypeAliasDecl() && !decl->isTypeDecl() && !decl->isTypeTemplate()) continue;
        if (!firstType) {
            firstType = decl;
        } else if (firstType->getModule() != currentModule) {
            return nullptr;
        } else if (decl->getModule() != currentModule) {
            break;
        }
    }
    return firstType && firstType->isTypeAliasDecl() ? llvm::cast<TypeAliasDecl>(firstType) : nullptr;
}

// tryFindDecl restricted to globals for signatures: the size's home-module
// top level, then its imports, then stdlib, with tryFindDecl's precedence.
// Throws on ambiguity like the module lookup it mirrors; the caller swallows
// that. A null home module (bare size names) falls back to checker scope.
static Decl* findSizeDeclInSignature(Typechecker& checker, llvm::StringRef name, Location location, Module* homeModule) {
    Module* scope = homeModule ? homeModule : checker.currentModule;
    auto top = scope->symbolTable.findInTopLevelScope(name);
    if (top.size() == 1) return top[0];
    if (top.size() > 1) ERROR_RANGE(location, getIdentifierEndLocation(location, name), "ambiguous reference to '" << name << "'");
    if (homeModule) {
        // The size's own file isn't recoverable from the module, so union
        // every file's imports; same-module names need no import anyway.
        std::vector<Module*> imports;
        for (auto& file : homeModule->sourceFiles) {
            for (Module* imported : file.importedModules) {
                if (std::find(imports.begin(), imports.end(), imported) == imports.end()) imports.push_back(imported);
            }
        }
        if (Decl* match = findDeclInModules(name, location, imports)) return match;
    } else if (checker.currentSourceFile) {
        if (Decl* match = findDeclInModules(name, location, checker.currentSourceFile->importedModules)) return match;
    }
    return findDeclInModules(name, location, Module::getStdlibModule());
}

// Binds the names in a deferred array size to the constants they denote in
// the current scope. Names that don't resolve to a single immutable variable
// stay unbound, so folding simply fails for them; nothing here throws.
static void bindArraySizeNames(Typechecker& checker, Expr& expr, Module* homeModule) {
    if (auto* varExpr = llvm::dyn_cast<VarExpr>(&expr)) {
        if (varExpr->decl) return;
        // Same lookup as value uses, so shadowing matches exactly; ambiguous
        // or unknown names simply stay unbound. Signatures resolve in the
        // size's home module (folding can run under another module's
        // instantiation), so they see globals only: no caller locals,
        // parameters, or receiver members.
        Decl* decl;
        try {
            llvm::SaveAndRestore suppress(checker.suppressEnsureSignature, true);
            if (checker.checkingFunctionSignature) {
                decl = findSizeDeclInSignature(checker, varExpr->identifier, varExpr->location, homeModule);
            } else {
                decl = checker.tryFindDecl(varExpr->identifier, varExpr->location);
            }
        } catch (const CompileError&) {
            return;
        }
        if (!decl) return;
        auto* varDecl = llvm::dyn_cast<VarDecl>(decl);
        // Mutable variables never fold; an unset type means an unchecked
        // inferred constant, whose value isn't known yet either.
        if (!varDecl || !varDecl->type || !varDecl->isConst) return;
        checker.markReferenced(varDecl);
        varExpr->decl = varDecl;
        return;
    }
    if (auto* memberExpr = llvm::dyn_cast<MemberExpr>(&expr)) {
        bindArraySizeNames(checker, *memberExpr->base, homeModule);
        return;
    }
    if (auto* arrayLiteral = llvm::dyn_cast<ArrayLiteralExpr>(&expr)) {
        for (Expr* element : arrayLiteral->elements)
            bindArraySizeNames(checker, *element, homeModule);
        return;
    }
    if (auto* unaryExpr = llvm::dyn_cast<UnaryExpr>(&expr)) {
        bindArraySizeNames(checker, unaryExpr->getOperand(), homeModule);
        return;
    }
    if (auto* binaryExpr = llvm::dyn_cast<BinaryExpr>(&expr)) {
        bindArraySizeNames(checker, binaryExpr->getLHS(), homeModule);
        bindArraySizeNames(checker, binaryExpr->getRHS(), homeModule);
        return;
    }
    if (auto* ifExpr = llvm::dyn_cast<IfExpr>(&expr)) {
        bindArraySizeNames(checker, *ifExpr->condition, homeModule);
        bindArraySizeNames(checker, *ifExpr->thenExpr, homeModule);
        bindArraySizeNames(checker, *ifExpr->elseExpr, homeModule);
        return;
    }
}

Type Typechecker::resolveArraySize(Expr& sizeExpr, Type elementType, Location location, Location endLocation, Module* homeModule) {
    // The payload is shared (aliases, repeated resolves, substituted clones),
    // so bind a clone; attempts stay independent and idempotent.
    Expr* attempt = sizeExpr.instantiate({});
    bindArraySizeNames(*this, *attempt, homeModule);
    if (!attempt->isFoldableIntConstant()) {
        ERROR_RANGE(getExprRangeStart(sizeExpr), sizeExpr.endLocation, "array size must be a constant integer expression");
    }
    checkArraySizeDivisors(*attempt);
    llvm::APSInt size = attempt->getConstantIntegerValue();
    if (size.isNegative()) {
        ERROR_RANGE(getExprRangeStart(sizeExpr), sizeExpr.endLocation, "array size must be non-negative");
    }
    if (size.getActiveBits() > 63) {
        ERROR_RANGE(getExprRangeStart(sizeExpr), sizeExpr.endLocation, "array size is too large");
    }
    return BasicType::getArray(elementType, size.getSExtValue(), location, endLocation);
}

Type Typechecker::resolveTypeAliases(Type type, AccessLevel userAccessLevel, bool foldArraySizes) {
    llvm::SmallPtrSet<const TypeAliasDecl*, 8> resolving;
    return resolveTypeAliases(std::move(type), userAccessLevel, resolving, foldArraySizes);
}

Type Typechecker::resolveTypeAliases(Type type, AccessLevel userAccessLevel, llvm::SmallPtrSetImpl<const TypeAliasDecl*>& resolving, bool foldArraySizes) {
    if (!type) return type;

    switch (type.getKind()) {
    case TypeKind::BasicType: {
        auto* basicType = llvm::cast<BasicType>(type.typeBase);
        if (auto* alias = findTypeAlias(type)) {
            if (!basicType->genericArgs.empty()) return type;
            if (!resolving.insert(alias).second) {
                if (!alias->cycleReported) {
                    for (auto* resolvingAlias : resolving)
                        const_cast<TypeAliasDecl*>(resolvingAlias)->cycleReported = true;
                    REPORT_ERROR_RANGE(type.location, type.endLocation, "cyclic type alias '" << alias->getName() << "'");
                }
                return type;
            }

            checkHasAccess(*alias, type.location, userAccessLevel);
            markReferenced(alias);
            Type resolved = resolveTypeAliases(alias->aliasedType, userAccessLevel, resolving, foldArraySizes);
            resolving.erase(alias);

            // Outermost alias wins: inner resolution already attached its own
            // spelling, so this overwrites it with the name used at this site.
            resolved = resolved.withLocation(type.location, type.endLocation);
            resolved.aliasSpelling = alias->getName();
            return resolved;
        }

        if (basicType->genericArgs.empty()) return type;

        auto genericArgs = map(basicType->genericArgs, [&](GenericArg arg) {
            if (!arg.isType()) return arg;
            arg.type = resolveTypeAliases(arg.type, userAccessLevel, resolving, foldArraySizes);
            return arg;
        });
        Type rebuilt = BasicType::get(basicType->name, genericArgs, type.location, type.endLocation);
        // A non-type element cannot fold; typecheckType diagnoses it below.
        if (rebuilt.isFixedArray() && !rebuilt.getGenericArgs()[0].isType()) return rebuilt;
        // Fold sizeof sizes whose operand now has a known size.
        if (rebuilt.isFixedArray() && rebuilt.hasSizeofArraySize()) {
            if (auto size = rebuilt.getSizeofArrayOperand().getSizeInBytes()) {
                return BasicType::getArray(rebuilt.getElementType(), int64_t(*size), type.location, type.endLocation);
            }
        }
        // Fold deferred sizes and bare size names at real declaration sites.
        // Probes keep them symbolic; typecheckType diagnoses leftovers.
        if (foldArraySizes && rebuilt.isFixedArray() && !rebuilt.hasSizeofArraySize()) {
            if (rebuilt.hasDeferredArraySize()) {
                try {
                    return resolveArraySize(*rebuilt.getDeferredArraySize(), rebuilt.getElementType(), type.location, type.endLocation,
                                            rebuilt.getDeferredArraySizeHome());
                } catch (const CompileError&) {
                }
            } else if (!rebuilt.getArraySizeParam().empty()) {
                auto* name = makeAST<VarExpr>(rebuilt.getArraySizeParam(), rebuilt.getGenericArgs()[1].location);
                name->endLocation = getIdentifierEndLocation(name->location, name->identifier);
                try {
                    return resolveArraySize(*name, rebuilt.getElementType(), type.location, type.endLocation, nullptr);
                } catch (const CompileError&) {
                }
            }
        }
        if (rebuilt == type) return type;
        return rebuilt;
    }
    case TypeKind::ArrayPointerType: {
        auto elementType = resolveTypeAliases(type.getElementType(), userAccessLevel, resolving, foldArraySizes);
        if (elementType == type.getElementType()) return type;
        return ArrayPointerType::get(elementType, type.location, type.endLocation);
    }
    case TypeKind::AnonymousStructType: {
        auto elements = mapAst(type.getAnonymousStructElements(), [&](const AnonymousStructElement& element) {
            return AnonymousStructElement{element.name, resolveTypeAliases(element.type, userAccessLevel, resolving, foldArraySizes)};
        });
        if (llvm::equal(elements, type.getAnonymousStructElements())) return type;
        return AnonymousStructType::get(std::move(elements), type.location, type.endLocation);
    }
    case TypeKind::FunctionType: {
        auto returnType = resolveTypeAliases(type.getReturnType(), userAccessLevel, resolving, foldArraySizes);
        auto paramTypes =
            mapAst(type.getParamTypes(), [&](Type paramType) { return resolveTypeAliases(paramType, userAccessLevel, resolving, foldArraySizes); });
        if (returnType == type.getReturnType() && llvm::equal(paramTypes, type.getParamTypes())) return type;
        return FunctionType::get(returnType, std::move(paramTypes), llvm::cast<FunctionType>(type.typeBase)->isVariadic, type.location, type.endLocation);
    }
    case TypeKind::PointerType: {
        auto pointeeType = resolveTypeAliases(type.getPointee(), userAccessLevel, resolving, foldArraySizes);
        if (pointeeType == type.getPointee()) return type;
        return PointerType::get(pointeeType, type.getPointerKind(), type.location, type.endLocation);
    }
    case TypeKind::UnresolvedType:
        return type;
    }
    llvm_unreachable("all cases handled");
}

void Typechecker::canonicalizeTypeAliases() {
    llvm::SaveAndRestore suppress(suppressEnsureSignature, true);
    auto hasTypeAlias = [](const Module& module) {
        return llvm::any_of(module.sourceFiles, [](const SourceFile& sourceFile) {
            return llvm::any_of(sourceFile.topLevelDecls, [](const Decl* decl) { return decl->isTypeAliasDecl(); });
        });
    };
    if (!hasTypeAlias(*currentModule) && llvm::none_of(Module::getAllImportedModules(), [&](const Module* module) { return hasTypeAlias(*module); })) {
        return;
    }

    auto resolveType = [&](Type& type, AccessLevel accessLevel) { type = resolveTypeAliases(std::move(type), accessLevel); };
    auto resolveParams = [&](AstVector<ParamDecl>& params, AccessLevel accessLevel) {
        for (auto& param : params) {
            resolveType(param.type, accessLevel);
        }
    };
    auto resolveFunction = [&](FunctionDecl& function, AccessLevel accessLevel) {
        resolveParams(function.proto.params, accessLevel);
        resolveType(function.proto.returnType, accessLevel);
    };
    auto resolveFunctionTemplate = [&](FunctionTemplate& function, AccessLevel accessLevel) {
        for (auto& genericParam : function.genericParams) {
            for (auto& constraint : genericParam.constraints) {
                resolveType(constraint, accessLevel);
            }
            resolveType(genericParam.valueType, accessLevel);
        }
        resolveFunction(*function.functionDecl, accessLevel);
    };
    auto resolveTypeDecl = [&](TypeDecl& type, AccessLevel accessLevel) {
        for (auto& interface : type.interfaces) {
            resolveType(interface, accessLevel);
        }
        for (auto& field : type.fields) {
            resolveType(field.type, std::min(field.accessLevel, accessLevel));
        }
        for (auto* staticConst : type.staticConsts) {
            resolveType(staticConst->type, std::min(staticConst->accessLevel, accessLevel));
        }
        if (auto* enumDecl = llvm::dyn_cast<EnumDecl>(&type)) {
            for (auto& enumCase : enumDecl->cases) {
                resolveType(enumCase.associatedType, std::min(enumCase.accessLevel, accessLevel));
            }
        }
        for (auto* method : type.methods) {
            if (auto* function = llvm::dyn_cast<FunctionDecl>(method)) {
                resolveFunction(*function, std::min(method->accessLevel, accessLevel));
            } else {
                resolveFunctionTemplate(*llvm::cast<FunctionTemplate>(method), std::min(method->accessLevel, accessLevel));
            }
        }
    };

    for (auto& sourceFile : currentModule->sourceFiles) {
        currentSourceFile = &sourceFile;
        for (Decl* decl : sourceFile.topLevelDecls) {
            switch (decl->kind) {
            case DeclKind::TypeAliasDecl:
                break;
            case DeclKind::VarDecl:
                resolveType(llvm::cast<VarDecl>(decl)->type, decl->accessLevel);
                break;
            case DeclKind::FunctionDecl:
                resolveFunction(*llvm::cast<FunctionDecl>(decl), decl->accessLevel);
                break;
            case DeclKind::FunctionTemplate:
                resolveFunctionTemplate(*llvm::cast<FunctionTemplate>(decl), decl->accessLevel);
                break;
            case DeclKind::TypeDecl:
                resolveTypeDecl(*llvm::cast<TypeDecl>(decl), decl->accessLevel);
                break;
            case DeclKind::TypeTemplate: {
                auto& typeTemplate = *llvm::cast<TypeTemplate>(decl);
                for (auto& genericParam : typeTemplate.genericParams) {
                    for (auto& constraint : genericParam.constraints) {
                        resolveType(constraint, typeTemplate.accessLevel);
                    }
                    resolveType(genericParam.valueType, typeTemplate.accessLevel);
                }
                resolveTypeDecl(*typeTemplate.typeDecl, typeTemplate.accessLevel);
                break;
            }
            case DeclKind::EnumDecl:
                resolveTypeDecl(*llvm::cast<EnumDecl>(decl), decl->accessLevel);
                break;
            case DeclKind::ImportDecl:
                break;
            case DeclKind::ParamDecl:
            case DeclKind::GenericParamDecl:
            case DeclKind::MethodDecl:
            case DeclKind::ConstructorDecl:
            case DeclKind::DestructorDecl:
            case DeclKind::EnumCase:
            case DeclKind::FieldDecl:
                llvm_unreachable("invalid top-level declaration kind");
            }
        }
    }
}

void Typechecker::typecheckType(Type type, AccessLevel userAccessLevel, bool recheckGenericArgs, bool allowReference) {
    type = resolveTypeAliases(std::move(type), userAccessLevel);
    if (auto* alias = findTypeAlias(type)) {
        if (type.isBasicType() && !type.getGenericArgs().empty()) {
            ERROR_RANGE(type.location, type.endLocation, "type alias '" << alias->getName() << "' does not take generic arguments");
        }
        if (!alias->cycleReported) ERROR_RANGE(type.location, type.endLocation, "cyclic type alias '" << alias->getName() << "'");
        throw CompileError::dependentError();
    }

    if (!allowReference && type.storesBorrow()) {
        // Report the outermost type (e.g. 'int&?' rather than the nested 'int&')
        // so the diagnostic matches what the user wrote.
        ERROR_RANGE(type.location, type.endLocation,
                    "reference type '" << type << "' may only appear as a function parameter, return type, local variable, or interface argument");
    }
    switch (type.getKind()) {
    case TypeKind::BasicType: {
        // Fixed arrays are a builtin-backed BasicType. Keep declaration
        // binding lazy: getTypeDecl resolves the stdlib methods only when a
        // member is actually looked up.
        if (type.isFixedArray()) {
            if (type.hasSizeofArraySize()) {
                Type operand = type.getSizeofArrayOperand();
                typecheckType(operand.withLocation(type.location, type.endLocation), userAccessLevel, recheckGenericArgs);
                if (resolveTypeAliases(operand).isVoid()) {
                    ERROR_RANGE(type.location, type.endLocation, "cannot take sizeof of 'void'");
                }
                // Element destructors are emitted per element with a static count,
                // which a backend-folded size cannot provide.
                if (type.getElementType().needsDestruction()) {
                    ERROR_RANGE(type.location, type.endLocation, "arrays with sizeof-computed size cannot hold owning elements");
                }
            } else if (!type.getArraySizeParam().empty()) {
                ERROR_RANGE(type.location, type.endLocation, "array size must be a constant integer expression");
            } else if (type.hasDeferredArraySize()) {
                // Resolution already tried folding this; diagnose the reason it cannot fold.
                resolveArraySize(*type.getDeferredArraySize(), type.getElementType(), type.location, type.endLocation, type.getDeferredArraySizeHome());
            }
            if (type.getGenericArgs()[1].isInt() && type.getArraySize() > std::numeric_limits<int>::max()) {
                ERROR_RANGE(type.location, type.endLocation, "array size is too large");
            }
            if (!type.getGenericArgs()[0].isType()) {
                ERROR_RANGE(type.location, type.endLocation, "array element type must be a type, not an integer");
            }
            typecheckType(type.getElementType(), userAccessLevel, recheckGenericArgs);
            break;
        }
        Decl* decl;
        auto* basicType = llvm::cast<BasicType>(type.typeBase);
        if (TypeDecl* typeDecl = type.getDecl()) {
            decl = typeDecl;

            // Check generic arguments on repeat uses too: resolving the declaration above
            // skips the lookup below, which would otherwise silence access warnings after
            // the first use. Type nodes are interned, so the stored arguments carry the
            // first use's locations; relocate them to the current use. This is exact when
            // the nested types start where the outer type starts (e.g. 'A' in 'A*?').
            if (recheckGenericArgs) {
                // Optional is transparent to the placement rule: 'T&?' is still just a borrow.
                bool nestedAllowReference = allowReference && (type.isOptionalType() || allowsBorrowArgs(decl));
                for (auto genericArg : basicType->genericArgs) {
                    if (genericArg.isType())
                        typecheckType(genericArg.getType().withLocation(type.location, type.endLocation), userAccessLevel, true, nestedAllowReference);
                }
            }
        } else {
            if (basicType->name.empty()) break; // Nothing to type-check.

            if (!type.isOptionalType() && type.isBuiltinType()) {
                validateGenericArgs({}, type.getGenericArgs(), type.getName(), type.location);
                break;
            }

            // Optional is transparent to the placement rule: 'T&?' is still just a borrow.
            // Interfaces likewise constrain but never store, so look them up to decide.
            // The lookup only runs when some argument actually holds a borrow.
            bool nestedAllowReference = allowReference && type.isOptionalType();
            if (allowReference && !nestedAllowReference && !basicType->name.empty()
                && llvm::any_of(basicType->genericArgs, [](GenericArg arg) { return arg.isType() && arg.type.storesBorrow(); })) {
                nestedAllowReference = llvm::any_of(findDecls(basicType->name), allowsBorrowArgs);
            }
            for (auto genericArg : basicType->genericArgs) {
                // Type nodes are canonicalized, so the stored arguments may carry another use's
                // locations (the first use wins at parse time, but laziness may check a later use
                // first); relocate them to the current use, mirroring the recheck above.
                if (genericArg.isType())
                    typecheckType(genericArg.getType().withLocation(type.location, type.endLocation), userAccessLevel, true, nestedAllowReference);
            }

            auto decls = findDecls(basicType->getQualifiedName());

            if (decls.empty()) {
                // For generic types, search again with the base name (without generic args).
                auto decls = findDecls(basicType->name);

                if (decls.empty()) {
                    ERROR_RANGE(type.location, type.endLocation, "unknown type '" << type << "'" << Type::didYouMeanBuiltin(basicType->name));
                }
                auto* typeTemplate = findTypeTemplateForGenericArgs(type, std::move(decls));
                decl = typeTemplate;
                ASSERT(!basicType->genericArgs.empty());
                if (!validateGenericArgs(typeTemplate->genericParams, basicType->genericArgs, basicType->name, type.location)) {
                    throw CompileError::dependentError();
                }
                auto instantiation = typeTemplate->instantiate(basicType->genericArgs);
                currentModule->addToSymbolTable(*instantiation);
                deferTypechecking(instantiation);
                ensureNestedInstantiations(*instantiation);
                checkHasAccess(*decl, type.location, userAccessLevel);
                // This first-mention path breaks out before the destructor
                // marking below, so mark here too.
                if (!checkingFunctionSignature) {
                    if (auto* typeDecl = llvm::dyn_cast<TypeDecl>(instantiation)) {
                        if (DestructorDecl* dtor = typeDecl->getDestructor()) markReferenced(dtor);
                    }
                }
                break;
            } else if (decls.size() > 1) {
                ERROR_WITH_NOTES_RANGE(type.location, type.endLocation, getTypeCandidateNotes(decls), "ambiguous reference to '" << type.getName() << "'");
            } else {
                decl = decls.front();
            }
        }

        if (decl->isTypeTemplate()) {
            validateGenericArgs(llvm::cast<TypeTemplate>(decl)->genericParams, basicType->genericArgs, basicType->name, type.location);
        } else if (decl->isGenericParamDecl()) {
            // A generic parameter in scope (e.g. a sibling mentioned in a function-type constraint).
            break;
        } else if (!decl->isTypeDecl()) {
            ERROR_RANGE(type.location, type.endLocation, "'" << type << "' is not a type");
        }

        // IRGen drops values of destructor types at scope exit without going through
        // name resolution, so the destructor would never be demand-checked otherwise.
        // Skipped for types mentioned in signatures: parameters are marked from the
        // body instead (see typecheckFunctionDecl), and no other values materialize
        // from a signature mention.
        if (!checkingFunctionSignature) {
            markDestructorFor(type);
        }

        checkHasAccess(*decl, type.location, userAccessLevel);
        break;
    }
    case TypeKind::ArrayPointerType:
        typecheckType(type.getElementType(), userAccessLevel, recheckGenericArgs);
        break;
    case TypeKind::AnonymousStructType:
        // Anonymous structs are transparent to the placement rule like Optional: elements of
        // a struct in borrowed position are borrowed too.
        for (auto& element : type.getAnonymousStructElements()) {
            typecheckType(element.type, userAccessLevel, recheckGenericArgs, allowReference);
        }
        break;
    case TypeKind::FunctionType:
        // A borrow in a parameter slot never outlives the call, even when the function type itself is stored.
        for (auto paramType : type.getParamTypes()) {
            typecheckType(paramType, userAccessLevel, recheckGenericArgs, true);
        }
        typecheckType(type.getReturnType(), userAccessLevel, recheckGenericArgs);
        break;
    case TypeKind::PointerType: {
        if (type.isReferenceType() && !allowReference) {
            ERROR_RANGE(type.location, type.endLocation,
                        "reference type '" << type << "' may only appear as a function parameter, return type, local variable, or interface argument");
        }
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

    typecheckType(decl.type, userAccessLevel, true, true);
    if (resolveTypeAliases(decl.type).isVoid()) {
        ERROR_RANGE(decl.getLocation(), getIdentifierEndLocation(decl), "cannot declare parameter '" << decl.getName() << "' of type 'void'");
    }
    if (!decl.getName().empty()) {
        currentModule->symbolTable.add(decl.getName(), &decl);
    }
}

// Returns true if the block can exit an enclosing loop via 'break', i.e. contains a
// 'break' that doesn't bind to a nested loop or switch. Lambda bodies can't break
// across function boundaries, so expressions are never descended into.
static bool blockCanBreak(llvm::ArrayRef<Stmt*> block) {
    for (auto* stmt : block) {
        switch (stmt->kind) {
        case StmtKind::BreakStmt:
            return true;
        case StmtKind::IfStmt: {
            auto& ifStmt = llvm::cast<IfStmt>(*stmt);
            if (blockCanBreak(ifStmt.thenBody) || blockCanBreak(ifStmt.elseBody)) return true;
            break;
        }
        case StmtKind::CompoundStmt:
            if (blockCanBreak(llvm::cast<CompoundStmt>(*stmt).body)) return true;
            break;
        case StmtKind::SwitchStmt:
        case StmtKind::WhileStmt:
        case StmtKind::DoWhileStmt:
        case StmtKind::ForStmt:
        case StmtKind::ForEachStmt:
            break; // Breaks inside bind to the nested statement, not to the enclosing loop.
        case StmtKind::ReturnStmt:
        case StmtKind::VarStmt:
        case StmtKind::ExprStmt:
        case StmtKind::DeferStmt:
        case StmtKind::ContinueStmt:
            break; // These contain no statements.
        }
    }
    return false;
}

// A missing condition, or a constant true one, never terminates, so the loop
// returns on every path unless its body can break out.
static bool nonTerminatingLoopReturns(const Expr* condition, llvm::ArrayRef<Stmt*> body) {
    if (condition && (!condition->isConstant() || !condition->getConstantBoolValue())) return false;
    return !blockCanBreak(body);
}

static bool allPathsReturn(llvm::ArrayRef<Stmt*> block, bool assertsOn) {
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
        if (assertsOn && isFalseAssert(*call)) return true;
        return false;
    }
    case StmtKind::IfStmt: {
        auto& ifStmt = llvm::cast<IfStmt>(*block.back());
        return allPathsReturn(ifStmt.thenBody, assertsOn) && allPathsReturn(ifStmt.elseBody, assertsOn);
    }
    case StmtKind::SwitchStmt: {
        auto& switchStmt = llvm::cast<SwitchStmt>(*block.back());
        if (!llvm::all_of(switchStmt.cases, [&](SwitchCase& c) { return allPathsReturn(c.stmts, assertsOn); })) return false;
        if (switchStmt.defaultStmts.empty()) return switchStmt.coversAllEnumCases;
        return allPathsReturn(switchStmt.defaultStmts, assertsOn);
    }
    case StmtKind::ForStmt: {
        // 'while' loops are lowered into 'for' loops before this runs. A missing condition ('for(;;)') never terminates.
        auto& forStmt = llvm::cast<ForStmt>(*block.back());
        return nonTerminatingLoopReturns(forStmt.condition, forStmt.body);
    }
    case StmtKind::DoWhileStmt: {
        auto& doWhileStmt = llvm::cast<DoWhileStmt>(*block.back());
        return nonTerminatingLoopReturns(doWhileStmt.condition, doWhileStmt.body);
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
    }

    // Function-type constraints may mention sibling parameters (e.g. `Pred: Output(int&)`),
    // so make them resolvable while checking constraints.
    Scope scope(nullptr, &currentModule->symbolTable);
    for (auto& genericParam : genericParams) {
        if (!genericParam.isValueParam) {
            currentModule->symbolTable.add(genericParam.getName(), const_cast<GenericParamDecl*>(&genericParam));
        }
    }

    for (auto& genericParam : genericParams) {
        if (genericParam.isValueParam) {
            try {
                const int errorsBefore = errors;
                typecheckType(genericParam.valueType, userAccessLevel);

                if (errors == errorsBefore && !genericParam.valueType.isInteger()) {
                    ERROR_RANGE(genericParam.valueType.location, genericParam.valueType.endLocation, "generic parameters must be types or integers");
                }
            } catch (const CompileError& error) {
                error.report();
            }
            continue;
        }

        for (Type constraint : genericParam.constraints) {
            try {
                if (!constraint.isFunctionType()) {
                    for (auto& sibling : genericParams) {
                        if (!sibling.isValueParam && containsGenericParam(constraint, sibling.getName())) {
                            ERROR_RANGE(constraint.location, constraint.endLocation,
                                        "interface constraint cannot mention generic parameter '" << sibling.getName() << "'");
                        }
                    }
                }
                const int errorsBefore = errors;
                typecheckType(constraint, userAccessLevel);

                TypeDecl* constraintDecl = constraint.isFunctionType() ? nullptr : constraint.getDecl();
                if (errors == errorsBefore && !constraint.isFunctionType() && (!constraintDecl || !constraintDecl->isInterface())) {
                    ERROR_RANGE(constraint.location, constraint.endLocation, "only interface and function types can be used as generic constraints");
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

// 'main' lowers directly to the C entry point, so only signatures the compiler
// can materialize from argc/argv are accepted.
static void checkMainSignature(const FunctionDecl& decl) {
    if (!decl.getReturnType().isVoid() && !decl.getReturnType().isInt32()) {
        ERROR_RANGE(decl.getLocation(), getIdentifierEndLocation(decl), "'main' must return 'void' or 'int'");
    }

    auto params = decl.getParams();
    bool validParams = params.empty();
    if (params.size() == 1 && params[0].type.isSlice()) {
        Type elementType = params[0].type.getElementType();
        validParams = elementType.isString();
    }
    if (!validParams) {
        ERROR_RANGE(decl.getLocation(), getIdentifierEndLocation(decl), "'main' must take no parameters or '(string[] args)'");
    }
}

// True when a by-value `extern` type holds a float satisfying the leaf test anywhere.
template<typename LeafTest> static bool containsFloatWhere(Type type, LeafTest leafTest) {
    if (leafTest(type)) return true;
    // Fixed arrays carry the fieldless Array decl, so check them before getDecl.
    if (type.isFixedArray()) return containsFloatWhere(type.getElementType(), leafTest);
    // Enum payloads are parsed as one-element anonymous structs.
    if (type.isAnonymousStructType()) {
        for (const AnonymousStructElement& element : type.getAnonymousStructElements()) {
            if (containsFloatWhere(element.type, leafTest)) return true;
        }
        return false;
    }
    if (TypeDecl* typeDecl = type.getDecl()) {
        if (typeDecl->isStruct() || typeDecl->tag == TypeTag::Union) {
            for (const FieldDecl& field : typeDecl->fields) {
                if (containsFloatWhere(field.type, leafTest)) return true;
            }
        } else if (typeDecl->isEnumDecl()) {
            for (const EnumCase& enumCase : llvm::cast<EnumDecl>(typeDecl)->cases) {
                if (enumCase.associatedType && containsFloatWhere(enumCase.associatedType, leafTest)) return true;
            }
        }
        return false;
    }
    return false;
}

static bool containsFloat(Type type) {
    return containsFloatWhere(type, [](Type t) { return t.isFloatingPoint(); });
}

// C layout of a type crossing an `extern` boundary by value: size and alignment in
// bytes, assuming natural alignment like LLVM's struct lowering. The frontend only targets
// its host, so host sizes are target sizes (see Type::getIntegerBitWidth).
struct CValueLayout {
    uint64_t size;
    uint64_t align;
};

// Computes the C layout of a by-value `extern` type, or nullopt when a member has no C
// counterpart (slices, strings, containers, enums with payloads, ...). Packed structs compute
// their unpacked layout, which only ever errs towards rejection.
static std::optional<CValueLayout> cValueLayout(Type type) {
    if (type.isInteger()) {
        uint64_t size = (uint64_t)type.getIntegerBitWidth() / 8;
        return CValueLayout{size, size};
    }
    if (type.isBool() || type.isChar()) return CValueLayout{1, 1};
    if (type.isFloat32() || type.isCFloat()) return CValueLayout{4, 4};
    if (type.isFloat64() || type.isCDouble()) return CValueLayout{8, 8};
    if (type.isFloat80()) return CValueLayout{16, 16};
    if (type.isPointerType() || type.isReferenceType() || type.isFunctionType() || type.isArrayPointer()
        || (type.isOptionalType() && type.isImplementedAsPointer())) {
        return CValueLayout{sizeof(void*), sizeof(void*)};
    }
    if (type.isFixedArray()) {
        auto element = cValueLayout(type.getElementType());
        int64_t count = type.getArraySize();
        if (!element || count <= 0) return std::nullopt;
        return CValueLayout{element->size * (uint64_t)count, element->align};
    }
    // Generic instantiations (slices, containers, optionals, ...) may have measurable cx layouts
    // but no C counterpart, so they cannot cross by value.
    if (!type.getGenericArgs().empty()) return std::nullopt;
    if (TypeDecl* typeDecl = type.getDecl()) {
        if (typeDecl->isEnumDecl()) {
            auto& enumDecl = llvm::cast<EnumDecl>(*typeDecl);
            if (enumDecl.hasAssociatedValues()) return std::nullopt;
            return cValueLayout(enumDecl.getTagType());
        }
        auto padTo = [](uint64_t offset, uint64_t align) { return (offset + align - 1) / align * align; };
        if (typeDecl->isStruct()) {
            CValueLayout layout{0, 1};
            for (const FieldDecl& field : typeDecl->fields) {
                auto member = cValueLayout(field.type);
                if (!member) return std::nullopt;
                layout.size = padTo(layout.size, member->align) + member->size;
                layout.align = std::max(layout.align, member->align);
            }
            layout.size = padTo(layout.size, layout.align);
            return layout;
        }
        if (typeDecl->tag == TypeTag::Union) {
            CValueLayout layout{0, 1};
            for (const FieldDecl& field : typeDecl->fields) {
                auto member = cValueLayout(field.type);
                if (!member) return std::nullopt;
                layout.size = std::max(layout.size, member->size);
                layout.align = std::max(layout.align, member->align);
            }
            layout.size = padTo(layout.size, layout.align);
            return layout;
        }
    }
    return std::nullopt;
}

void cx::validateCppVariadicExtra(Type type, const Expr& arg, llvm::StringRef callee) {
    // Fixed arrays decay to pointers in variadic calls, and scalars, pointers, and references
    // cross opaquely; only by-value aggregates need the signature rules.
    if (type.isFixedArray()) return;
    if (type.isSlice()) {
        ERROR_RANGE(getExprRangeStart(arg), arg.endLocation,
                    "type '" << type << "' cannot be passed as a variadic argument to extern \"C++\" function '" << callee
                             << "'; pass a pointer and length instead");
    }
    TypeDecl* typeDecl = type.getDecl();
    if (!typeDecl || (!typeDecl->isStruct() && typeDecl->tag != TypeTag::Union)) return;
    if (type.needsDestruction()) {
        ERROR_RANGE(getExprRangeStart(arg), arg.endLocation,
                    "type '" << type << "' cannot be passed as a variadic argument to extern \"C++\" function '" << callee
                             << "' because it needs destruction; pass it behind a pointer instead");
    }
    if (containsFloat(type)) {
        ERROR_RANGE(getExprRangeStart(arg), arg.endLocation,
                    "type '" << type << "' cannot be passed as a variadic argument to extern \"C++\" function '" << callee
                             << "' because it contains floating-point members; pass it behind a pointer instead");
    }
    auto layout = cValueLayout(type);
    if (!layout) {
        ERROR_RANGE(getExprRangeStart(arg), arg.endLocation,
                    "type '" << type << "' cannot be passed as a variadic argument to extern \"C++\" function '" << callee
                             << "' because it has a member with no C++ counterpart; pass it behind a pointer instead");
    }
    if (layout->size == 0) {
        ERROR_RANGE(getExprRangeStart(arg), arg.endLocation,
                    "type '" << type << "' cannot be passed as a variadic argument to extern \"C++\" function '" << callee
                             << "' because it is empty; pass it behind a pointer instead");
    }
    if (layout->size > 16) {
        ERROR_RANGE(getExprRangeStart(arg), arg.endLocation,
                    "type '" << type << "' cannot be passed as a variadic argument to extern \"C++\" function '" << callee << "' because it is " << layout->size
                             << " bytes; only structs up to 16 bytes can cross by value, pass it behind a pointer instead");
    }
    if (layout->align > 8) {
        ERROR_RANGE(getExprRangeStart(arg), arg.endLocation,
                    "type '" << type << "' cannot be passed as a variadic argument to extern \"C++\" function '" << callee << "' because it requires "
                             << layout->align << "-byte alignment; pass it behind a pointer instead");
    }
}

// Maps an `extern "C++"` parameter, return, or nested type to its Itanium ABI
// encoding. Only types with a C++ counterpart are accepted: C-compatible
// scalars, pointers, references, plain structs, and function pointers. Anything
// else (slices, strings, enums, closures, fixed-width 64-bit integers, ...)
// has no C++ type to mangle as, so it is a compile error rather than a miscompile.
// Structs crossing by value must not need destruction: the boundary copies bytes,
// so each side would destroy its own copy.
static void mangleCppType(llvm::raw_string_ostream& out, Type type, const llvm::Triple& triple, bool byValue, bool isReturn, bool pointeeConst = false) {
    if (type.isPointerType()) {
        Type pointee = type.getPointee();
        out << (type.getPointerKind() == PointerKind::Reference ? 'R' : 'P');
        if (pointeeConst) out << 'K';
        mangleCppType(out, pointee, triple, false, false);
        return;
    }
    if (type.isVoid() || type.isNeverType()) {
        out << 'v';
        return;
    }
    if (type.isBool()) {
        out << 'b';
        return;
    }
    if (type.isChar()) {
        out << 'c';
        return;
    }
    if (type.isInt8() || type.isCSChar()) {
        out << 'a';
        return;
    }
    if (type.isUInt8() || type.isCUChar()) {
        out << 'h';
        return;
    }
    if (type.isInt16() || type.isCShort()) {
        out << 's';
        return;
    }
    if (type.isUInt16() || type.isCUShort()) {
        out << 't';
        return;
    }
    if (type.isInt32() || type.isCInt()) {
        out << 'i';
        return;
    }
    if (type.isUInt32() || type.isCUInt()) {
        out << 'j';
        return;
    }
    if (type.isCLong()) {
        out << 'l';
        return;
    }
    if (type.isCULong()) {
        out << 'm';
        return;
    }
    if (type.isCLongLong()) {
        out << 'x';
        return;
    }
    if (type.isCULongLong()) {
        out << 'y';
        return;
    }
    if (type.isFloat32() || type.isCFloat()) {
        out << 'f';
        return;
    }
    if (type.isFloat64() || type.isCDouble()) {
        out << 'd';
        return;
    }
    if (type.isCSizeT()) {
        // size_t mangles as its underlying integer type, which is platform-dependent.
        out << (!triple.isArch64Bit() ? 'j' : triple.isOSWindows() ? 'y' : 'm');
        return;
    }
    if (type.isInt64() || type.isUInt64()) {
        ERROR_RANGE(type.location, type.endLocation,
                    "integer type '" << type
                                     << "' has no single C++ counterpart; use c_long, c_ulong, c_longlong, or c_ulonglong in extern \"C++\" signatures");
    }
    if (type.isFloat80()) {
        ERROR_RANGE(type.location, type.endLocation, "'float80' has no C++ counterpart; use c_double in extern \"C++\" signatures");
    }
    // Slices and arrays are generic types with declarations, so reject them before the struct rule below.
    if (type.isSlice()) ERROR_RANGE(type.location, type.endLocation, "slices cannot be used in extern \"C++\" signatures; pass a pointer and length instead");
    if (type.isArrayType()) {
        ERROR_RANGE(type.location, type.endLocation, "arrays cannot be used in extern \"C++\" signatures; pass a pointer instead");
    }
    if (type.isOptionalType()) {
        Type wrapped = type.getWrappedType();
        if (wrapped.isPointerType() && wrapped.getPointerKind() == PointerKind::Pointer) {
            mangleCppType(out, wrapped, triple, false, false, pointeeConst); // nullable pointers pass as raw pointers
            return;
        }
        ERROR_RANGE(type.location, type.endLocation, "type '" << type << "' cannot be used in extern \"C++\" signatures");
    }
    if (type.isFunctionType()) {
        auto& functionType = llvm::cast<FunctionType>(*type);
        if (byValue && isReturn)
            ERROR_RANGE(type.location, type.endLocation, "functions cannot be returned in extern \"C++\" signatures; return a function pointer instead");
        // Function parameters decay to pointers, like array parameters in C.
        if (byValue) out << 'P';
        out << 'F';
        mangleCppType(out, functionType.returnType, triple, true, true);
        for (Type paramType : functionType.paramTypes)
            mangleCppType(out, paramType, triple, true, false);
        if (functionType.paramTypes.empty() && !functionType.isVariadic) out << 'v';
        if (functionType.isVariadic) out << 'z';
        out << 'E';
        return;
    }
    if (TypeDecl* typeDecl = type.getDecl()) {
        if (typeDecl->isEnumDecl())
            ERROR_RANGE(type.location, type.endLocation, "enums cannot be used in extern \"C++\" signatures; pass the underlying integer instead");
        if (!type.getGenericArgs().empty())
            ERROR_RANGE(type.location, type.endLocation, "generic type '" << type << "' cannot be used in extern \"C++\" signatures");
        if (!typeDecl->isStruct() && typeDecl->tag != TypeTag::Union)
            ERROR_RANGE(type.location, type.endLocation, "type '" << type << "' cannot be used in extern \"C++\" signatures");
        // Small-struct returns miscompile (returns lack the parameter integer-chunk
        // coercion), so reject all by-value struct returns until that is fixed.
        if (byValue && isReturn) {
            ERROR_RANGE(type.location, type.endLocation,
                        "type '" << type << "' cannot be returned by value in extern \"C++\" signatures; use an out-parameter instead");
        }
        if (byValue && type.needsDestruction()) {
            ERROR_RANGE(
                type.location, type.endLocation,
                "type '"
                    << type
                    << "' cannot be passed by value in extern \"C++\" signatures because it needs destruction; pass it behind a pointer or reference instead");
        }
        if (byValue && containsFloat(type)) {
            ERROR_RANGE(type.location, type.endLocation,
                        "type '" << type
                                 << "' cannot be passed by value in extern \"C++\" signatures because it contains floating-point members; pass it "
                                    "behind a pointer or reference instead");
        }
        if (byValue) {
            // Larger or over-aligned aggregates cross indirectly in cx but in memory or registers
            // in C++, so only small, normally-aligned structs cross by value.
            auto layout = cValueLayout(type);
            if (!layout) {
                ERROR_RANGE(type.location, type.endLocation,
                            "type '" << type
                                     << "' cannot be passed by value in extern \"C++\" signatures because it has a member with no C++ counterpart; "
                                        "pass it behind a pointer or reference instead");
            }
            if (layout->size == 0) {
                ERROR_RANGE(
                    type.location, type.endLocation,
                    "type '" << type
                             << "' cannot be passed by value in extern \"C++\" signatures because it is empty; pass it behind a pointer or reference instead");
            }
            if (layout->size > 16) {
                ERROR_RANGE(type.location, type.endLocation,
                            "type '" << type << "' cannot be passed by value in extern \"C++\" signatures because it is " << layout->size
                                     << " bytes; only structs up to 16 bytes can cross by value, pass it behind a pointer or reference instead");
            }
            if (layout->align > 8) {
                ERROR_RANGE(type.location, type.endLocation,
                            "type '" << type << "' cannot be passed by value in extern \"C++\" signatures because it requires " << layout->align
                                     << "-byte alignment; pass it behind a pointer or reference instead");
            }
        }
        out << typeDecl->getName().size() << typeDecl->getName();
        return;
    }
    ERROR_RANGE(type.location, type.endLocation, "type '" << type << "' cannot be used in extern \"C++\" signatures");
}

// True when a by-value `extern` type holds an 80-bit float anywhere. Those
// have no register coercion (the x87 class crosses in memory), unlike
// 32/64-bit floats, which the LLVM backend classifies per platform.
static bool containsLongDouble(Type type) {
    return containsFloatWhere(type, [](Type t) { return t.isFloat80(); });
}

// Rejects types that cannot cross an `extern "C"` boundary by value:
// fixed-array returns (C cannot return arrays), aggregates with no C
// counterpart, ones holding 80-bit floats (which cross in memory), and, on
// targets without register classification, small float-containing ones.
// Larger ones cross indirectly and are fine.
static void validateExternCByValue(Type type, bool isReturn) {
    // Bare floating-point scalars cross in their own register; only aggregates need the check below.
    if (type.isFloatingPoint()) return;
    // Fixed-array parameters decay to pointers, but C cannot return arrays.
    if (type.isFixedArray()) {
        if (!isReturn) return;
        ERROR_RANGE(type.location, type.endLocation,
                    "type '" << type
                             << "' cannot be returned by value in extern \"C\" signatures because C functions cannot return arrays; "
                                "use an out-parameter instead");
    }
    bool aggregate = false;
    if (TypeDecl* typeDecl = type.getDecl()) aggregate = typeDecl->isStruct() || typeDecl->tag == TypeTag::Union || typeDecl->isEnumDecl();
    if (!aggregate || !containsFloat(type)) return;
    const char* action = isReturn ? "returned" : "passed";
    const char* instead = isReturn ? "use an out-parameter instead" : "pass it behind a pointer or reference instead";
    auto layout = cValueLayout(type);
    if (!layout) {
        ERROR_RANGE(type.location, type.endLocation,
                    "type '" << type << "' cannot be " << action
                             << " by value in extern \"C\" signatures because it contains "
                                "floating-point members and has a member with no C counterpart; "
                             << instead);
    }
    if (containsLongDouble(type)) {
        ERROR_RANGE(type.location, type.endLocation,
                    "type '" << type << "' cannot be " << action
                             << " by value in extern \"C\" signatures because it contains 80-bit float members, which cross in memory; " << instead);
    }
    if (layout->size > 16) return;
    llvm::Triple triple(llvm::sys::getDefaultTargetTriple());
    if (!triple.isX86_64() && !triple.isAArch64()) {
        ERROR_RANGE(type.location, type.endLocation,
                    "type '" << type << "' cannot be " << action << " by value in extern \"C\" signatures because it is " << layout->size
                             << " bytes and contains floating-point members; " << instead);
    }
}

void cx::validateCVariadicExtra(Type type, const Expr& arg) {
    // Fixed arrays decay to pointers in variadic calls; everything else obeys
    // the signature rules, located at the argument instead of a parameter.
    if (type.isFixedArray()) return;
    validateExternCByValue(type.withLocation(getExprRangeStart(arg), arg.endLocation), false);
}

// Computes the Itanium-mangled symbol for an `extern "C++"` declaration, used
// both when importing a C++ function and when exporting a cx one to C++.
static void mangleCppFunction(FunctionDecl& decl) {
    llvm::Triple triple(llvm::sys::getDefaultTargetTriple());
    if (triple.isWindowsMSVCEnvironment()) {
        ERROR_RANGE(decl.getLocation(), getIdentifierEndLocation(decl), "extern \"C++\" uses the Itanium ABI, which is not supported on MSVC targets");
    }
    llvm::StringRef name = decl.getName();
    auto isIdentifierChar = [](char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_'; };
    if (name.empty() || !llvm::all_of(name, isIdentifierChar)) {
        ERROR_RANGE(decl.getLocation(), getIdentifierEndLocation(decl), "extern \"C++\" functions must have a plain identifier name");
    }
    std::string mangled;
    llvm::raw_string_ostream out(mangled);
    out << "_Z" << name.size() << name;
    for (const ParamDecl& param : decl.getParams()) {
        if (param.cxxConstPointee) {
            Type target = param.type.removeOptional();
            if (!target.isPointerType() || target.getPointee().isPointerType() || target.getPointee().isArrayPointer()
                || target.getPointee().isFunctionType()) {
                ERROR_RANGE(param.getLocation(), getIdentifierEndLocation(param),
                            "'const' must qualify a single-level pointee in extern \"C++\" signatures (e.g. 'const T*' or 'const T&')");
            }
            if (decl.body) {
                ERROR_RANGE(param.getLocation(), getIdentifierEndLocation(param),
                            "'const' parameters are only allowed on extern \"C++\" declarations, not definitions");
            }
        }
        mangleCppType(out, param.type, triple, true, false, param.cxxConstPointee);
    }
    if (decl.getParams().empty() && !decl.isVariadic()) out << 'v';
    if (decl.isVariadic()) out << 'z';
    out.flush();
    if (decl.getReturnType()) {
        // The return type is not part of the symbol, but it must still have a C++ counterpart.
        std::string discarded;
        llvm::raw_string_ostream discard(discarded);
        mangleCppType(discard, decl.getReturnType(), triple, true, true);
    }
    // The \01 marker bypasses LLVM's target symbol prefix, so add the Mach-O/MinGW '_' explicitly.
    if (triple.isOSBinFormatMachO() || triple.isOSCygMing()) mangled = "_" + mangled;
    decl.proto.asmLabel = internString(mangled);
}

void Typechecker::typecheckFunctionSignature(FunctionDecl& decl) {
    if (decl.checkState != Decl::CheckState::Unchecked) return;
    decl.checkState = Decl::CheckState::CheckingSignature;
    llvm::SaveAndRestore saveModule(currentModule);
    llvm::SaveAndRestore saveFile(currentSourceFile);
    llvm::SaveAndRestore setCheckingSignature(checkingFunctionSignature, true);
    setDeclContext(decl);
    try {
        for (auto& param : decl.proto.params) {
            param.type = resolveTypeAliases(param.type, decl.accessLevel, /*foldArraySizes=*/true);
        }
        decl.proto.returnType = resolveTypeAliases(decl.proto.returnType, decl.accessLevel, /*foldArraySizes=*/true);

        if (decl.hasPack()) {
            ERROR_RANGE(decl.getPackParam()->getLocation(), getIdentifierEndLocation(*decl.getPackParam()), "variadic parameter requires a generic function");
        }

        Scope scope(&decl, &currentModule->symbolTable);
        llvm::SaveAndRestore setCurrentFunction(currentFunction, &decl);

        typecheckParams(decl.getParams(), decl.accessLevel);

        if (!decl.isConstructorDecl() && !decl.isDestructorDecl() && decl.getReturnType()) {
            // Element accessors (e.g. List.front, Map.operator[]) return borrows into the container.
            typecheckType(decl.getReturnType(), decl.accessLevel, true, true);
        }

        if (decl.proto.cppLinkage) {
            mangleCppFunction(decl);
        }

        if (decl.isExtern() && !decl.proto.cppLinkage) {
            for (const ParamDecl& param : decl.getParams())
                validateExternCByValue(param.type, false);
            if (decl.getReturnType()) validateExternCByValue(decl.getReturnType(), true);
        }

        if ((!decl.isExtern() || decl.body) && decl.isMain() && !decl.isMethodDecl() && decl.getModule() == mainModule && decl.genericArgs.empty()) {
            if (entryMain) {
                REPORT_ERROR_WITH_NOTES(decl.getLocation(), getPreviousDefinitionNotes(entryMain), "multiple definitions of 'main'");
            } else {
                entryMain = &decl;
                decl.isEntryPoint = true;
            }
            checkMainSignature(decl);
        }
    } catch (const CompileError&) {
        // Leave the partial signature in place (as eager checking would) but
        // report only once: later uses see SignatureChecked and don't rethrow.
        decl.checkState = Decl::CheckState::SignatureChecked;
        throw;
    }
    decl.checkState = Decl::CheckState::SignatureChecked;
}

void Typechecker::typecheckFunctionDecl(FunctionDecl& decl) {
    if (decl.checkState == Decl::CheckState::Checked || decl.checkState == Decl::CheckState::CheckingBody) return;
    typecheckFunctionSignature(decl);
    if (decl.isExtern() && !decl.body) {
        decl.checkState = Decl::CheckState::Checked;
        return;
    }
    // Value parameters are owned by the body: it drops them at exit, but their
    // types are only mentioned in the signature, where destructor marking is
    // skipped (see checkingFunctionSignature).
    for (auto& param : decl.getParams()) {
        markDestructorFor(param.type);
    }
    decl.checkState = Decl::CheckState::CheckingBody;
    llvm::SaveAndRestore saveModule(currentModule);
    llvm::SaveAndRestore saveFile(currentSourceFile);
    setDeclContext(decl);
    try {
        int errorsBefore = errors;
        llvm::SaveAndRestore saveNarrowings(narrowedTypes, NarrowMap{});
        // Lambda bodies are checked inline within the enclosing function; moves they record
        // must not clobber the enclosing move state, which is restored when the body is done.
        llvm::SaveAndRestore saveMovedDecls(movedDecls, movedDecls);
        llvm::SaveAndRestore saveMaybeMovedDecls(maybeMovedDecls, maybeMovedDecls);
        llvm::SaveAndRestore saveMoveLocations(moveLocations, moveLocations);
        llvm::SaveAndRestore saveCondWarnedDecls(condWarnedDecls, condWarnedDecls);
        llvm::SaveAndRestore saveAssignedDecls(definitelyAssignedDecls, definitelyAssignedDecls);
        // 'break' and 'continue' must not cross function boundaries into enclosing loops or switches.
        llvm::SaveAndRestore saveControlStmts(currentControlStmts, std::vector<Stmt*>());
        llvm::SaveAndRestore saveLocalVarDecls(localVarDecls, std::vector<VarDecl*>());
        llvm::SaveAndRestore saveLoopEntryLocalCount(loopEntryLocalCount, std::optional<size_t>());
        llvm::SaveAndRestore saveAssignTarget(assignTarget, static_cast<Decl*>(nullptr));
        llvm::SaveAndRestore saveInReturnValue(inReturnValue, false);

        TypeDecl* receiverTypeDecl = decl.getTypeDecl();
        // Methods reached by name (e.g. interface copies in the module table)
        // must see their receiver's fields; methods reached through the type
        // find it already ensured.
        if (receiverTypeDecl) {
            ensureSignature(*receiverTypeDecl);
        }

        Scope scope(&decl, &currentModule->symbolTable);
        llvm::SaveAndRestore setCurrentFunction(currentFunction, &decl);

        // The signature phase registered the parameters in its own scope,
        // which has since popped; re-add them (already checked) for the body.
        for (auto& param : decl.getParams()) {
            if (!param.getName().empty()) {
                currentModule->symbolTable.add(param.getName(), &param);
            }
        }

        llvm::SmallPtrSet<FieldDecl*, 32> initializedFields;
        llvm::SaveAndRestore setInitializedFields(currentInitializedFields, &initializedFields);

        if (receiverTypeDecl) {
            Type thisType = PointerType::get(receiverTypeDecl->getType(), PointerKind::Reference).withLocation(receiverTypeDecl->getLocation());
            auto* varDecl = makeAST<VarDecl>(thisType, "this", nullptr, &decl, AccessLevel::None, *currentModule, decl.getLocation());
            currentModule->addToSymbolTable(varDecl);
            definitelyAssignedDecls.insert(varDecl);
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

            // This prevents creating destructors calls during codegen. Maybe-moved
            // values (only from conditional expressions now) are destroyed on
            // no path: skipping the call is sound but leaks the value on
            // paths where it is still live.
            movedDecls.insert(maybeMovedDecls.begin(), maybeMovedDecls.end());
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
            maybeMovedDecls.clear();
            moveLocations.clear();
            condWarnedDecls.clear();
        }

        if (decl.isConstructorDecl() && !delegatedInit) {
            for (auto& field : decl.getTypeDecl()->fields) {
                if (!field.defaultValue && initializedFields.count(&field) == 0) {
                    // Constructors are spelled with the type name, not the synthetic "init".
                    WARN_RANGE(decl.getLocation(), getIdentifierEndLocation(decl.getLocation(), decl.getTypeDecl()->getName()),
                               "constructor doesn't initialize member variable '" << field.getName() << "'");
                }
            }
        }

        bool assertsOn = assertsEnabled(options.mode, decl.isTest);
        if ((!receiverTypeDecl || !receiverTypeDecl->isInterface()) && !decl.getReturnType().isVoid() && !allPathsReturn(*decl.body, assertsOn)) {
            if (decl.getReturnType().isNeverType()) {
                WARN_RANGE(decl.getLocation(), getIdentifierEndLocation(decl), "'" << decl.getName() << "' is declared to never return but it does return");
            } else {
                REPORT_ERROR_RANGE(decl.getLocation(), getIdentifierEndLocation(decl), "'" << decl.getName() << "' is missing a return statement");
            }
        }

        // Don't warn about unused variables in the standard library or after errors.
        if (errors == errorsBefore && decl.getModule()->name != "std" && !options.noUnusedWarnings) {
            for (auto* varDecl : localVarDecls) {
                if (!varDecl->isReferenced() && !varDecl->getName().starts_with("_")) {
                    WARN_RANGE(varDecl->getLocation(), getIdentifierEndLocation(*varDecl),
                               "unused variable '" << varDecl->getName() << "'; prefix with '_' to suppress");
                }
            }
        }
    } catch (const CompileError&) {
        decl.checkState = Decl::CheckState::Checked;
        throw;
    }
    decl.checkState = Decl::CheckState::Checked;
}

void Typechecker::typecheckFunctionTemplate(FunctionTemplate& decl) {
    // Patterns check once; instantiations are separate declarations that check on use.
    if (decl.checkState == Decl::CheckState::Checked) return;
    decl.checkState = Decl::CheckState::Checked;
    if (decl.functionDecl->isMain() && !decl.functionDecl->isMethodDecl() && decl.getModule() == mainModule) {
        ERROR_RANGE(decl.getLocation(), getIdentifierEndLocation(decl), "'main' cannot be generic");
    }
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
            ERROR_RANGE(params.back().getLocation(), getIdentifierEndLocation(params.back()),
                        "generic parameter '" << genericParam.getName() << "' cannot be used in both fixed and variadic parameters");
        }
        Type returnType = functionDecl->getReturnType();
        if (inPack && returnType && containsGenericParam(returnType, genericParam.getName())) {
            ERROR_RANGE(returnType.location, returnType.endLocation,
                        "variadic generic parameter '" << genericParam.getName() << "' cannot be used in return type");
        }
    }
}

void Typechecker::ensureInterfaces(TypeDecl& decl) {
    if (decl.interfacesEnsured) return;
    decl.interfacesEnsured = true;
    llvm::StringMap<GenericArg> genericArgs = {{"This", GenericArg(decl.getType())}};

    for (Type interface : decl.interfaces) {
        try {
            // Interfaces constrain but never store, so borrows may appear in them.
            typecheckType(interface, decl.accessLevel, true, true);
        } catch (const CompileError& error) {
            error.report();
        }
        if (!interface.getDecl()) continue;
        // Inheriting from a non-interface is meaningless; the conformance check reports it.
        if (!interface.getDecl()->isInterface()) continue;

        // Enums have no fields, so an interface field requirement fails conformance instead.
        if (!decl.isEnumDecl()) {
            std::vector<FieldDecl> inheritedFields;

            for (auto& field : interface.getDecl()->fields) {
                auto duplicate = llvm::find_if(decl.fields, [&](const FieldDecl& f) { return f.getName() == field.getName(); });
                if (duplicate != decl.fields.end()) {
                    WARN(duplicate->getLocation(),
                         "field '" << field.getName() << "' duplicates inherited field from interface '" << interface.getDecl()->getName() << "'");
                }
                inheritedFields.push_back(field.instantiate(genericArgs, decl));
            }

            decl.fields.insert(decl.fields.begin(), inheritedFields.begin(), inheritedFields.end());
        }

        for (auto member : interface.getDecl()->methods) {
            auto methodDecl = llvm::cast<MethodDecl>(member);
            if (methodDecl->body) {
                auto copy = methodDecl->instantiate(genericArgs, {}, decl);
                currentModule->addToSymbolTable(*copy);
                decl.addMethod(copy);
            }
        }
    }

    // The parser-generated constructor misses inherited fields, so
    // regenerate it now that they're added. Duplicate field names
    // can't form parameters; leave the parser version in that case.
    if (decl.isStruct() && !decl.interfaces.empty()) {
        llvm::SmallDenseSet<llvm::StringRef, 8> fieldNames;
        bool hasDuplicates = false;
        for (auto& field : decl.fields) {
            if (!fieldNames.insert(field.getName()).second) {
                hasDuplicates = true;
                break;
            }
        }
        if (!hasDuplicates) {
            auto& methods = decl.methods;
            auto newEnd = llvm::remove_if(methods, [](Decl* decl) {
                auto* ctor = llvm::dyn_cast<ConstructorDecl>(decl);
                return ctor && ctor->isAutogenerated;
            });
            if (newEnd != methods.end()) {
                methods.erase(newEnd, methods.end());
                decl.addAutogeneratedConstructor();
            }
        }
    }
    // Inherited fields arrived after instantiation, so their nested generics
    // were never walked.
    ensureNestedInstantiations(decl);
}

// Members of generic instantiations carry template-definition locations;
// access warnings for them would duplicate the use-site checks, so suppress.
struct DeclContextScope {
    llvm::SaveAndRestore<bool> suppressAccessWarnings;
    llvm::SaveAndRestore<Module*> currentModule;
    llvm::SaveAndRestore<SourceFile*> currentSourceFile;

    DeclContextScope(Typechecker& checker, TypeDecl& decl)
    : suppressAccessWarnings(checker.suppressAccessWarnings, checker.suppressAccessWarnings || decl.instantiatedFrom != nullptr),
      currentModule(checker.currentModule), currentSourceFile(checker.currentSourceFile) {
        checker.setDeclContext(decl);
    }
};

static bool isCopyableConstraint(Type interface) {
    return interface.isBasicType() && interface.getName() == "Copyable";
}

static void checkDeclaredInterfaces(Typechecker& checker, TypeDecl& decl) {
    checker.ensureInterfaces(decl);

    if (decl.isUnion() && !decl.interfaces.empty()) {
        for (Type interface : decl.interfaces) {
            REPORT_ERROR_RANGE(interface.location, interface.endLocation, "unions cannot implement interfaces");
        }
        decl.interfaces.clear();
        return;
    }

    // Conformance runs before methods are checked, comparing raw signatures on both sides.
    for (Type interface : decl.interfaces) {
        if (isCopyableConstraint(interface)) {
            REPORT_ERROR_RANGE(interface.location, interface.endLocation,
                               "': Copyable' is not allowed; types are Copyable by default unless they declare a destructor or hold "
                               "a non-Copyable field");
            continue;
        }
        // Interfaces constrain but never store, so borrows may appear in them (e.g. Iterator<Element&>).
        checker.typecheckType(interface, decl.accessLevel, true, true);
        auto* interfaceDecl = interface.getDecl();

        if (!interfaceDecl->isInterface()) {
            REPORT_ERROR_RANGE(interface.location, interface.endLocation, "'" << interface << "' is not an interface");
            continue;
        }

        std::string errorReason;
        if (!checker.providesInterfaceRequirements(decl, *interfaceDecl, &errorReason)) {
            REPORT_ERROR_RANGE(decl.getLocation(), getIdentifierEndLocation(decl),
                               "'" << decl.getName() << "' " << errorReason << " required by interface '" << interfaceDecl->getName() << "'");
        }
    }

    // Drop the rejected entry so later queries never see it.
    decl.interfaces.erase(llvm::remove_if(decl.interfaces, isCopyableConstraint), decl.interfaces.end());
}

static void checkForInfiniteSizeEarly(Typechecker& checker, TypeDecl& decl, llvm::ArrayRef<Type> memberTypes) {
    try {
        checkForInfiniteSize(decl, memberTypes);
        return;
    } catch (const CompileError& error) {
        error.report();
        checker.infiniteSizeReported.insert(&decl);
    }
}

static void checkForInfiniteSizeLate(Typechecker& checker, const TypeDecl& decl, llvm::ArrayRef<Type> memberTypes) {
    if (!checker.infiniteSizeReported.contains(&decl)) checkForInfiniteSize(decl, memberTypes);
}

static void diagnoseStaticRedefinition(const VarDecl& constant) {
    ERROR_RANGE(constant.getLocation(), getIdentifierEndLocation(constant), "redefinition of '" << constant.getName() << "'");
}

static void checkStaticConstNames(llvm::ArrayRef<VarDecl*> constants, llvm::function_ref<bool(llvm::StringRef)> conflictsWith) {
    for (size_t i = 0; i < constants.size(); ++i) {
        auto* constant = constants[i];
        for (size_t j = 0; j < i; ++j) {
            if (constants[j]->getName() == constant->getName()) diagnoseStaticRedefinition(*constant);
        }
        if (conflictsWith(constant->getName())) diagnoseStaticRedefinition(*constant);
    }
}

static void typecheckMethodBodies(Typechecker& checker, TypeDecl& decl) {
    if (decl.checkState == Decl::CheckState::Checked || decl.checkState == Decl::CheckState::CheckingBody) return;
    decl.checkState = Decl::CheckState::CheckingBody;
    DeclContextScope scope(checker, decl);
    try {
        TypeDecl* realDecl = decl.isInterface() ? llvm::cast<TypeDecl>(decl.instantiate({{"This", decl.getType()}}, {})) : &decl;
        for (auto& methodDecl : realDecl->methods) {
            checker.typecheckMethodDecl(*methodDecl);
        }
    } catch (const CompileError&) {
        decl.checkState = Decl::CheckState::Checked;
        throw;
    }
    decl.checkState = Decl::CheckState::Checked;
}

void Typechecker::typecheckTypeSignature(TypeDecl& decl) {
    if (decl.checkState != Decl::CheckState::Unchecked) return;
    decl.checkState = Decl::CheckState::CheckingSignature;
    DeclContextScope scope(*this, decl);
    try {
        checkDeclaredInterfaces(*this, decl);

        TypeDecl* realDecl;

        if (decl.isInterface()) {
            realDecl = llvm::cast<TypeDecl>(decl.instantiate({{"This", decl.getType()}}, {}));
        } else {
            realDecl = &decl;
        }

        // Report this type's own size error before descending into members: member checks
        // ensure nested declarations whose errors would otherwise precede it. Type links are
        // parse-time (types are canonicalized), so non-generic cycles are visible already;
        // cycles through not-yet-instantiated generics are caught by the late check below.
        checkForInfiniteSizeEarly(*this, decl, map(realDecl->fields, [](const FieldDecl& field) { return field.type; }));

        for (auto& fieldDecl : realDecl->fields) {
            typecheckFieldDecl(fieldDecl);
        }

        // A manually-destroyed owning field is never destroyed automatically,
        // so the struct must declare a destructor that destroys it explicitly.
        if (decl.getDestructor() == nullptr) {
            for (auto& fieldDecl : realDecl->fields) {
                if (fieldDecl.isManuallyDestroy && fieldDecl.type.needsDestruction()) {
                    ERROR_RANGE(fieldDecl.getLocation(), getIdentifierEndLocation(fieldDecl),
                                (decl.isUnion() ? "union '" : "struct '") << decl.getName() << "' has a '@manuallyDestroy' field but declares no destructor");
                }
            }
        }

        for (auto& methodDecl : realDecl->methods) {
            ensureSignature(*methodDecl);
        }

        // Static constants share the value namespace with fields; duplicates would
        // make `Type.name` and `instance.name` resolve differently (type access
        // prefers the constant, instance access prefers the field).
        checkStaticConstNames(realDecl->staticConsts, [&](llvm::StringRef name) {
            return llvm::any_of(realDecl->fields, [&](const FieldDecl& field) { return field.getName() == name; });
        });

        checkForInfiniteSizeLate(*this, decl, map(realDecl->fields, [](const FieldDecl& field) { return field.type; }));
    } catch (const CompileError&) {
        decl.checkState = Decl::CheckState::SignatureChecked;
        throw;
    }
    decl.checkState = Decl::CheckState::SignatureChecked;
}

void Typechecker::typecheckTypeDecl(TypeDecl& decl) {
    typecheckTypeSignature(decl);
    typecheckMethodBodies(*this, decl);
}

void Typechecker::typecheckTypeTemplate(TypeTemplate& decl) {
    if (decl.checkState == Decl::CheckState::Checked) return;
    decl.checkState = Decl::CheckState::Checked;
    typecheckGenericParamDecls(decl.genericParams, decl.accessLevel);
}

void Typechecker::typecheckTypeAliasDecl(TypeAliasDecl& decl) {
    if (decl.checkState == Decl::CheckState::Checked) return;
    decl.checkState = Decl::CheckState::Checked;
    typecheckType(resolveTypeAliases(decl.aliasedType, decl.accessLevel), decl.accessLevel, true, true);
}

void Typechecker::typecheckEnumSignature(EnumDecl& decl) {
    if (decl.checkState != Decl::CheckState::Unchecked) return;
    decl.checkState = Decl::CheckState::CheckingSignature;
    DeclContextScope scope(*this, decl);
    try {
        checkDeclaredInterfaces(*this, decl);

        std::vector<const EnumCase*> cases = map(decl.cases, [](const EnumCase& c) { return &c; });
        std::ranges::sort(cases, [](auto* a, auto* b) { return a->getName() < b->getName(); });
        auto it = std::ranges::adjacent_find(cases, [](auto* a, auto* b) { return a->getName() == b->getName(); });

        if (it != cases.end()) {
            ERROR_RANGE((*it)->getLocation(), getIdentifierEndLocation((**it)), "duplicate enum case '" << (*it)->getName() << "'");
        }

        bool allowReference = allowsSubstitutedReference(decl);

        // Report this type's own size error before descending into members (see the struct
        // case above); cycles through not-yet-instantiated generics fall to the late check.
        checkForInfiniteSizeEarly(*this, decl, map(decl.cases, [](const EnumCase& enumCase) { return enumCase.associatedType; }));

        for (auto& enumCase : decl.cases) {
            typecheckExpr(*enumCase.value);

            if (enumCase.associatedType) {
                enumCase.associatedType = resolveTypeAliases(enumCase.associatedType, enumCase.accessLevel, /*foldArraySizes=*/true);
                // Payloads are storage, like fields: mark even during signature checks.
                llvm::SaveAndRestore unguard(checkingFunctionSignature, false);
                typecheckType(enumCase.associatedType, enumCase.accessLevel, true, allowReference);
            }
        }

        for (auto& methodDecl : decl.methods) {
            ensureSignature(*methodDecl);
        }

        // Static constants share the value namespace with cases; duplicates would
        // make `Enum.name` ambiguous between a case and a constant.
        checkStaticConstNames(decl.staticConsts, [&](llvm::StringRef name) {
            return llvm::any_of(decl.cases, [&](const EnumCase& enumCase) { return enumCase.getName() == name; });
        });

        checkForInfiniteSizeLate(*this, decl, map(decl.cases, [](const EnumCase& enumCase) { return enumCase.associatedType; }));
    } catch (const CompileError&) {
        decl.checkState = Decl::CheckState::SignatureChecked;
        throw;
    }
    decl.checkState = Decl::CheckState::SignatureChecked;
}

void Typechecker::typecheckEnumDecl(EnumDecl& decl) {
    typecheckEnumSignature(decl);
    typecheckMethodBodies(*this, decl);
}

// Global initializers are emitted as constant expressions; anything needing runtime
// evaluation would emit instructions outside any function and corrupt codegen.
static bool containsNonEmptyArrayLiteral(const Expr& expr) {
    if (auto* array = llvm::dyn_cast<ArrayLiteralExpr>(&expr)) return !array->elements.empty();
    if (auto* cast = llvm::dyn_cast<ImplicitCastExpr>(&expr)) return containsNonEmptyArrayLiteral(*cast->operand);
    return false;
}

static bool isSupportedConstInitializerImpl(const Expr& expr, llvm::SmallPtrSetImpl<const VarDecl*>& seen) {
    switch (expr.kind) {
    case ExprKind::IntLiteralExpr:
    case ExprKind::FloatLiteralExpr:
    case ExprKind::BoolLiteralExpr:
    case ExprKind::CharacterLiteralExpr:
    case ExprKind::StringLiteralExpr:
    case ExprKind::NullLiteralExpr:
    case ExprKind::UndefinedLiteralExpr:
    case ExprKind::SizeofExpr:
        return true;
    case ExprKind::ArrayLiteralExpr:
        for (auto& element : llvm::cast<ArrayLiteralExpr>(expr).elements) {
            if (!isSupportedConstInitializerImpl(*element, seen)) return false;
        }
        return true;
    case ExprKind::VarExpr: {
        auto* decl = llvm::cast<VarExpr>(expr).decl;
        if (!decl) return true; // Unresolved references are diagnosed elsewhere, don't cascade.
        if (llvm::isa<FunctionDecl>(decl)) return true;
        auto* varDecl = llvm::dyn_cast<VarDecl>(decl);
        // Immutable globals inline their initializer; mutable ones need a runtime load.
        if (!varDecl || !varDecl->isConst || !varDecl->initializer || !seen.insert(varDecl).second) return false;
        // Path-scoped: constants shared between converging paths stay supported, only true cycles fail.
        bool result = isSupportedConstInitializerImpl(*varDecl->initializer, seen);
        seen.erase(varDecl);
        return result;
    }
    case ExprKind::MemberExpr: {
        // Tag-only cases lower to their tag constant; payload cases need runtime construction.
        if (auto* enumCase = llvm::dyn_cast_or_null<EnumCase>(llvm::cast<MemberExpr>(expr).decl)) {
            if (!enumCase->getEnumDecl()->hasAssociatedValues()) return true;
        }
        // Foldable member reads (e.g. `.x` on a constant array) evaluate before codegen.
        return (expr.type.isInteger() && expr.isFoldableIntConstant()) || (expr.type.isBool() && expr.isFoldableBoolConstant());
    }
    case ExprKind::UnaryExpr: {
        auto& unary = llvm::cast<UnaryExpr>(expr);
        switch (unary.op) {
        case Token::Plus:
        case Token::Minus:
        case Token::MinusWrap:
        case Token::Tilde:
            // Enum tags lower to integers; pointers and optionals would miscompile.
            // (Bool is only reachable for '~'; ill-typed '-bool' never gets here.)
            return (expr.type.isInteger() || expr.type.isBool() || expr.type.isFloatingPoint() || expr.type.isChar()
                    || (expr.type.isEnumType() && !expr.type.isOptionalType()))
                && isSupportedConstInitializerImpl(unary.getOperand(), seen);
        case Token::Not:
            return unary.getOperand().type.isBool() && isSupportedConstInitializerImpl(unary.getOperand(), seen);
        case Token::And: {
            // Addresses of globals and functions are constants, but const globals have no storage.
            auto* varExpr = llvm::dyn_cast<VarExpr>(&unary.getOperand());
            if (!varExpr) return false;
            if (!varExpr->decl) return true;
            if (auto* varDecl = llvm::dyn_cast<VarDecl>(varExpr->decl)) {
                return varDecl->isGlobal() && !varDecl->isConst;
            }
            return llvm::isa<FunctionDecl>(varExpr->decl);
        }
        default:
            return false;
        }
    }
    case ExprKind::BinaryExpr: {
        auto& binary = llvm::cast<BinaryExpr>(expr);
        if (!isSupportedConstInitializerImpl(binary.getLHS(), seen) || !isSupportedConstInitializerImpl(binary.getRHS(), seen)) {
            return false;
        }
        // && and || have no constant instruction form; only foldable ones are supported.
        if (binary.op == Token::AndAnd || binary.op == Token::OrOr) {
            return expr.isFoldableBoolConstant();
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
        case Token::PlusWrap:
        case Token::MinusWrap:
        case Token::StarWrap:
        case Token::PlusSat:
        case Token::MinusSat:
        case Token::StarSat:
        case Token::LeftShiftSat:
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
    case ExprKind::CallExpr: {
        // Autogenerated constructors just copy arguments into fields, so the call is pure
        // data when every argument is. User constructors run arbitrary code, and owning
        // fields (strings, containers) need runtime construction; both stay rejected.
        auto& call = llvm::cast<CallExpr>(expr);
        auto* constructor = llvm::dyn_cast_or_null<ConstructorDecl>(call.calleeDecl);
        if (!constructor || !constructor->isAutogenerated || !expr.type.isImplicitlyCopyable()) return false;
        for (auto& arg : call.args) {
            if (!isSupportedConstInitializerImpl(*arg.value, seen)) return false;
        }
        return true;
    }
    case ExprKind::ImplicitCastExpr: {
        auto& cast = llvm::cast<ImplicitCastExpr>(expr);
        // Pointer-implemented optionals need no construction; value-implemented ones are built as
        // constant aggregates. Other casts need loads or branches.
        return cast.castKind == ImplicitCastExpr::OptionalWrap && expr.type.isOptionalType() && isSupportedConstInitializerImpl(*cast.operand, seen);
    }
    case ExprKind::IfExpr: {
        // Ternaries emit branches unless folded; only integer and boolean ones fold.
        auto& ifExpr = llvm::cast<IfExpr>(expr);
        if (!isSupportedConstInitializerImpl(*ifExpr.condition, seen) || !isSupportedConstInitializerImpl(*ifExpr.thenExpr, seen)
            || !isSupportedConstInitializerImpl(*ifExpr.elseExpr, seen)) {
            return false;
        }
        return (expr.type.isInteger() && expr.isFoldableIntConstant()) || (expr.type.isBool() && expr.isFoldableBoolConstant());
    }
    default:
        return false;
    }
}

bool Typechecker::isSupportedConstInitializer(const Expr& expr) {
    llvm::SmallPtrSet<const VarDecl*, 8> seen;
    return isSupportedConstInitializerImpl(expr, seen);
}

void Typechecker::typecheckVarDecl(VarDecl& decl) {
    if (decl.checkState == Decl::CheckState::Checked) return;
    if (decl.checkState == Decl::CheckState::CheckingBody) {
        CompileError error(decl.getLocation(), (StringBuilder() << "cyclic reference to '" << decl.getName() << "' in its initializer").string, {},
                           getIdentifierEndLocation(decl));
        error.isCyclic = true;
        printStackTrace();
        throw error;
    }
    decl.checkState = Decl::CheckState::CheckingBody;
    // Checked on every exit, including errors (like signatures): the error
    // reports once and later uses see the partial state instead of rechecking.
    llvm::scope_exit markChecked([&decl] { decl.checkState = Decl::CheckState::Checked; });
    decl.type = resolveTypeAliases(decl.type, decl.isGlobal() ? decl.accessLevel : AccessLevel::None, /*foldArraySizes=*/true);
    if (!decl.isGlobal()) {
        localVarDecls.push_back(&decl);
    }

    Type declaredType = decl.type;
    if (declaredType) {
        // Locals may declare plain borrows ('int& r = x;') and optional borrows
        // ('int&? r = ...'). Other borrow-storing types stay rejected, as do all
        // borrows in globals.
        bool allowReference = !decl.isGlobal() && declaredType.isBorrowOrOptionalBorrow();
        typecheckType(declaredType, !decl.isGlobal() ? AccessLevel::None : decl.accessLevel, true, allowReference);
        if (declaredType.isVoid()) {
            ERROR_RANGE(decl.getLocation(), getIdentifierEndLocation(decl), "cannot declare variable '" << decl.getName() << "' of type 'void'");
        }
    }

    if (decl.initializer) {
        try {
            typecheckExpr(*decl.initializer, false, declaredType);
        } catch (const CompileError&) {
            if (!decl.isGlobal()) currentModule->addToSymbolTable(decl);
            throw;
        }
    }

    // Constants need a constant-expression initializer, except compiler-bound
    // temporaries, whose values come from elsewhere.
    bool isConstChecked = decl.isConst && !decl.isImplicitlyBound;
    if (!decl.isGlobal()) currentModule->addToSymbolTable(decl);
    if (!decl.initializer) {
        if (isConstChecked) {
            ERROR_RANGE(decl.getLocation(), getIdentifierEndLocation(decl), "const '" << decl.getName() << "' must have an initializer");
        }
        if (!declaredType) {
            ERROR_RANGE(decl.getLocation(), getIdentifierEndLocation(decl),
                        "couldn't infer type of '" << decl.getName() << "', add a type annotation or initializer");
        }
        if (declaredType.isReferenceType() && !decl.isGlobal() && !decl.isImplicitlyBound) {
            ERROR_RANGE(decl.getLocation(), getIdentifierEndLocation(decl),
                        "reference variable '" << decl.getName() << "' must be initialized (borrows cannot be rebound)");
        }
        if (decl.isGlobal()) {
            WARN_RANGE(decl.getLocation(), getIdentifierEndLocation(decl), "missing initializer");
        }
        return;
    }
    if (!decl.isGlobal()) {
        definitelyAssignedDecls.insert(&decl);
    }
    Type initializerType = decl.initializer->type;
    if (!initializerType) return;

    if (declaredType) {
        if (auto converted = convert(decl.initializer, declaredType)) {
            decl.initializer = converted;
        } else {
            std::string hint;

            if (initializerType.isNull() && !declaredType.isOptionalType()) {
                hint = " (add '?' to the type to make it nullable)";
            } else {
                hint = narrowingHint(initializerType, declaredType);
            }

            diagnoseClosureConversion(initializerType, declaredType, *decl.initializer);
            if (isBorrowOfConstant(*decl.initializer, initializerType, declaredType)) {
                // Initializing a borrow binds it; "assign" misdescribes what failed.
                ERROR_RANGE(getExprRangeStart(*decl.initializer), decl.initializer->endLocation,
                            "cannot bind '" << declaredType << "' to constant '" << initializerType << "'" << narrowingHint(initializerType, declaredType)
                                            << ambiguousConversionHint(decl.initializer, initializerType, declaredType));
            } else {
                ERROR_RANGE(getExprRangeStart(*decl.initializer), decl.initializer->endLocation,
                            "cannot assign '" << initializerType << "' to '" << declaredType << "'" << hint
                                              << ambiguousConversionHint(decl.initializer, initializerType, declaredType));
            }
        }
    } else {
        if (initializerType.isNull()) {
            ERROR_RANGE(decl.getLocation(), getIdentifierEndLocation(decl), "couldn't infer type of '" << decl.getName() << "', add a type annotation");
        }
        if (initializerType.isVoid()) {
            ERROR_RANGE(decl.getLocation(), getIdentifierEndLocation(decl), "cannot infer type of '" << decl.getName() << "' from expression of type 'void'");
        }

        decl.type = NOTNULL(initializerType);
    }

    // An inferred local borrow aliases the referent in place: `var x = list[0]` deduces
    // `Element&`, so writes through `x` affect the element. Globals still read the value
    // out (copying or moving it) since borrows cannot live in globals. Explicitly declared
    // reference locals and for-loop element variables are exempt for plain borrows; they
    // alias the referent in place. Optional borrows name the borrow-or-null value, which
    // uses null-check and unwrap like any optional. Other borrow-storing types stay rejected.
    bool explicitLocalBorrow = declaredType && declaredType.isBorrowOrOptionalBorrow() && !decl.isGlobal();
    bool inferredBorrow = !declaredType && !decl.isGlobal() && decl.type.isBorrowOrOptionalBorrow();
    if (decl.type.isReferenceType() && !decl.isForLoopElement && !explicitLocalBorrow && !inferredBorrow) {
        decl.initializer = makeAST<ImplicitCastExpr>(decl.initializer, decl.type.getPointee(), ImplicitCastExpr::AutoDereference);
        decl.type = decl.type.getPointee();
    } else if (decl.type.storesBorrow() && !(decl.isForLoopElement && decl.type.isReferenceType()) && !explicitLocalBorrow && !inferredBorrow) {
        ERROR(decl.getLocation(),
              "reference type '" << decl.type << "' may only appear as a function parameter, return type, local variable, or interface argument");
    }

    if (!isArrayBorrow(decl.initializer->type, decl.type)) {
        setMoved(decl.initializer, true, /*trackVars=*/!decl.type.isImplicitlyCopyable());
    }

    if (isConstChecked || decl.isGlobal()) {
        llvm::SmallPtrSet<const VarDecl*, 8> seen;
        if (!isSupportedConstInitializerImpl(*decl.initializer, seen)) {
            if (isConstChecked) {
                ERROR_RANGE(getExprRangeStart(*decl.initializer), decl.initializer->endLocation,
                            "const '" << decl.getName() << "' initializer must be a constant expression");
            } else {
                ERROR_RANGE(getExprRangeStart(*decl.initializer), decl.initializer->endLocation, "global variable initializer must be a constant expression");
            }
        }
    }

    // Array-to-slice conversion needs a data global that codegen does not
    // materialize, so slice-typed globals cannot be initialized from array
    // literals. Null, empty, and inferred (fixed-array) forms work.
    if (decl.isGlobal() && decl.type.containsSlice() && containsNonEmptyArrayLiteral(*decl.initializer)) {
        const char* kind = decl.isConst ? "const" : "global";
        ERROR_RANGE(getExprRangeStart(*decl.initializer), decl.initializer->endLocation,
                    "cannot initialize " << kind << " '" << decl.getName() << "' of type '" << decl.type
                                         << "' with an array literal; omit the type to infer a fixed-size array instead");
    }

    // Locals materialize storage (inferred types have no other mention), so
    // their destructors mark here. Globals never scope-exit-destroy.
    if (!decl.isGlobal()) markDestructorFor(decl.type);
    bindDeinitPtrTarget(decl);
}

void Typechecker::typecheckFieldDecl(FieldDecl& decl) {
    decl.type = resolveTypeAliases(decl.type, std::min(decl.accessLevel, decl.getParentDecl()->accessLevel), /*foldArraySizes=*/true);
    bool allowReference = false;
    auto* parentDecl = llvm::dyn_cast<TypeDecl>(decl.getParentDecl());
    if (parentDecl) allowReference = allowsSubstitutedReference(*parentDecl);
    // Members overlap, so at most one is active: no default values, no implicitly-destroyed owning members.
    if (parentDecl && parentDecl->isUnion()) {
        if (decl.defaultValue) {
            ERROR_RANGE(decl.getLocation(), getIdentifierEndLocation(decl), "union member '" << decl.getName() << "' cannot have a default value");
        }
        if (!decl.isManuallyDestroy && decl.type.needsDestruction()) {
            ERROR_RANGE(decl.getLocation(), getIdentifierEndLocation(decl),
                        "union member '" << decl.getName() << "' needs destruction; mark it '@manuallyDestroy' and destroy it explicitly");
        }
    }
    // Fields are storage, not signature mentions: their types mark destructors
    // even when the declaration is checked during a signature check.
    llvm::SaveAndRestore unguard(checkingFunctionSignature, false);
    typecheckType(decl.type, std::min(decl.accessLevel, decl.getParentDecl()->accessLevel), true, allowReference);
    if (decl.type.isVoid()) {
        ERROR_RANGE(decl.getLocation(), getIdentifierEndLocation(decl), "cannot declare field '" << decl.getName() << "' of type 'void'");
    }

    if (decl.defaultValue) {
        typecheckExpr(*decl.defaultValue, false, decl.type);
        if (Expr* converted = convert(decl.defaultValue, decl.type)) {
            decl.defaultValue = converted;
        } else {
            ERROR_RANGE(getExprRangeStart(*decl.defaultValue), decl.defaultValue->endLocation,
                        "cannot assign '" << decl.defaultValue->type << "' to '" << decl.type << "'" << narrowingHint(decl.defaultValue->type, decl.type));
        }
    }
}

void Typechecker::typecheckImportDecl(ImportDecl& decl) {
    if (decl.target.ends_with(".h") || isCxxHeader(decl.target)) {
        const int errorsBefore = errors;
        if (!importCHeader(*currentSourceFile, decl, *this) && errors == errorsBefore) {
            auto language = isCxxHeader(decl.target) ? "C++" : "C";
            REPORT_ERROR(decl.getLocation(), "couldn't import " << language << " header file '" << decl.target << "'");
        }
        return;
    }
    if (dependencies) {
        auto resolution = resolveDependency(*dependencies, decl.target);
        if (resolution.ambiguous) {
            REPORT_ERROR(decl.getLocation(), resolution.ambiguityDetail);
            return;
        }
    }
    auto module = importModule(currentSourceFile, decl.target);
    if (!module) {
        if (module.getError() == std::make_error_code(std::errc::no_such_file_or_directory)) {
            REPORT_ERROR(decl.getLocation(), "couldn't find module '" << decl.target << "' in the following locations:\n"
                                                                      << llvm::join(options.importSearchPaths, "\n"));
        } else {
            REPORT_ERROR(decl.getLocation(), "couldn't import module '" << decl.target << "': " << module.getError().message());
        }
    }
}

void Typechecker::ensureSignature(Decl& decl) {
    if (suppressEnsureSignature) return;
    // Lookups run in the caller's context, but the declaration must resolve names
    // (and report redefinitions) in its own module and file.
    llvm::SaveAndRestore saveModule(currentModule);
    llvm::SaveAndRestore saveFile(currentSourceFile);
    setDeclContext(decl);
    // Nested declarations report here without aborting the outer check: each error is
    // reported exactly once, at discovery, and the outer declaration still gets its own
    // diagnostics. (Declaration-order exceptions like the size check below handle their
    // own ordering explicitly.)
    try {
        ensureSignatureImpl(decl);
    } catch (const CompileError& error) {
        error.report();
    }
}

void Typechecker::ensureSignatureImpl(Decl& decl) {
    switch (decl.kind) {
    case DeclKind::FunctionDecl:
    case DeclKind::MethodDecl:
    case DeclKind::ConstructorDecl:
    case DeclKind::DestructorDecl: {
        auto& functionDecl = llvm::cast<FunctionDecl>(decl);
        // Interface methods mention the unbound `This` type; only their This-instantiated
        // clones (made at call sites and for conformance checks) are ever checked.
        if (auto* receiver = functionDecl.getTypeDecl(); receiver && receiver->isInterface() && !receiver->instantiatedFrom) break;
        typecheckFunctionSignature(functionDecl);
        break;
    }
    case DeclKind::FunctionTemplate:
        typecheckFunctionTemplate(llvm::cast<FunctionTemplate>(decl));
        break;
    case DeclKind::TypeDecl:
        typecheckTypeSignature(llvm::cast<TypeDecl>(decl));
        break;
    case DeclKind::TypeTemplate:
        typecheckTypeTemplate(llvm::cast<TypeTemplate>(decl));
        break;
    case DeclKind::TypeAliasDecl:
        // Aliases resolve on demand through resolveTypeAliases; checking one here would
        // re-enter (and reorder) the very resolution that triggered the lookup. Main-module
        // aliases are still checked eagerly.
        break;
    case DeclKind::EnumDecl:
        typecheckEnumSignature(llvm::cast<EnumDecl>(decl));
        break;
    case DeclKind::FieldDecl:
        // Fields are resolved with their parent type, which may itself be
        // unensured when the field is found by name (implicit receiver).
        if (auto* typeDecl = llvm::dyn_cast<TypeDecl>(static_cast<VariableDecl&>(decl).parent)) {
            typecheckTypeSignature(*typeDecl);
        }
        break;
    default:
        // Globals are prepass roots, locals/params check inline, and imports
        // resolve through the imports prepass: nothing to ensure on lookup.
        break;
    }
}

void Typechecker::typecheckTopLevelDecl(Decl& decl) {
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
    case DeclKind::TypeAliasDecl:
        typecheckTypeAliasDecl(llvm::cast<TypeAliasDecl>(decl));
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
        typecheckImportDecl(llvm::cast<ImportDecl>(decl));
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
