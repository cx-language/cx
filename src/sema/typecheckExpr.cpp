#include "typecheck.h"
#include <limits>
#include <memory>
#include <string>
#include <vector>
#include <llvm/ADT/APSInt.h>
#include <llvm/ADT/iterator_range.h>
#include <llvm/ADT/Optional.h>
#include <llvm/Support/ErrorHandling.h>
#include "../ast/decl.h"
#include "../ast/expr.h"
#include "../ast/mangle.h"
#include "../ast/module.h"
#include "../ast/type.h"
#include "../support/utility.h"

using namespace delta;

Type TypeChecker::typecheckVarExpr(VarExpr& expr) const {
    Decl& decl = findDecl(expr.getIdentifier(), expr.getLocation());
    expr.setDecl(&decl);

    switch (decl.getKind()) {
        case DeclKind::VarDecl: return llvm::cast<VarDecl>(decl).getType();
        case DeclKind::ParamDecl: return llvm::cast<ParamDecl>(decl).getType();
        case DeclKind::FunctionDecl:
        case DeclKind::MethodDecl: return llvm::cast<FunctionDecl>(decl).getFunctionType();
        case DeclKind::GenericParamDecl: llvm_unreachable("cannot refer to generic parameters yet");
        case DeclKind::InitDecl: llvm_unreachable("cannot refer to initializers yet");
        case DeclKind::DeinitDecl: llvm_unreachable("cannot refer to deinitializers yet");
        case DeclKind::TypeDecl: error(expr.getLocation(), "'", expr.getIdentifier(), "' is not a variable");
        case DeclKind::FieldDecl:
            ASSERT(currentFunction);
            if (currentFunction->isMutating() || currentFunction->isInitDecl()) {
                return llvm::cast<FieldDecl>(decl).getType();
            } else {
                return llvm::cast<FieldDecl>(decl).getType().asImmutable();
            }
        case DeclKind::ImportDecl:
            llvm_unreachable("import statement validation not implemented yet");
    }
    abort();
}

Type typecheckStringLiteralExpr(StringLiteralExpr&) {
    return Type::getString();
}

Type typecheckIntLiteralExpr(IntLiteralExpr& expr) {
    if (expr.getValue() >= std::numeric_limits<int32_t>::min() &&
        expr.getValue() <= std::numeric_limits<int32_t>::max()) {
        return Type::getInt();
    } else if (expr.getValue() >= std::numeric_limits<int64_t>::min() &&
               expr.getValue() <= std::numeric_limits<int64_t>::max()) {
        return Type::getInt64();
    }
    error(expr.getLocation(), "integer literal is too large");
}

Type typecheckFloatLiteralExpr(FloatLiteralExpr&) {
    return Type::getFloat64();
}

Type typecheckBoolLiteralExpr(BoolLiteralExpr&) {
    return Type::getBool();
}

Type typecheckNullLiteralExpr(NullLiteralExpr&) {
    return Type::getNull();
}

Type TypeChecker::typecheckArrayLiteralExpr(ArrayLiteralExpr& array) const {
    Type firstType = typecheckExpr(*array.getElements()[0]);
    for (auto& element : llvm::drop_begin(array.getElements(), 1)) {
        Type type = typecheckExpr(*element);
        if (type != firstType) {
            error(element->getLocation(), "mixed element types in array literal (expected '",
                  firstType, "', found '", type, "')");
        }
    }
    return ArrayType::get(firstType, int64_t(array.getElements().size()));
}

Type TypeChecker::typecheckPrefixExpr(PrefixExpr& expr) const {
    Type operandType = typecheckExpr(expr.getOperand());

    if (expr.getOperator() == NOT) {
        if (!operandType.isBool()) {
            error(expr.getOperand().getLocation(), "invalid operand type '", operandType,
                  "' to logical not");
        }
        return operandType;
    }
    if (expr.getOperator() == STAR) { // Dereference operation
        if (!operandType.isPointerType()) {
            error(expr.getOperand().getLocation(), "cannot dereference non-pointer type '",
                  operandType, "'");
        }
        return operandType.getPointee();
    }
    if (expr.getOperator() == AND) { // Address-of operation
        return PointerType::get(operandType, true);
    }
    return operandType;
}

Type TypeChecker::typecheckBinaryExpr(BinaryExpr& expr) const {
    Type leftType = resolve(typecheckExpr(expr.getLHS()));
    Type rightType = resolve(typecheckExpr(expr.getRHS()));

    if (!expr.isBuiltinOp(*this)) {
        return typecheckCallExpr((CallExpr&) expr);
    }

    if (expr.getOperator() == AND_AND || expr.getOperator() == OR_OR) {
        if (leftType.isBool() && rightType.isBool()) {
            return Type::getBool();
        }
        error(expr.getLocation(), "invalid operands to binary expression ('", leftType, "' and '",
              rightType, "')");
    }

    if (expr.getOperator().isBitwiseOperator() &&
        (leftType.isFloatingPoint() || rightType.isFloatingPoint())) {
        error(expr.getLocation(), "invalid operands to binary expression ('", leftType, "' and '",
              rightType, "')");
    }

    if (!isValidConversion(expr.getLHS(), leftType, rightType) &&
        !isValidConversion(expr.getRHS(), rightType, leftType)) {
        error(expr.getLocation(), "invalid operands to binary expression ('", leftType, "' and '",
              rightType, "')");
    }

    if (expr.getOperator() == DOTDOT) {
        return RangeType::get(leftType, /* hasExclusiveUpperBound */ true);
    }
    if (expr.getOperator() == DOTDOTDOT) {
        return RangeType::get(leftType, /* hasExclusiveUpperBound */ false);
    }

    return expr.getOperator().isComparisonOperator() ? Type::getBool() : leftType;
}

template<int bitWidth, bool isSigned>
bool checkRange(Expr& expr, int64_t value, Type type) {
    if ((isSigned && !llvm::APSInt::get(value).isSignedIntN(bitWidth)) ||
        (!isSigned && !llvm::APSInt::get(value).isIntN(bitWidth))) {
        error(expr.getLocation(), value, " is out of range for type '", type, "'");
    }
    expr.setType(type);
    return true;
}

Type TypeChecker::resolveTypePlaceholder(llvm::StringRef name) const {
    auto it = currentGenericArgs.find(name);
    if (it == currentGenericArgs.end()) return nullptr;
    return it->second;
}

bool TypeChecker::isInterface(Type unresolvedType) const {
    auto type = resolve(unresolvedType);
    return type.isBasicType() && !type.isBuiltinType() && !type.isVoid() &&
           getTypeDecl(llvm::cast<BasicType>(*type))->isInterface();
}

static bool hasField(TypeDecl& type, const FieldDecl& field) {
    return llvm::any_of(type.getFields(), [&](const FieldDecl& ownField) {
        return ownField.getName() == field.getName() && ownField.getType() == field.getType();
    });
}

bool TypeChecker::hasMethod(TypeDecl& type, FunctionDecl& functionDecl) const {
    auto decls = findDecls(mangleFunctionDecl(type.getName(), functionDecl.getName()));
    for (Decl* decl : decls) {
        if (!decl->isFunctionDecl()) continue;
        if (!llvm::cast<FunctionDecl>(decl)->getTypeDecl()) continue;
        if (llvm::cast<FunctionDecl>(decl)->getTypeDecl()->getName() != type.getName()) continue;
        if (!llvm::cast<FunctionDecl>(decl)->signatureMatches(functionDecl, /* matchReceiver: */ false)) continue;
        return true;
    }
    return false;
}

bool TypeChecker::implementsInterface(TypeDecl& type, TypeDecl& interface) const {
    for (auto& fieldRequirement : interface.getFields()) {
        if (!hasField(type, fieldRequirement)) {
            return false;
        }
    }
    for (auto& requiredMethod : interface.getMethods()) {
        if (auto* functionDecl = llvm::dyn_cast<FunctionDecl>(requiredMethod.get())) {
            if (!hasMethod(type, *functionDecl)) {
                return false;
            }
        } else {
            fatalError("non-function interface member requirements are not supported yet");
        }
    }
    return true;
}

bool TypeChecker::isValidConversion(Expr& expr, Type unresolvedSource,
                                    Type unresolvedTarget) const {
    Type source = resolve(unresolvedSource);
    Type target = resolve(unresolvedTarget);

    if (expr.isLvalue() && source.isBasicType()) {
        TypeDecl* typeDecl = getTypeDecl(llvm::cast<BasicType>(*source));
        if (typeDecl && !typeDecl->passByValue() && !target.isPointerType() &&
            typeDecl->getDeinitializer()) {
            error(expr.getLocation(), "move semantics not yet implemented for classes with deinitializers");
        }
    }

    if (source.isImplicitlyConvertibleTo(target)) {
        return true;
    }

    // Check compatibility with interface type.
    if (isInterface(target) && source.isBasicType()) {
        if (implementsInterface(*getTypeDecl(llvm::cast<BasicType>(*source)),
                                *getTypeDecl(llvm::cast<BasicType>(*target)))) {
            return true;
        }
    }

    // Autocast integer literals to parameter type if within range, error out if not within range.
    if (expr.isIntLiteralExpr() && target.isBasicType()) {
        auto value = llvm::cast<IntLiteralExpr>(expr).getValue();

        if (target.isInteger()) {
            if (target.isInt()) return checkRange<32, true>(expr, value, target);
            if (target.isUInt()) return checkRange<32, false>(expr, value, target);
            if (target.isInt8()) return checkRange<8, true>(expr, value, target);
            if (target.isInt16()) return checkRange<16, true>(expr, value, target);
            if (target.isInt32()) return checkRange<32, true>(expr, value, target);
            if (target.isInt64()) return checkRange<64, true>(expr, value, target);
            if (target.isUInt8()) return checkRange<8, false>(expr, value, target);
            if (target.isUInt16()) return checkRange<16, false>(expr, value, target);
            if (target.isUInt32()) return checkRange<32, false>(expr, value, target);
            if (target.isUInt64()) return checkRange<64, false>(expr, value, target);
        }

        if (target.isFloatingPoint()) {
            // TODO: Check that the integer value is losslessly convertible to the target type?
            expr.setType(target);
            return true;
        }
    } else if (expr.isNullLiteralExpr() && target.isNullablePointer()) {
        expr.setType(target);
        return true;
    } else if (expr.isStringLiteralExpr() && target.isPointerType() && target.getPointee().isChar() &&
               !target.getPointee().isMutable()) {
        // Special case: allow passing string literals as C-strings (const char*).
        expr.setType(target);
        return true;
    } else if (expr.isLvalue() && source.isBasicType() && target.isPointerType()) {
        auto typeDecl = getTypeDecl(llvm::cast<BasicType>(*source));
        if (!typeDecl || typeDecl->passByValue()) {
            error(expr.getLocation(),
                  "cannot implicitly pass value types by reference, add explicit '&'");
        }
        if (source.isImplicitlyConvertibleTo(target.getPointee())) {
            return true;
        }
    }

    return false;
}

std::vector<Type> TypeChecker::inferGenericArgs(llvm::ArrayRef<GenericParamDecl> genericParams,
                                                const CallExpr& call,
                                                llvm::ArrayRef<ParamDecl> params) const {
    ASSERT(call.getArgs().size() == params.size());
    return map(genericParams, [&](const GenericParamDecl& genericParam) {
        Type genericArg;

        for (auto tuple : llvm::zip_first(params, call.getArgs())) {
            Type paramType = std::get<0>(tuple).getType();
            if (paramType.isBasicType() && paramType.getName() == genericParam.getName()) {
                // FIXME: The args will also be typechecked by validateArgs()
                // after this function. Get rid of this duplicated typechecking.
                Type argType = typecheckExpr(*std::get<1>(tuple).getValue());

                if (!genericArg) {
                    genericArg = argType;
                } else if (!argType.isImplicitlyConvertibleTo(genericArg)) {
                    error(std::get<1>(tuple).getLocation(), "couldn't infer generic parameter '",
                          genericParam.getName(), "' because of conflicting argument types '",
                          genericArg, "' and '", argType, "'");
                }
            }
        }

        if (genericArg) {
            return genericArg;
        } else {
            error(call.getLocation(), "couldn't infer generic parameter '", genericParam.getName(), "'");
        }
    });
}

static void validateGenericArgCount(size_t genericParamCount, const CallExpr& call) {
    if (call.getGenericArgs().size() < genericParamCount) {
        error(call.getLocation(), "too few generic arguments to '", call.getFunctionName(), "', expected ",
              genericParamCount);
    } else if (call.getGenericArgs().size() > genericParamCount) {
        error(call.getLocation(), "too many generic arguments to '", call.getFunctionName(), "', expected ",
              genericParamCount);
    }
}

void TypeChecker::setCurrentGenericArgs(llvm::ArrayRef<GenericParamDecl> genericParams,
                                        CallExpr& call, llvm::ArrayRef<ParamDecl> params) const {
    if (genericParams.empty()) return;

    if (call.getGenericArgs().empty()) {
        call.setGenericArgs(inferGenericArgs(genericParams, call, params));
        ASSERT(call.getGenericArgs().size() == genericParams.size());
    } else {
        validateGenericArgCount(genericParams.size(), call);
    }

    auto genericArg = call.getGenericArgs().begin();
    for (const GenericParamDecl& genericParam : genericParams) {
        if (!genericParam.getConstraints().empty()) {
            ASSERT(genericParam.getConstraints().size() == 1, "cannot have multiple generic constraints yet");

            auto interfaces = findDecls(genericParam.getConstraints()[0]);
            ASSERT(interfaces.size() == 1);

            if (genericArg->isBasicType() &&
                !implementsInterface(*getTypeDecl(llvm::cast<BasicType>(**genericArg)),
                                     llvm::cast<TypeDecl>(*interfaces[0]))) {
                error(call.getLocation(), "type '", *genericArg, "' doesn't implement interface '",
                      genericParam.getConstraints()[0], "'");
            }
        }
        currentGenericArgs.insert({genericParam.getName(), *genericArg++});
    }
}

void TypeChecker::setCurrentGenericArgsForGenericFunction(FunctionLikeDecl& functionDecl,
                                                          CallExpr& callExpr) const {
    auto* typeDecl = functionDecl.getTypeDecl();

    if (typeDecl && typeDecl->isGeneric()) {
        ASSERT(typeDecl->getGenericParams().size() ==
               callExpr.getReceiverType().removePointer().getGenericArgs().size());
        for (auto t : llvm::zip_first(typeDecl->getGenericParams(),
                                      callExpr.getReceiverType().removePointer().getGenericArgs())) {
            currentGenericArgs.emplace(std::get<0>(t).getName(), std::get<1>(t));
        }
    }

    setCurrentGenericArgs(functionDecl.getGenericParams(), callExpr, functionDecl.getParams());
}

Type TypeChecker::typecheckBuiltinConversion(CallExpr& expr) const {
    if (expr.getArgs().size() != 1) {
        error(expr.getLocation(), "expected single argument to converting initializer");
    }
    if (!expr.getGenericArgs().empty()) {
        error(expr.getLocation(), "expected no generic arguments to converting initializer");
    }
    if (!expr.getArgs().front().getName().empty()) {
        error(expr.getLocation(), "expected unnamed argument to converting initializer");
    }
    typecheckExpr(*expr.getArgs().front().getValue());
    expr.setType(BasicType::get(expr.getFunctionName(), {}));
    return expr.getType();
}

Decl& TypeChecker::resolveOverload(CallExpr& expr, llvm::StringRef callee) const {
    llvm::SmallVector<Decl*, 1> matches;
    bool isInitCall = false;
    bool atLeastOneFunction = false;
    auto decls = findDecls(callee, typecheckingGenericFunction);

    for (Decl* decl : decls) {
        switch (decl->getKind()) {
            case DeclKind::FunctionDecl: case DeclKind::MethodDecl: {
                auto& functionDecl = llvm::cast<FunctionDecl>(*decl);
                SAVE_STATE(currentGenericArgs);

                if (expr.isMethodCall()) {
                    Type receiverType = expr.getReceiver()->getType().removePointer();
                    if (receiverType.isBasicType() && !receiverType.getGenericArgs().empty()) {
                        TypeDecl* typeDecl = getTypeDecl(llvm::cast<BasicType>(*receiverType));
                        ASSERT(typeDecl->getGenericParams().size() == receiverType.getGenericArgs().size());
                        for (auto t : llvm::zip_first(typeDecl->getGenericParams(), receiverType.getGenericArgs())) {
                            currentGenericArgs.emplace(std::get<0>(t).getName(), std::get<1>(t));
                        }
                    }
                }

                setCurrentGenericArgs(functionDecl.getGenericParams(), expr, functionDecl.getParams());

                if (decls.size() == 1) {
                    validateArgs(expr.getArgs(), functionDecl.getParams(), functionDecl.isVariadic(),
                                 callee, expr.getCallee().getLocation());
                    return *decl;
                }
                if (validateArgs(expr.getArgs(), functionDecl.getParams(), functionDecl.isVariadic())) {
                    matches.push_back(decl);
                }
                break;
            }
            case DeclKind::TypeDecl: {
                isInitCall = true;
                auto mangledName = mangleInitDecl(llvm::cast<TypeDecl>(decl)->getName());
                auto initDecls = findDecls(mangledName);

                for (Decl* decl : initDecls) {
                    InitDecl& initDecl = llvm::cast<InitDecl>(*decl);

                    if (initDecls.size() == 1) {
                        validateArgs(expr.getArgs(), initDecl.getParams(), false, callee,
                                     expr.getCallee().getLocation());
                        return initDecl;
                    }
                    if (validateArgs(expr.getArgs(), initDecl.getParams(), false)) {
                        matches.push_back(&initDecl);
                    }
                }
                break;
            }
            default: continue;
        }
        atLeastOneFunction = true;
    }

    switch (matches.size()) {
        case 1: return *matches.front();
        case 0:
            if (decls.size() == 0) {
                error(expr.getCallee().getLocation(), "unknown identifier '", callee, "'");
            } else if (atLeastOneFunction) {
                error(expr.getCallee().getLocation(), "no matching ",
                      isInitCall ? "initializer for '" : "function for call to '", callee, "'");
            } else {
                error(expr.getCallee().getLocation(), "'", callee, "' is not a function");
            }
        default:
            bool allMatchesAreFromC = llvm::all_of(matches, [](Decl* match) {
                return match->getModule() && match->getModule()->getName().endswith_lower(".h");
            });
            if (allMatchesAreFromC) {
                return *matches.front();
            }

            for (Decl* match : matches) {
                if (match->getModule() && match->getModule()->getName() == "std") {
                    return *match;
                }
            }
            error(expr.getCallee().getLocation(), "ambiguous reference to '", callee,
                  isInitCall ? ".init'" : "'");
    }
}

Type TypeChecker::typecheckCallExpr(CallExpr& expr) const {
    if (!expr.callsNamedFunction()) {
        fatalError("anonymous function calls not implemented yet");
    }

    if (Type::isBuiltinScalar(expr.getFunctionName())) {
        return typecheckBuiltinConversion(expr);
    }

    if (expr.getFunctionName() == "sizeOf") {
        validateArgs(expr.getArgs(), {}, false, expr.getFunctionName(), expr.getLocation());
        validateGenericArgCount(1, expr);
        expr.setType(Type::getUInt64());
        return expr.getType();
    }

    Decl* decl;

    if (expr.getCallee().isMemberExpr()) {
        Type receiverType = typecheckExpr(*expr.getReceiver());
        expr.setReceiverType(receiverType);

        if (receiverType.isPointerType() && expr.getFunctionName() == "offsetUnsafely") {
            validateArgs(expr.getArgs(), {ParamDecl(Type::getInt64(), "pointer", SourceLocation::invalid())},
                         false, expr.getFunctionName(), expr.getLocation());
            validateGenericArgCount(0, expr);
            expr.setType(receiverType);
            return expr.getType();
        }

        decl = &resolveOverload(expr, expr.getMangledFunctionName(*this));

        if (receiverType.isNullablePointer()) {
            error(expr.getReceiver()->getLocation(), "cannot call member function through pointer '",
                  receiverType, "', pointer may be null");
        }
    } else {
        decl = &resolveOverload(expr, expr.getFunctionName());

        if (decl->isMethodDecl() && !decl->isInitDecl()) {
            auto& varDecl = llvm::cast<VarDecl>(findDecl("this", expr.getCallee().getLocation()));
            expr.setReceiverType(varDecl.getType());
        }
    }

    expr.setCalleeDecl(decl);

    if (auto* functionDecl = llvm::dyn_cast<FunctionDecl>(decl)) {
        auto* receiverTypeDecl = functionDecl->getTypeDecl();
        bool hasGenericReceiverType = receiverTypeDecl && receiverTypeDecl->isGeneric();

        if (!functionDecl->isGeneric() && !hasGenericReceiverType) {
            return functionDecl->getFunctionType()->getReturnType();
        } else {
            setCurrentGenericArgsForGenericFunction(*functionDecl, expr);
            // TODO: Don't typecheck more than once with the same generic arguments.
            genericFunctionInstantiationsToTypecheck.emplace_back(*functionDecl, expr);
            Type returnType = resolve(functionDecl->getFunctionType()->getReturnType());
            currentGenericArgs.clear();
            return returnType;
        }
    } else if (auto* initDecl = llvm::dyn_cast<InitDecl>(decl)) {
        if (initDecl->getTypeDecl()->isGeneric()) {
            SAVE_STATE(currentGenericArgs);
            setCurrentGenericArgs(initDecl->getTypeDecl()->getGenericParams(), expr,
                                  initDecl->getParams());
            // TODO: Don't typecheck more than once with the same generic arguments.
            SAVE_STATE(typecheckingGenericFunction);
            typecheckingGenericFunction = true;
            typecheckInitDecl(*initDecl);
            if (auto* deinitDecl = initDecl->getTypeDecl()->getDeinitializer()) {
                typecheckDeinitDecl(*deinitDecl);
            }
        }
        return initDecl->getTypeDecl()->getType(expr.getGenericArgs());
    }
    llvm_unreachable("all cases handled");
}

bool TypeChecker::validateArgs(llvm::ArrayRef<Argument> args, llvm::ArrayRef<ParamDecl> params,
                               bool isVariadic, llvm::StringRef functionName,
                               SourceLocation location) const {
    bool returnOnError = functionName.empty();

    if (args.size() < params.size()) {
        if (returnOnError) return false;
        error(location, "too few arguments to '", functionName, "', expected ",
              isVariadic ? "at least " : "", params.size());
    }
    if (!isVariadic && args.size() > params.size()) {
        if (returnOnError) return false;
        error(location, "too many arguments to '", functionName, "', expected ", params.size());
    }

    for (size_t i = 0; i < args.size(); ++i) {
        const Argument& arg = args[i];
        const ParamDecl* param = i < params.size() ? &params[i] : nullptr;

        if (!arg.getName().empty() && (!param || arg.getName() != param->getName())) {
            if (returnOnError) return false;
            error(arg.getLocation(), "invalid argument name '", arg.getName(), "' for parameter '", param->getName(), "'");
        }
        auto argType = typecheckExpr(*arg.getValue());
        if (param && !isValidConversion(*arg.getValue(), argType, param->getType())) {
            if (returnOnError) return false;
            error(arg.getLocation(), "invalid argument #", i + 1, " type '", argType, "' to '", functionName,
                  "', expected '", param->getType(), "'");
        }
    }

    return true;
}

Type TypeChecker::typecheckCastExpr(CastExpr& expr) const {
    Type sourceType = typecheckExpr(expr.getExpr());
    Type targetType = expr.getTargetType();

    switch (sourceType.getKind()) {
        case TypeKind::BasicType:
            if (sourceType.isBool() && targetType.isInt()) {
                return targetType; // bool -> int
            }
        case TypeKind::ArrayType:
        case TypeKind::RangeType:
        case TypeKind::TupleType:
        case TypeKind::FunctionType:
            break;
        case TypeKind::PointerType:
            Type sourcePointee = sourceType.getPointee();
            if (targetType.isPointerType()) {
                Type targetPointee = targetType.getPointee();
                if (sourcePointee.isVoid() &&
                    (!targetPointee.isMutable() || sourcePointee.isMutable())) {
                    return targetType; // (mutable) void* -> T* / mutable void* -> mutable T*
                }
                if (targetPointee.isVoid() &&
                    (!targetPointee.isMutable() || sourcePointee.isMutable())) {
                    return targetType; // (mutable) T* -> void* / mutable T* -> mutable void*
                }
            }
            break;
    }
    error(expr.getLocation(), "illegal cast from '", sourceType, "' to '", targetType, "'");
}

Type TypeChecker::typecheckMemberExpr(MemberExpr& expr) const {
    Type baseType = resolve(typecheckExpr(*expr.getBaseExpr()));

    if (baseType.isPointerType()) {
        if (!baseType.isReference()) {
            error(expr.getBaseExpr()->location, "cannot access member through pointer '", baseType,
                  "', pointer may be null");
        }
        baseType = baseType.getPointee();
    }

    if (baseType.isArrayType() || baseType.isString()) {
        if (expr.getMemberName() == "data") return PointerType::get(Type::getChar(), true);
        if (expr.getMemberName() == "count") return Type::getInt();
        error(expr.getLocation(), "no member named '", expr.getMemberName(), "' in '", baseType, "'");
    }

    Decl& typeDecl = findDecl(baseType.getName(), SourceLocation::invalid());

    for (auto& field : llvm::cast<TypeDecl>(typeDecl).getFields()) {
        if (field.getName() == expr.getMemberName()) {
            if (baseType.isMutable()) {
                return field.getType();
            } else {
                return field.getType().asImmutable();
            }
        }
    }
    error(expr.getLocation(), "no member named '", expr.getMemberName(), "' in '", baseType, "'");
}

Type TypeChecker::typecheckSubscriptExpr(SubscriptExpr& expr) const {
    Type lhsType = typecheckExpr(*expr.getBaseExpr());
    const ArrayType* arrayType;

    if (lhsType.isArrayType()) {
        arrayType = &llvm::cast<ArrayType>(*lhsType);
    } else if (lhsType.isReference() && lhsType.getReferee().isArrayType()) {
        arrayType = &llvm::cast<ArrayType>(*lhsType.getReferee());
    } else {
        error(expr.getBaseExpr()->getLocation(), "cannot subscript '", lhsType,
              "', expected array or reference-to-array");
    }

    Type indexType = typecheckExpr(*expr.getIndexExpr());
    if (!isValidConversion(*expr.getIndexExpr(), indexType, Type::getInt())) {
        error(expr.getIndexExpr()->getLocation(), "illegal subscript index type '", indexType,
              "', expected 'int'");
    }

    if (!arrayType->isUnsized()) {
        if (auto* intLiteralExpr = llvm::dyn_cast<IntLiteralExpr>(expr.getIndexExpr())) {
            if (intLiteralExpr->getValue() >= arrayType->getSize()) {
                error(intLiteralExpr->getLocation(), "accessing array out-of-bounds with index ",
                      intLiteralExpr->getValue(), ", array size is ", arrayType->getSize());
            }
        }
    }

    return arrayType->getElementType();
}

Type TypeChecker::typecheckUnwrapExpr(UnwrapExpr& expr) const {
    Type type = typecheckExpr(expr.getOperand());
    if (!type.isNullablePointer()) {
        error(expr.getLocation(), "cannot unwrap non-pointer type '", type, "'");
    }
    return PointerType::get(type.getPointee(), true);
}

Type TypeChecker::typecheckExpr(Expr& expr) const {
    llvm::Optional<Type> type;
    switch (expr.getKind()) {
        case ExprKind::VarExpr: type = typecheckVarExpr(llvm::cast<VarExpr>(expr)); break;
        case ExprKind::StringLiteralExpr: type = typecheckStringLiteralExpr(llvm::cast<StringLiteralExpr>(expr)); break;
        case ExprKind::IntLiteralExpr: type = typecheckIntLiteralExpr(llvm::cast<IntLiteralExpr>(expr)); break;
        case ExprKind::FloatLiteralExpr: type = typecheckFloatLiteralExpr(llvm::cast<FloatLiteralExpr>(expr)); break;
        case ExprKind::BoolLiteralExpr: type = typecheckBoolLiteralExpr(llvm::cast<BoolLiteralExpr>(expr)); break;
        case ExprKind::NullLiteralExpr: type = typecheckNullLiteralExpr(llvm::cast<NullLiteralExpr>(expr)); break;
        case ExprKind::ArrayLiteralExpr: type = typecheckArrayLiteralExpr(llvm::cast<ArrayLiteralExpr>(expr)); break;
        case ExprKind::PrefixExpr: type = typecheckPrefixExpr(llvm::cast<PrefixExpr>(expr)); break;
        case ExprKind::BinaryExpr: type = typecheckBinaryExpr(llvm::cast<BinaryExpr>(expr)); break;
        case ExprKind::CallExpr: type = typecheckCallExpr(llvm::cast<CallExpr>(expr)); break;
        case ExprKind::CastExpr: type = typecheckCastExpr(llvm::cast<CastExpr>(expr)); break;
        case ExprKind::MemberExpr: type = typecheckMemberExpr(llvm::cast<MemberExpr>(expr)); break;
        case ExprKind::SubscriptExpr: type = typecheckSubscriptExpr(llvm::cast<SubscriptExpr>(expr)); break;
        case ExprKind::UnwrapExpr: type = typecheckUnwrapExpr(llvm::cast<UnwrapExpr>(expr)); break;
    }
    ASSERT(*type);
    expr.setType(*type);
    return expr.getType();
}

bool TypeChecker::isValidConversion(llvm::ArrayRef<std::unique_ptr<Expr>> exprs, Type source,
                                    Type target) const {
    if (!source.isTupleType()) {
        ASSERT(!target.isTupleType());
        ASSERT(exprs.size() == 1);
        return isValidConversion(*exprs[0], source, target);
    }
    ASSERT(target.isTupleType());

    for (size_t i = 0; i < exprs.size(); ++i) {
        if (!isValidConversion(*exprs[i], source.getSubtypes()[i], target.getSubtypes()[i])) {
            return false;
        }
    }
    return true;
}
