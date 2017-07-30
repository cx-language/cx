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
    Decl& decl = findDecl(expr.identifier, expr.getLocation());
    expr.setDecl(&decl);

    switch (decl.getKind()) {
        case DeclKind::VarDecl: return decl.getVarDecl().getType();
        case DeclKind::ParamDecl: return decl.getParamDecl().type;
        case DeclKind::FunctionDecl: return decl.getFunctionDecl().getFunctionType();
        case DeclKind::GenericParamDecl: llvm_unreachable("cannot refer to generic parameters yet");
        case DeclKind::InitDecl: llvm_unreachable("cannot refer to initializers yet");
        case DeclKind::DeinitDecl: llvm_unreachable("cannot refer to deinitializers yet");
        case DeclKind::TypeDecl: error(expr.getLocation(), "'", expr.identifier, "' is not a variable");
        case DeclKind::FieldDecl:
            assert(currentFunction);
            if ((currentFunction->isFunctionDecl() && currentFunction->getFunctionDecl().isMutating()) ||
                currentFunction->isInitDecl()) {
                return decl.getFieldDecl().type;
            } else {
                return decl.getFieldDecl().type.asImmutable();
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
    if (expr.value >= std::numeric_limits<int32_t>::min() &&
        expr.value <= std::numeric_limits<int32_t>::max()) {
        return Type::getInt();
    } else if (expr.value >= std::numeric_limits<int64_t>::min() &&
               expr.value <= std::numeric_limits<int64_t>::max()) {
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
    Type firstType = typecheckExpr(*array.elements[0]);
    for (auto& element : llvm::drop_begin(array.elements, 1)) {
        Type type = typecheckExpr(*element);
        if (type != firstType) {
            error(element->getLocation(), "mixed element types in array literal (expected '",
                  firstType, "', found '", type, "')");
        }
    }
    return ArrayType::get(firstType, int64_t(array.elements.size()));
}

Type TypeChecker::typecheckPrefixExpr(PrefixExpr& expr) const {
    Type operandType = typecheckExpr(expr.getOperand());

    if (expr.op == NOT) {
        if (!operandType.isBool()) {
            error(expr.getOperand().getLocation(), "invalid operand type '", operandType,
                  "' to logical not");
        }
        return operandType;
    }
    if (expr.op == STAR) { // Dereference operation
        if (!operandType.isPointerType()) {
            error(expr.getOperand().getLocation(), "cannot dereference non-pointer type '",
                  operandType, "'");
        }
        return operandType.getPointee();
    }
    if (expr.op == AND) { // Address-of operation
        return PointerType::get(operandType, true);
    }
    return operandType;
}

Type TypeChecker::typecheckBinaryExpr(BinaryExpr& expr) const {
    Type leftType = typecheckExpr(expr.getLHS());
    Type rightType = typecheckExpr(expr.getRHS());

    if (!expr.isBuiltinOp()) {
        return typecheckCallExpr((CallExpr&) expr);
    }

    if (expr.op == AND_AND || expr.op == OR_OR) {
        if (leftType.isBool() && rightType.isBool()) {
            return Type::getBool();
        }
        error(expr.getLocation(), "invalid operands to binary expression ('", leftType, "' and '",
              rightType, "')");
    }

    if (expr.op.isBitwiseOperator() &&
        (leftType.isFloatingPoint() || rightType.isFloatingPoint())) {
        error(expr.getLocation(), "invalid operands to binary expression ('", leftType, "' and '",
              rightType, "')");
    }

    if (!isValidConversion(expr.getLHS(), leftType, rightType) &&
        !isValidConversion(expr.getRHS(), rightType, leftType)) {
        error(expr.getLocation(), "invalid operands to binary expression ('", leftType, "' and '",
              rightType, "')");
    }

    if (expr.op == DOTDOT) {
        return RangeType::get(leftType, /* hasExclusiveUpperBound */ true);
    }
    if (expr.op == DOTDOTDOT) {
        return RangeType::get(leftType, /* hasExclusiveUpperBound */ false);
    }

    return expr.op.isComparisonOperator() ? Type::getBool() : leftType;
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

/// Resolves generic parameters to their corresponding types, returns other types as is.
Type TypeChecker::resolve(Type type) const {
    switch (type.getKind()) {
        case TypeKind::BasicType: {
            auto it = currentGenericArgs.find(type.getName());
            if (it == currentGenericArgs.end()) return type;
            return it->second;
        }
        case TypeKind::PointerType:
            return PointerType::get(resolve(type.getPointee()), type.isReference(), type.isMutable());
        case TypeKind::ArrayType:
            return ArrayType::get(resolve(type.getElementType()), type.getArraySize(),
                                  type.isMutable());
        case TypeKind::RangeType:
            return RangeType::get(resolve(type.getIterableElementType()),
                                  llvm::cast<RangeType>(*type).isExclusive(), type.isMutable());
        case TypeKind::FunctionType: {
            std::vector<Type> resolvedParamTypes;
            resolvedParamTypes.reserve(type.getParamTypes().size());
            for (Type type : type.getParamTypes()) {
                resolvedParamTypes.emplace_back(resolve(type));
            }
            return FunctionType::get(resolve(type.getReturnType()), std::move(resolvedParamTypes),
                                 type.isMutable());
        }
        default:
            fatalError(("resolve() not implemented for type '" + type.toString() + "'").c_str());
    }
}

bool TypeChecker::isInterface(Type unresolvedType) const {
    auto type = resolve(unresolvedType);
    return type.isBasicType() && !type.isBuiltinType() && !type.isVoid() &&
           getTypeDecl(llvm::cast<BasicType>(*type))->isInterface();
}

bool hasField(TypeDecl& type, FieldDecl& field) {
    return llvm::any_of(type.fields, [&](FieldDecl& ownField) {
        return ownField.name == field.name && ownField.type == field.type;
    });
}

bool TypeChecker::hasMethod(TypeDecl& type, FunctionDecl& functionDecl) const {
    auto decls = findDecls(mangleFunctionDecl(type.name, functionDecl.name));
    for (Decl* decl : decls) {
        if (!decl->isFunctionDecl()) continue;
        if (!decl->getFunctionDecl().getReceiverTypeDecl()) continue;
        if (decl->getFunctionDecl().getReceiverTypeDecl()->name != type.name) continue;
        if (!decl->getFunctionDecl().signatureMatches(functionDecl, /* matchReceiver: */ false)) continue;
        return true;
    }
    return false;
}

bool TypeChecker::implementsInterface(TypeDecl& type, TypeDecl& interface) const {
    for (auto& fieldRequirement : interface.fields) {
        if (!hasField(type, fieldRequirement)) {
            return false;
        }
    }
    for (auto& requiredMethod : interface.methods) {
        if (!requiredMethod->isFunctionDecl()) {
            fatalError("non-function interface member requirements are not supported yet");
        }
        if (!hasMethod(type, requiredMethod->getFunctionDecl())) {
            return false;
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
        if (typeDecl && !typeDecl->passByValue() && !target.isPointerType()) {
            error(expr.getLocation(), "implicit copying of class instances is disallowed");
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
        int64_t value{expr.getIntLiteralExpr().value};

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
    std::vector<Type> genericArgs;
    genericArgs.reserve(genericParams.size());

    assert(call.args.size() == params.size());

    for (auto& genericParam : genericParams) {
        Type genericArg;

        for (auto tuple : llvm::zip_first(params, call.args)) {
            Type paramType = std::get<0>(tuple).getType();
            if (paramType.isBasicType() && paramType.getName() == genericParam.name) {
                // FIXME: The args will also be typechecked by validateArgs()
                // after this function. Get rid of this duplicated typechecking.
                Type argType = typecheckExpr(*std::get<1>(tuple).getValue());

                if (!genericArg) {
                    genericArg = argType;
                } else if (!argType.isImplicitlyConvertibleTo(genericArg)) {
                    error(std::get<1>(tuple).getLocation(), "couldn't infer generic parameter '",
                          genericParam.name, "' because of conflicting argument types '",
                          genericArg, "' and '", argType, "'");
                }
            }
        }

        if (genericArg) {
            genericArgs.emplace_back(genericArg);
        } else {
            error(call.getLocation(), "couldn't infer generic parameter '", genericParam.name, "'");
        }
    }

    return genericArgs;
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
        assert(call.getGenericArgs().size() == genericParams.size());
    } else {
        validateGenericArgCount(genericParams.size(), call);
    }

    auto genericArg = call.getGenericArgs().begin();
    for (const GenericParamDecl& genericParam : genericParams) {
        if (!genericParam.constraints.empty()) {
            assert(genericParam.constraints.size() == 1 &&
                   "cannot have multiple generic constraints yet");

            auto interfaces = findDecls(genericParam.constraints[0]);
            assert(interfaces.size() == 1);

            if (genericArg->isBasicType() &&
                !implementsInterface(*getTypeDecl(llvm::cast<BasicType>(**genericArg)),
                                     interfaces[0]->getTypeDecl())) {
                error(call.getLocation(), "type '", *genericArg, "' doesn't implement interface '",
                      genericParam.constraints[0], "'");
            }
        }
        currentGenericArgs.insert({genericParam.name, *genericArg++});
    }
}

Type TypeChecker::typecheckBuiltinConversion(CallExpr& expr) const {
    if (expr.args.size() != 1) {
        error(expr.getLocation(), "expected single argument to converting initializer");
    }
    if (!expr.getGenericArgs().empty()) {
        error(expr.getLocation(), "expected no generic arguments to converting initializer");
    }
    if (!expr.args.front().getName().empty()) {
        error(expr.getLocation(), "expected unnamed argument to converting initializer");
    }
    typecheckExpr(*expr.args.front().getValue());
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
            case DeclKind::FunctionDecl: {
                auto previousGenericArgs = std::move(currentGenericArgs);

                if (expr.isMethodCall()) {
                    Type receiverType = expr.getReceiver()->getType().removePointer();
                    if (receiverType.isBasicType() && !receiverType.getGenericArgs().empty()) {
                        TypeDecl* typeDecl = getTypeDecl(llvm::cast<BasicType>(*receiverType));
                        assert(typeDecl->genericParams.size() == receiverType.getGenericArgs().size());
                        for (auto t : llvm::zip_first(typeDecl->genericParams, receiverType.getGenericArgs())) {
                            currentGenericArgs.emplace(std::get<0>(t).name, std::get<1>(t));
                        }
                    }
                }

                setCurrentGenericArgs(decl->getFunctionDecl().getGenericParams(), expr,
                                      decl->getFunctionDecl().getParams());

                if (decls.size() == 1) {
                    validateArgs(expr.args, decl->getFunctionDecl().params, callee,
                                 decl->getFunctionDecl().isVariadic(), expr.getCallee().getLocation());
                    return *decl;
                }
                if (matchArgs(expr.args, decl->getFunctionDecl().params, decl->getFunctionDecl().isVariadic())) {
                    matches.push_back(decl);
                }

                currentGenericArgs = std::move(previousGenericArgs);
                break;
            }
            case DeclKind::TypeDecl: {
                isInitCall = true;
                auto mangledName = mangleInitDecl(decl->getTypeDecl().name);
                auto initDecls = findDecls(mangledName);

                for (Decl* initDecl : initDecls) {
                    if (initDecls.size() == 1) {
                        validateArgs(expr.args, initDecl->getInitDecl().params, callee, false,
                                     expr.getCallee().getLocation());
                        return *initDecl;
                    }
                    if (matchArgs(expr.args, initDecl->getInitDecl().params, false)) {
                        matches.push_back(initDecl);
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
        validateArgs(expr.args, {}, expr.getFunctionName(), false, expr.getLocation());
        validateGenericArgCount(1, expr);
        expr.setType(Type::getUInt64());
        return expr.getType();
    }

    Decl* decl;

    if (expr.getCallee().isMemberExpr()) {
        Type receiverType = typecheckExpr(*expr.getReceiver());
        expr.setReceiverType(receiverType);

        if (receiverType.isPointerType() && expr.getFunctionName() == "offsetUnsafely") {
            validateArgs(expr.args, {ParamDecl(Type::getInt64(), "pointer", SourceLocation::invalid())},
                         expr.getFunctionName(), false, expr.getLocation());
            validateGenericArgCount(0, expr);
            expr.setType(receiverType);
            return expr.getType();
        }

        decl = &resolveOverload(expr, expr.getMangledFunctionName());

        if (receiverType.isNullablePointer()) {
            error(expr.getReceiver()->getLocation(), "cannot call member function through pointer '",
                  receiverType, "', pointer may be null");
        }
    } else {
        decl = &resolveOverload(expr, expr.getFunctionName());

        if (decl->isFunctionDecl() && decl->getFunctionDecl().isMethod()) {
            expr.setReceiverType(findDecl("this", expr.getCallee().getLocation()).getVarDecl().getType());
        }
    }

    expr.setCalleeDecl(decl);

    if (auto* functionDecl = llvm::dyn_cast<FunctionDecl>(decl)) {
        auto* receiverTypeDecl = functionDecl->getReceiverTypeDecl();
        bool hasGenericReceiverType = receiverTypeDecl && receiverTypeDecl->isGeneric();

        if (!functionDecl->isGeneric() && !hasGenericReceiverType) {
            return functionDecl->getFunctionType()->returnType;
        } else {
            setCurrentGenericArgs(functionDecl->getGenericParams(), expr, functionDecl->getParams());
            if (hasGenericReceiverType) {
                assert(receiverTypeDecl->genericParams.size() ==
                       expr.getReceiverType().removePointer().getGenericArgs().size());
                for (auto t : llvm::zip_first(receiverTypeDecl->genericParams,
                                              expr.getReceiverType().removePointer().getGenericArgs())) {
                    currentGenericArgs.emplace(std::get<0>(t).name, std::get<1>(t));
                }
            }

            // TODO: Don't typecheck more than once with the same generic arguments.
            auto wasTypecheckingGenericFunction = typecheckingGenericFunction;
            typecheckingGenericFunction = true;
            typecheckFunctionDecl(*functionDecl);
            typecheckingGenericFunction = wasTypecheckingGenericFunction;
            Type returnType = resolve(functionDecl->getFunctionType()->returnType);
            currentGenericArgs.clear();
            return returnType;
        }
    } else if (auto* initDecl = llvm::dyn_cast<InitDecl>(decl)) {
        if (initDecl->getTypeDecl().isGeneric()) {
            auto previousGenericArgs = std::move(currentGenericArgs);
            setCurrentGenericArgs(initDecl->getTypeDecl().genericParams, expr,
                                  initDecl->getParams());
            // TODO: Don't typecheck more than once with the same generic arguments.
            typecheckInitDecl(*initDecl);
            currentGenericArgs = std::move(previousGenericArgs);
        }
        return initDecl->getTypeDecl().getType(expr.getGenericArgs());
    }
    llvm_unreachable("all cases handled");
}

bool TypeChecker::matchArgs(llvm::ArrayRef<Argument> args, llvm::ArrayRef<ParamDecl> params,
                            bool isVariadic) const {
    if (isVariadic) {
        if (args.size() < params.size()) return false;
    } else {
        if (args.size() != params.size()) return false;
    }

    for (size_t i = 0; i < args.size(); ++i) {
        const Argument& arg = args[i];
        const ParamDecl* param = i < params.size() ? &params[i] : nullptr;

        if (!arg.name.empty() && (!param || arg.name != param->name)) return false;
        auto argType = typecheckExpr(*arg.value);
        if (param && !isValidConversion(*arg.value, argType, param->type)) return false;
    }
    return true;
}

void TypeChecker::validateArgs(const std::vector<Argument>& args, const std::vector<ParamDecl>& params,
                               const std::string& functionName, bool isVariadic, SourceLocation location) const {
    if (args.size() < params.size()) {
        error(location, "too few arguments to '", functionName, "', expected ",
              isVariadic ? "at least " : "", params.size());
    }
    if (!isVariadic && args.size() > params.size()) {
        error(location, "too many arguments to '", functionName, "', expected ", params.size());
    }

    for (size_t i = 0; i < args.size(); ++i) {
        const Argument& arg = args[i];
        const ParamDecl* param = i < params.size() ? &params[i] : nullptr;

        if (!arg.name.empty() && (!param || arg.name != param->name)) {
            error(arg.getLocation(), "invalid argument name '", arg.name, "' for parameter '", param->name, "'");
        }
        auto argType = typecheckExpr(*arg.value);
        if (param && !isValidConversion(*arg.value, argType, param->type)) {
            error(arg.getLocation(), "invalid argument #", i + 1, " type '", argType, "' to '", functionName,
                  "', expected '", param->type, "'");
        }
    }
}

Type TypeChecker::typecheckCastExpr(CastExpr& expr) const {
    Type sourceType = typecheckExpr(*expr.expr);
    Type targetType = expr.type;

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
    Type baseType = typecheckExpr(*expr.base);

    if (baseType.isPointerType()) {
        if (!baseType.isReference()) {
            error(expr.base->location, "cannot access member through pointer '", baseType,
                  "', pointer may be null");
        }
        baseType = baseType.getPointee();
    }

    if (baseType.isArrayType() || baseType.isString()) {
        if (expr.member == "data") return PointerType::get(Type::getChar(), true);
        if (expr.member == "count") return Type::getInt();
        error(expr.getLocation(), "no member named '", expr.member, "' in '", baseType, "'");
    }

    Decl& typeDecl = findDecl(baseType.getName(), SourceLocation::invalid());

    for (const FieldDecl& field : typeDecl.getTypeDecl().fields) {
        if (field.name == expr.member) {
            if (baseType.isMutable()) {
                return field.type;
            } else {
                return field.type.asImmutable();
            }
        }
    }
    error(expr.getLocation(), "no member named '", expr.member, "' in '", baseType, "'");
}

Type TypeChecker::typecheckSubscriptExpr(SubscriptExpr& expr) const {
    Type lhsType = typecheckExpr(*expr.array);
    const ArrayType* arrayType;

    if (lhsType.isArrayType()) {
        arrayType = &llvm::cast<ArrayType>(*lhsType);
    } else if (lhsType.isReference() && lhsType.getReferee().isArrayType()) {
        arrayType = &llvm::cast<ArrayType>(*lhsType.getReferee());
    } else {
        error(expr.array->getLocation(), "cannot subscript '", lhsType,
              "', expected array or reference-to-array");
    }

    Type indexType = typecheckExpr(*expr.index);
    if (!isValidConversion(*expr.index, indexType, Type::getInt())) {
        error(expr.index->getLocation(), "illegal subscript index type '", indexType,
              "', expected 'int'");
    }

    if (!arrayType->isUnsized() && expr.index->isIntLiteralExpr() &&
        expr.index->getIntLiteralExpr().value >= arrayType->size) {
        error(expr.index->getLocation(), "accessing array out-of-bounds with index ",
              expr.index->getIntLiteralExpr().value, ", array size is ", arrayType->size);
    }

    return arrayType->elementType;
}

Type TypeChecker::typecheckUnwrapExpr(UnwrapExpr& expr) const {
    Type type = typecheckExpr(*expr.operand);
    if (!type.isNullablePointer()) {
        error(expr.getLocation(), "cannot unwrap non-pointer type '", type, "'");
    }
    return PointerType::get(type.getPointee(), true);
}

Type TypeChecker::typecheckExpr(Expr& expr) const {
    llvm::Optional<Type> type;
    switch (expr.getKind()) {
        case ExprKind::VarExpr: type = typecheckVarExpr(expr.getVarExpr()); break;
        case ExprKind::StringLiteralExpr: type = typecheckStringLiteralExpr(expr.getStringLiteralExpr()); break;
        case ExprKind::IntLiteralExpr: type = typecheckIntLiteralExpr(expr.getIntLiteralExpr()); break;
        case ExprKind::FloatLiteralExpr: type = typecheckFloatLiteralExpr(expr.getFloatLiteralExpr()); break;
        case ExprKind::BoolLiteralExpr: type = typecheckBoolLiteralExpr(expr.getBoolLiteralExpr()); break;
        case ExprKind::NullLiteralExpr: type = typecheckNullLiteralExpr(expr.getNullLiteralExpr()); break;
        case ExprKind::ArrayLiteralExpr: type = typecheckArrayLiteralExpr(expr.getArrayLiteralExpr()); break;
        case ExprKind::PrefixExpr: type = typecheckPrefixExpr(expr.getPrefixExpr()); break;
        case ExprKind::BinaryExpr: type = typecheckBinaryExpr(expr.getBinaryExpr()); break;
        case ExprKind::CallExpr: type = typecheckCallExpr(expr.getCallExpr()); break;
        case ExprKind::CastExpr: type = typecheckCastExpr(expr.getCastExpr()); break;
        case ExprKind::MemberExpr: type = typecheckMemberExpr(expr.getMemberExpr()); break;
        case ExprKind::SubscriptExpr: type = typecheckSubscriptExpr(expr.getSubscriptExpr()); break;
        case ExprKind::UnwrapExpr: type = typecheckUnwrapExpr(expr.getUnwrapExpr()); break;
    }
    assert(*type);
    expr.setType(resolve(*type));
    return expr.getType();
}

bool TypeChecker::isValidConversion(std::vector<std::unique_ptr<Expr>>& exprs, Type source,
                                    Type target) const {
    if (!source.isTupleType()) {
        assert(!target.isTupleType());
        assert(exprs.size() == 1);
        return isValidConversion(*exprs[0], source, target);
    }
    assert(target.isTupleType());

    for (size_t i = 0; i < exprs.size(); ++i) {
        if (!isValidConversion(*exprs[i], source.getSubtypes()[i], target.getSubtypes()[i])) {
            return false;
        }
    }
    return true;
}
