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
    Decl& decl = findDecl(expr.identifier, expr.srcLoc);
    expr.setDecl(&decl);

    switch (decl.getKind()) {
        case DeclKind::VarDecl: return decl.getVarDecl().getType();
        case DeclKind::ParamDecl: return decl.getParamDecl().type;
        case DeclKind::FuncDecl: return decl.getFuncDecl().getFuncType();
        case DeclKind::GenericParamDecl: llvm_unreachable("cannot refer to generic parameters yet");
        case DeclKind::InitDecl: llvm_unreachable("cannot refer to initializers yet");
        case DeclKind::DeinitDecl: llvm_unreachable("cannot refer to deinitializers yet");
        case DeclKind::TypeDecl: error(expr.srcLoc, "'", expr.identifier, "' is not a variable");
        case DeclKind::FieldDecl:
            assert(currentFunc);
            if ((currentFunc->isFuncDecl() && currentFunc->getFuncDecl().isMutating()) ||
                currentFunc->isInitDecl()) {
                return decl.getFieldDecl().type;
            } else {
                return decl.getFieldDecl().type.asImmutable();
            }
        case DeclKind::ImportDecl:
            llvm_unreachable("import statement validation not implemented yet");
    }
    abort();
}

Type typecheckStrLiteralExpr(StrLiteralExpr&) {
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
    error(expr.srcLoc, "integer literal is too large");
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
            error(element->getSrcLoc(), "mixed element types in array literal (expected '",
                  firstType, "', found '", type, "')");
        }
    }
    return ArrayType::get(firstType, int64_t(array.elements.size()));
}

Type TypeChecker::typecheckPrefixExpr(PrefixExpr& expr) const {
    Type operandType = typecheckExpr(expr.getOperand());

    if (expr.op == NOT) {
        if (!operandType.isBool()) {
            error(expr.getOperand().getSrcLoc(), "invalid operand type '", operandType,
                  "' to logical not");
        }
        return operandType;
    }
    if (expr.op == STAR) { // Dereference operation
        if (!operandType.isPtrType()) {
            error(expr.getOperand().getSrcLoc(), "cannot dereference non-pointer type '",
                  operandType, "'");
        }
        return operandType.getPointee();
    }
    if (expr.op == AND) { // Address-of operation
        return PtrType::get(operandType, true);
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
        error(expr.srcLoc, "invalid operands to binary expression ('", leftType, "' and '",
              rightType, "')");
    }

    if (expr.op.isBitwiseOperator() &&
        (leftType.isFloatingPoint() || rightType.isFloatingPoint())) {
        error(expr.srcLoc, "invalid operands to binary expression ('", leftType, "' and '",
              rightType, "')");
    }

    if (!isValidConversion(expr.getLHS(), leftType, rightType) &&
        !isValidConversion(expr.getRHS(), rightType, leftType)) {
        error(expr.srcLoc, "invalid operands to binary expression ('", leftType, "' and '",
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
        error(expr.getSrcLoc(), value, " is out of range for type '", type, "'");
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
        case TypeKind::PtrType:
            return PtrType::get(resolve(type.getPointee()), type.isRef(), type.isMutable());
        case TypeKind::ArrayType:
            return ArrayType::get(resolve(type.getElementType()), type.getArraySize(),
                                  type.isMutable());
        case TypeKind::RangeType:
            return RangeType::get(resolve(type.getIterableElementType()),
                                  llvm::cast<RangeType>(*type).isExclusive(), type.isMutable());
        case TypeKind::FuncType: {
            std::vector<Type> resolvedParamTypes;
            resolvedParamTypes.reserve(type.getParamTypes().size());
            for (Type type : type.getParamTypes()) {
                resolvedParamTypes.emplace_back(resolve(type));
            }
            return FuncType::get(resolve(type.getReturnType()), std::move(resolvedParamTypes),
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

bool TypeChecker::hasMemberFunc(TypeDecl& type, FuncDecl& func) const {
    auto decls = findDecls(mangleFuncDecl(type.name, func.name));
    for (Decl* decl : decls) {
        if (!decl->isFuncDecl()) continue;
        if (!decl->getFuncDecl().getReceiverTypeDecl()) continue;
        if (decl->getFuncDecl().getReceiverTypeDecl()->name != type.name) continue;
        if (!decl->getFuncDecl().signatureMatches(func, /* matchReceiver: */ false)) continue;
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
    for (auto& memberFuncRequirement : interface.memberFuncs) {
        if (!memberFuncRequirement->isFuncDecl()) {
            fatalError("non-function interface member requirements are not supported yet");
        }
        if (!hasMemberFunc(type, memberFuncRequirement->getFuncDecl())) {
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
        if (typeDecl && !typeDecl->passByValue() && !target.isPtrType()) {
            error(expr.getSrcLoc(), "implicit copying of class instances is disallowed");
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
    } else if (expr.isStrLiteralExpr() && target.isPtrType() && target.getPointee().isChar() &&
               !target.getPointee().isMutable()) {
        // Special case: allow passing string literals as C-strings (const char*).
        expr.setType(target);
        return true;
    } else if (expr.isLvalue() && source.isBasicType() && target.isPtrType()) {
        auto typeDecl = getTypeDecl(llvm::cast<BasicType>(*source));
        if (!typeDecl || typeDecl->passByValue()) {
            error(expr.getSrcLoc(),
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
                Type argType = typecheckExpr(*std::get<1>(tuple).value);

                if (!genericArg) {
                    genericArg = argType;
                } else if (!argType.isImplicitlyConvertibleTo(genericArg)) {
                    error(std::get<1>(tuple).srcLoc, "couldn't infer generic parameter '",
                          genericParam.name, "' because of conflicting argument types '",
                          genericArg, "' and '", argType, "'");
                }
            }
        }

        if (genericArg) {
            genericArgs.emplace_back(genericArg);
        } else {
            error(call.getSrcLoc(), "couldn't infer generic parameter '", genericParam.name, "'");
        }
    }

    return genericArgs;
}

static void validateGenericArgCount(size_t genericParamCount, const CallExpr& call) {
    if (call.genericArgs.size() < genericParamCount) {
        error(call.srcLoc, "too few generic arguments to '", call.getFuncName(), "', expected ",
              genericParamCount);
    } else if (call.genericArgs.size() > genericParamCount) {
        error(call.srcLoc, "too many generic arguments to '", call.getFuncName(), "', expected ",
              genericParamCount);
    }
}

void TypeChecker::setCurrentGenericArgs(llvm::ArrayRef<GenericParamDecl> genericParams,
                                        CallExpr& call, llvm::ArrayRef<ParamDecl> params) const {
    if (genericParams.empty()) return;

    if (call.genericArgs.empty()) {
        call.genericArgs = inferGenericArgs(genericParams, call, params);
        assert(call.genericArgs.size() == genericParams.size());
    } else {
        validateGenericArgCount(genericParams.size(), call);
    }

    auto genericArg = call.genericArgs.begin();
    for (const GenericParamDecl& genericParam : genericParams) {
        if (!genericParam.constraints.empty()) {
            assert(genericParam.constraints.size() == 1 &&
                   "cannot have multiple generic constraints yet");

            auto interfaces = findDecls(genericParam.constraints[0]);
            assert(interfaces.size() == 1);

            if (genericArg->isBasicType() &&
                !implementsInterface(*getTypeDecl(llvm::cast<BasicType>(**genericArg)),
                                     interfaces[0]->getTypeDecl())) {
                error(call.srcLoc, "type '", *genericArg, "' doesn't implement interface '",
                      genericParam.constraints[0], "'");
            }
        }
        currentGenericArgs.insert({genericParam.name, *genericArg++});
    }
}

Type TypeChecker::typecheckBuiltinConversion(CallExpr& expr) const {
    if (expr.args.size() != 1) {
        error(expr.srcLoc, "expected single argument to converting initializer");
    }
    if (!expr.genericArgs.empty()) {
        error(expr.srcLoc, "expected no generic arguments to converting initializer");
    }
    if (!expr.args.front().name.empty()) {
        error(expr.srcLoc, "expected unnamed argument to converting initializer");
    }
    typecheckExpr(*expr.args.front().value);
    expr.setType(BasicType::get(expr.getFuncName(), {}));
    return expr.getType();
}

Decl& TypeChecker::resolveOverload(CallExpr& expr, llvm::StringRef callee) const {
    llvm::SmallVector<Decl*, 1> matches;
    bool isInitCall = false;
    bool atLeastOneFunction = false;
    auto decls = findDecls(callee, typecheckingGenericFunc);

    for (Decl* decl : decls) {
        switch (decl->getKind()) {
            case DeclKind::FuncDecl:
                if (expr.isMemberFuncCall()) {
                    Type receiverType = expr.getReceiver()->getType().removePtr();
                    if (receiverType.isBasicType() && !receiverType.getGenericArgs().empty()) {
                        TypeDecl* typeDecl = getTypeDecl(llvm::cast<BasicType>(*receiverType));
                        assert(typeDecl->genericParams.size() == receiverType.getGenericArgs().size());
                        for (auto t : llvm::zip_first(typeDecl->genericParams, receiverType.getGenericArgs())) {
                            currentGenericArgs.emplace(std::get<0>(t).name, std::get<1>(t));
                        }
                    }
                }

                setCurrentGenericArgs(decl->getFuncDecl().getGenericParams(), expr,
                                      decl->getFuncDecl().getParams());

                if (decls.size() == 1) {
                    validateArgs(expr.args, decl->getFuncDecl().params, callee,
                                 expr.func->getSrcLoc());
                    return *decl;
                }
                if (matchArgs(expr.args, decl->getFuncDecl().params)) {
                    matches.push_back(decl);
                }

                currentGenericArgs.clear();
                break;

            case DeclKind::TypeDecl: {
                isInitCall = true;
                auto mangledName = mangleInitDecl(decl->getTypeDecl().name);
                auto initDecls = findDecls(mangledName);

                for (Decl* initDecl : initDecls) {
                    if (initDecls.size() == 1) {
                        validateArgs(expr.args, initDecl->getInitDecl().params, callee,
                                     expr.func->getSrcLoc());
                        return *initDecl;
                    }
                    if (matchArgs(expr.args, initDecl->getInitDecl().params)) {
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
                error(expr.func->getSrcLoc(), "unknown identifier '", callee, "'");
            } else if (atLeastOneFunction) {
                error(expr.func->getSrcLoc(), "no matching ",
                      isInitCall ? "initializer for '" : "function for call to '", callee, "'");
            } else {
                error(expr.func->getSrcLoc(), "'", callee, "' is not a function");
            }
        default:
            for (Decl* match : matches) {
                if (match->getModule() && match->getModule()->getName() == "std") {
                    return *match;
                }
            }
            error(expr.func->getSrcLoc(), "ambiguous reference to '", callee,
                  isInitCall ? ".init'" : "'");
    }
}

Type TypeChecker::typecheckCallExpr(CallExpr& expr) const {
    if (!expr.callsNamedFunc()) {
        fatalError("anonymous function calls not implemented yet");
    }

    if (Type::isBuiltinScalar(expr.getFuncName())) {
        return typecheckBuiltinConversion(expr);
    }

    if (expr.getFuncName() == "sizeOf") {
        validateArgs(expr.args, {}, expr.getFuncName(), expr.getSrcLoc());
        validateGenericArgCount(1, expr);
        return Type::getUInt64();
    }

    Decl* decl;

    if (expr.func->isMemberExpr()) {
        Type receiverType = typecheckExpr(*expr.getReceiver());
        expr.setReceiverType(receiverType);
        decl = &resolveOverload(expr, expr.getMangledFuncName());

        if (receiverType.isNullablePointer()) {
            error(expr.getReceiver()->getSrcLoc(), "cannot call member function through pointer '",
                  receiverType, "', pointer may be null");
        }
    } else {
        decl = &resolveOverload(expr, expr.getFuncName());

        if (decl->isFuncDecl() && decl->getFuncDecl().isMemberFunc()) {
            expr.setReceiverType(findDecl("this", expr.func->getSrcLoc()).getVarDecl().getType());
        }
    }

    expr.setCalleeDecl(decl);

    if (auto* funcDecl = llvm::dyn_cast<FuncDecl>(decl)) {
        bool hasGenericReceiverType =
            funcDecl->getReceiverTypeDecl() && funcDecl->getReceiverTypeDecl()->isGeneric();

        if (!funcDecl->isGeneric() && !hasGenericReceiverType) {
            return funcDecl->getFuncType()->returnType;
        } else {
            setCurrentGenericArgs(funcDecl->getGenericParams(), expr, funcDecl->getParams());
            // TODO: Don't typecheck more than once with the same generic arguments.
            typecheckingGenericFunc = true;
            typecheckFuncDecl(*funcDecl);
            typecheckingGenericFunc = false;
            Type returnType = resolve(funcDecl->getFuncType()->returnType);
            currentGenericArgs.clear();
            return returnType;
        }
    } else if (auto* initDecl = llvm::dyn_cast<InitDecl>(decl)) {
        if (initDecl->getTypeDecl().isGeneric()) {
            setCurrentGenericArgs(initDecl->getTypeDecl().genericParams, expr,
                                  initDecl->getParams());
            // TODO: Don't typecheck more than once with the same generic arguments.
            typecheckInitDecl(*initDecl);
            currentGenericArgs.clear();
        }
        return initDecl->getTypeDecl().getType(expr.genericArgs);
    }
    llvm_unreachable("all cases handled");
}

bool TypeChecker::matchArgs(llvm::ArrayRef<Arg> args, llvm::ArrayRef<ParamDecl> params) const {
    if (args.size() != params.size()) return false;

    for (size_t i = 0; i < params.size(); ++i) {
        const Arg& arg = args[i];
        const ParamDecl& param = params[i];

        if (!arg.name.empty() && arg.name != param.name) return false;
        auto argType = typecheckExpr(*arg.value);
        if (!isValidConversion(*arg.value, argType, param.type)) return false;
    }
    return true;
}

void TypeChecker::validateArgs(const std::vector<Arg>& args, const std::vector<ParamDecl>& params,
                               const std::string& funcName, SrcLoc srcLoc) const {
    if (args.size() < params.size()) {
        error(srcLoc, "too few arguments to '", funcName, "', expected ", params.size());
    }
    if (args.size() > params.size()) {
        error(srcLoc, "too many arguments to '", funcName, "', expected ", params.size());
    }

    for (size_t i = 0; i < params.size(); ++i) {
        const Arg& arg = args[i];
        const ParamDecl& param = params[i];

        if (!arg.name.empty() && arg.name != param.name) {
            error(arg.srcLoc, "invalid argument name '", arg.name, "' for parameter '", param.name, "'");
        }
        auto argType = typecheckExpr(*arg.value);
        if (!isValidConversion(*arg.value, argType, param.type)) {
            error(arg.srcLoc, "invalid argument #", i + 1, " type '", argType, "' to '", funcName,
                  "', expected '", param.type, "'");
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
        case TypeKind::FuncType:
            break;
        case TypeKind::PtrType:
            Type sourcePointee = sourceType.getPointee();
            if (targetType.isPtrType()) {
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
    error(expr.srcLoc, "illegal cast from '", sourceType, "' to '", targetType, "'");
}

Type TypeChecker::typecheckMemberExpr(MemberExpr& expr) const {
    Type baseType = typecheckExpr(*expr.base);

    if (baseType.isPtrType()) {
        if (!baseType.isRef()) {
            error(expr.base->srcLoc, "cannot access member through pointer '", baseType,
                  "', pointer may be null");
        }
        baseType = baseType.getPointee();
    }

    if (baseType.isArrayType() || baseType.isString()) {
        if (expr.member == "data") return PtrType::get(Type::getChar(), true);
        if (expr.member == "count") return Type::getInt();
        error(expr.srcLoc, "no member named '", expr.member, "' in '", baseType, "'");
    }

    Decl& typeDecl = findDecl(baseType.getName(), SrcLoc::invalid());

    for (const FieldDecl& field : typeDecl.getTypeDecl().fields) {
        if (field.name == expr.member) {
            if (baseType.isMutable()) {
                return field.type;
            } else {
                return field.type.asImmutable();
            }
        }
    }
    error(expr.srcLoc, "no member named '", expr.member, "' in '", baseType, "'");
}

Type TypeChecker::typecheckSubscriptExpr(SubscriptExpr& expr) const {
    Type lhsType = typecheckExpr(*expr.array);
    const ArrayType* arrayType;

    if (lhsType.isArrayType()) {
        arrayType = &llvm::cast<ArrayType>(*lhsType);
    } else if (lhsType.isRef() && lhsType.getReferee().isArrayType()) {
        arrayType = &llvm::cast<ArrayType>(*lhsType.getReferee());
    } else {
        error(expr.array->getSrcLoc(), "cannot subscript '", lhsType,
              "', expected array or reference-to-array");
    }

    Type indexType = typecheckExpr(*expr.index);
    if (!isValidConversion(*expr.index, indexType, Type::getInt())) {
        error(expr.index->getSrcLoc(), "illegal subscript index type '", indexType,
              "', expected 'int'");
    }

    if (!arrayType->isUnsized() && expr.index->isIntLiteralExpr() &&
        expr.index->getIntLiteralExpr().value >= arrayType->size) {
        error(expr.index->getSrcLoc(), "accessing array out-of-bounds with index ",
              expr.index->getIntLiteralExpr().value, ", array size is ", arrayType->size);
    }

    return arrayType->elementType;
}

Type TypeChecker::typecheckUnwrapExpr(UnwrapExpr& expr) const {
    Type type = typecheckExpr(*expr.operand);
    if (!type.isNullablePointer()) {
        error(expr.srcLoc, "cannot unwrap non-pointer type '", type, "'");
    }
    return PtrType::get(type.getPointee(), true);
}

Type TypeChecker::typecheckExpr(Expr& expr) const {
    llvm::Optional<Type> type;
    switch (expr.getKind()) {
        case ExprKind::VarExpr: type = typecheckVarExpr(expr.getVarExpr()); break;
        case ExprKind::StrLiteralExpr: type = typecheckStrLiteralExpr(expr.getStrLiteralExpr()); break;
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
