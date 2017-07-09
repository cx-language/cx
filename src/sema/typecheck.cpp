#include <algorithm> // std::equal
#include <iterator>
#include <limits>
#include <memory>
#include <unordered_map>
#include <vector>
#include <cstdlib>
#include <system_error>
#include <llvm/ADT/APSInt.h>
#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/Optional.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/ADT/iterator_range.h>
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
#include "../ast/token.h"
#include "../ast/mangle.h"
#include "../support/utility.h"

using namespace delta;

namespace delta {
    std::unordered_map<std::string, std::shared_ptr<Module>> allImportedModules;
}

std::vector<Module*> delta::getAllImportedModules() {
    std::vector<Module*> modules;
    modules.reserve(allImportedModules.size());
    for (auto& p : allImportedModules) modules.emplace_back(p.second.get());
    return modules;
}

namespace {

/// Storage for Decls that are not in the AST but are referenced by the symbol table.
std::vector<std::unique_ptr<Decl>> nonASTDecls;

std::unordered_map<std::string, Type> currentGenericArgs;
llvm::MutableArrayRef<FieldDecl> currentFieldDecls;
Type funcReturnType = nullptr;
bool inInitializer = false;
int breakableBlocks = 0;

}

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
        case DeclKind::ImportDecl: llvm_unreachable("import statement validation not implemented yet");
    }
    abort();
}

Type typecheckStrLiteralExpr(StrLiteralExpr&) {
    return Type::getString();
}

Type typecheckIntLiteralExpr(IntLiteralExpr& expr) {
    if (expr.value >= std::numeric_limits<int32_t>::min()
    &&  expr.value <= std::numeric_limits<int32_t>::max())
        return Type::getInt();
    else
    if (expr.value >= std::numeric_limits<int64_t>::min()
    &&  expr.value <= std::numeric_limits<int64_t>::max())
        return Type::getInt64();
    else
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
            error(expr.getOperand().getSrcLoc(), "invalid operand type '", operandType, "' to logical not");
        }
        return operandType;
    }
    if (expr.op == STAR) { // Dereference operation
        if (!operandType.isPtrType()) {
            error(expr.getOperand().getSrcLoc(), "cannot dereference non-pointer type '", operandType, "'");
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

    if (!expr.isBuiltinOp()) return typecheckCallExpr((CallExpr&) expr);

    if (expr.op == AND_AND || expr.op == OR_OR) {
        if (leftType.isBool() && rightType.isBool()) {
            return Type::getBool();
        }
        error(expr.srcLoc, "invalid operands to binary expression ('", leftType, "' and '", rightType, "')");
    }

    if (expr.op.isBitwiseOperator() && (leftType.isFloatingPoint() || rightType.isFloatingPoint())) {
        error(expr.srcLoc, "invalid operands to binary expression ('", leftType, "' and '", rightType, "')");
    }

    if (!isValidConversion(expr.getLHS(), leftType, rightType)
    &&  !isValidConversion(expr.getRHS(), rightType, leftType)) {
        error(expr.srcLoc, "invalid operands to binary expression ('", leftType, "' and '", rightType, "')");
    }

    if (expr.op == DOTDOT) return RangeType::get(leftType, /*hasExclusiveUpperBound*/ true);
    if (expr.op == DOTDOTDOT) return RangeType::get(leftType, /*hasExclusiveUpperBound*/ false);

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
Type resolve(Type type) {
    switch (type.getKind()) {
        case TypeKind::BasicType: {
            auto it = currentGenericArgs.find(type.getName());
            if (it == currentGenericArgs.end()) return type;
            return it->second;
        }
        case TypeKind::PtrType:
            return PtrType::get(resolve(type.getPointee()), type.isRef(), type.isMutable());
        case TypeKind::ArrayType:
            return ArrayType::get(resolve(type.getElementType()),
                                  type.getArraySize(), type.isMutable());
        case TypeKind::RangeType:
            return RangeType::get(resolve(type.getIterableElementType()),
                                  llvm::cast<RangeType>(*type).isExclusive(), type.isMutable());
        case TypeKind::FuncType:
            return FuncType::get(resolve(type.getReturnType()),
                                 map(type.getParamTypes(), resolve), type.isMutable());
        default:
            fatalError(("resolve() not implemented for type '" + type.toString() + "'").c_str());
    }
}

bool TypeChecker::isInterface(Type unresolvedType) const {
    auto type = resolve(unresolvedType);
    return type.isBasicType() && !type.isBuiltinType() && !type.isVoid()
        && getTypeDecl(llvm::cast<BasicType>(*type))->isInterface();
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
        if (!hasField(type, fieldRequirement)) return false;
    }
    for (auto& memberFuncRequirement : interface.memberFuncs) {
        if (!memberFuncRequirement->isFuncDecl()) {
            fatalError("non-function interface member requirements are not supported yet");
        }
        if (!hasMemberFunc(type, memberFuncRequirement->getFuncDecl())) return false;
    }
    return true;
}

bool TypeChecker::isValidConversion(Expr& expr, Type unresolvedSource, Type unresolvedTarget) const {
    Type source = resolve(unresolvedSource);
    Type target = resolve(unresolvedTarget);

    if (expr.isLvalue() && source.isBasicType()) {
        TypeDecl* typeDecl = getTypeDecl(llvm::cast<BasicType>(*source));
        if (typeDecl && !typeDecl->passByValue() && !target.isPtrType()) {
            error(expr.getSrcLoc(), "implicit copying of class instances is disallowed");
        }
    }
    if (source.isImplicitlyConvertibleTo(target)) return true;

    // Check compatibility with interface type.
    if (isInterface(target) && source.isBasicType()) {
        if (implementsInterface(*getTypeDecl(llvm::cast<BasicType>(*source)),
                                *getTypeDecl(llvm::cast<BasicType>(*target))))
            return true;
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
    } else if (expr.isStrLiteralExpr() && target.isPtrType()
               && target.getPointee().isChar()
               && !target.getPointee().isMutable()) {
        // Special case: allow passing string literals as C-strings (const char*).
        expr.setType(target);
        return true;
    } else if (expr.isLvalue() && source.isBasicType() && target.isPtrType()) {
        auto typeDecl = getTypeDecl(llvm::cast<BasicType>(*source));
        if (!typeDecl || typeDecl->passByValue()) {
            error(expr.getSrcLoc(), "cannot implicitly pass value types by reference, add explicit '&'");
        }
        if (source.isImplicitlyConvertibleTo(target.getPointee())) return true;
    }

    return false;
}

std::vector<Type> TypeChecker::inferGenericArgs(llvm::ArrayRef<GenericParamDecl> genericParams,
                                                const CallExpr& call, llvm::ArrayRef<ParamDecl> params) const {
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

void TypeChecker::setCurrentGenericArgs(llvm::ArrayRef<GenericParamDecl> genericParams,
                                        CallExpr& call, llvm::ArrayRef<ParamDecl> params) const {
    if (genericParams.empty()) return;

    if (call.genericArgs.empty()) {
        call.genericArgs = inferGenericArgs(genericParams, call, params);
        assert(call.genericArgs.size() == genericParams.size());
    }
    else if (call.genericArgs.size() < genericParams.size()) {
        error(call.srcLoc, "too few generic arguments to '", call.getFuncName(),
              "', expected ", genericParams.size());
    }
    else if (call.genericArgs.size() > genericParams.size()) {
        error(call.srcLoc, "too many generic arguments to '", call.getFuncName(),
              "', expected ", genericParams.size());
    }

    auto genericArg = call.genericArgs.begin();
    for (const GenericParamDecl& genericParam : genericParams) {
        if (!genericParam.constraints.empty()) {
            assert(genericParam.constraints.size() == 1
                   && "cannot have multiple generic constraints yet");

            auto interfaces = findDecls(genericParam.constraints[0]);
            assert(interfaces.size() == 1);

            if (genericArg->isBasicType()
            && !implementsInterface(*getTypeDecl(llvm::cast<BasicType>(**genericArg)),
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
    auto decls = findDecls(callee);

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
                    validateArgs(expr.args, decl->getFuncDecl().params,
                                 callee, expr.func->getSrcLoc());
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
                        validateArgs(expr.args, initDecl->getInitDecl().params,
                                     callee, expr.func->getSrcLoc());
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
                error(expr.func->getSrcLoc(), "no matching ", isInitCall ?
                      "initializer for '" : "function for call to '", callee, "'");
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
    if (!expr.callsNamedFunc()) fatalError("anonymous function calls not implemented yet");

    if (Type::isBuiltinScalar(expr.getFuncName()))
        return typecheckBuiltinConversion(expr);

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
        bool hasGenericReceiverType = funcDecl->getReceiverTypeDecl() &&
                                      funcDecl->getReceiverTypeDecl()->isGeneric();

        if (!funcDecl->isGeneric() && !hasGenericReceiverType) {
            return funcDecl->getFuncType()->returnType;
        } else {
            setCurrentGenericArgs(funcDecl->getGenericParams(), expr,
                                  funcDecl->getParams());
            // TODO: Don't typecheck more than once with the same generic arguments.
            typecheckFuncDecl(*funcDecl);
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
            error(arg.srcLoc, "invalid argument name '", arg.name,
                  "' for parameter '", param.name, "'");
        }
        auto argType = typecheckExpr(*arg.value);
        if (!isValidConversion(*arg.value, argType, param.type)) {
            error(arg.srcLoc, "invalid argument #", i + 1, " type '", argType,
                  "' to '", funcName, "', expected '", param.type, "'");
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
                if (sourcePointee.isVoid()
                && (!targetPointee.isMutable() || sourcePointee.isMutable())) {
                    return targetType; // (mutable) void* -> T* / mutable void* -> mutable T*
                }
                if (targetPointee.isVoid()
                && (!targetPointee.isMutable() || sourcePointee.isMutable())) {
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
        error(expr.array->getSrcLoc(), "cannot subscript '", lhsType, "', expected array or reference-to-array");
    }

    Type indexType = typecheckExpr(*expr.index);
    if (!isValidConversion(*expr.index, indexType, Type::getInt())) {
        error(expr.index->getSrcLoc(), "illegal subscript index type '", indexType, "', expected 'int'");
    }

    if (!arrayType->isUnsized() && expr.index->isIntLiteralExpr()
    && expr.index->getIntLiteralExpr().value >= arrayType->size) {
        error(expr.index->getSrcLoc(), "accessing array out-of-bounds with index ",
              expr.index->getIntLiteralExpr().value, ", array size is ", arrayType->size);
    }

    return arrayType->elementType;
}

Type TypeChecker::typecheckUnwrapExpr(UnwrapExpr& expr) const {
    Type type = typecheckExpr(*expr.operand);
    if (!type.isNullablePointer())
        error(expr.srcLoc, "cannot unwrap non-pointer type '", type, "'");
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

bool TypeChecker::isValidConversion(std::vector<std::unique_ptr<Expr>>& exprs, Type source, Type target) const {
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

void TypeChecker::typecheckReturnStmt(ReturnStmt& stmt) const {
    if (stmt.values.empty()) {
        if (!funcReturnType.isVoid()) {
            error(stmt.srcLoc, "expected return statement to return a value of type '", funcReturnType, "'");
        }
        return;
    }
    std::vector<Type> returnValueTypes;
    for (auto& expr : stmt.values) {
        returnValueTypes.push_back(typecheckExpr(*expr));
    }
    Type returnType = returnValueTypes.size() > 1
        ? TupleType::get(std::move(returnValueTypes)) : returnValueTypes[0];
    if (!isValidConversion(stmt.values, returnType, funcReturnType)) {
        error(stmt.srcLoc, "mismatching return type '", returnType, "', expected '", funcReturnType, "'");
    }
}

void TypeChecker::typecheckVarStmt(VarStmt& stmt) const {
    typecheckVarDecl(*stmt.decl, false);
}

void TypeChecker::typecheckIncrementStmt(IncrementStmt& stmt) const {
    auto type = typecheckExpr(*stmt.operand);
    if (!type.isMutable()) {
        error(stmt.srcLoc, "cannot increment immutable value");
    }
    // TODO: check that operand supports increment operation.
}

void TypeChecker::typecheckDecrementStmt(DecrementStmt& stmt) const {
    auto type = typecheckExpr(*stmt.operand);
    if (!type.isMutable()) {
        error(stmt.srcLoc, "cannot decrement immutable value");
    }
    // TODO: check that operand supports decrement operation.
}

void TypeChecker::typecheckIfStmt(IfStmt& ifStmt) const {
    Type conditionType = typecheckExpr(*ifStmt.condition);
    if (!conditionType.isBool()) {
        error(ifStmt.condition->getSrcLoc(), "'if' condition must have type 'bool'");
    }
    for (auto& stmt : ifStmt.thenBody) typecheckStmt(*stmt);
    for (auto& stmt : ifStmt.elseBody) typecheckStmt(*stmt);
}

void TypeChecker::typecheckSwitchStmt(SwitchStmt& stmt) const {
    Type conditionType = typecheckExpr(*stmt.condition);
    breakableBlocks++;
    for (SwitchCase& switchCase : stmt.cases) {
        Type caseType = typecheckExpr(*switchCase.value);
        if (!caseType.isImplicitlyConvertibleTo(conditionType)) {
            error(switchCase.value->getSrcLoc(), "case value type '", caseType,
                  "' doesn't match switch condition type '", conditionType, "'");
        }
        for (auto& caseStmt : switchCase.stmts) typecheckStmt(*caseStmt);
    }
    for (auto& defaultStmt : stmt.defaultStmts) typecheckStmt(*defaultStmt);
    breakableBlocks--;
}

void TypeChecker::typecheckWhileStmt(WhileStmt& whileStmt) const {
    Type conditionType = typecheckExpr(*whileStmt.condition);
    if (!conditionType.isBool()) {
        error(whileStmt.condition->getSrcLoc(), "'while' condition must have type 'bool'");
    }
    breakableBlocks++;
    for (auto& stmt : whileStmt.body) typecheckStmt(*stmt);
    breakableBlocks--;
}

void TypeChecker::typecheckForStmt(ForStmt& forStmt) const {
    if (getCurrentModule()->getSymbolTable().contains(forStmt.id)) {
        error(forStmt.srcLoc, "redefinition of '", forStmt.id, "'");
    }

    Type rangeType = typecheckExpr(*forStmt.range);
    if (!rangeType.isIterable()) {
        error(forStmt.range->getSrcLoc(), "'for' range expression is not an 'Iterable'");
    }

    getCurrentModule()->getSymbolTable().pushScope();
    addToSymbolTable(VarDecl(rangeType.getIterableElementType(), std::string(forStmt.id),
                             nullptr, getCurrentModule(), forStmt.srcLoc));
    breakableBlocks++;
    for (auto& stmt : forStmt.body) typecheckStmt(*stmt);
    breakableBlocks--;
    getCurrentModule()->getSymbolTable().popScope();
}

void TypeChecker::typecheckBreakStmt(BreakStmt& breakStmt) const {
    if (breakableBlocks == 0) {
        error(breakStmt.srcLoc, "'break' is only allowed inside 'while' and 'switch' statements");
    }
}

void TypeChecker::typecheckAssignment(Expr& lhs, Expr& rhs, SrcLoc srcLoc) const {
    Type lhsType = typecheckExpr(lhs);
    if (lhsType.isFuncType()) error(srcLoc, "cannot assign to function");
    Type rhsType = typecheckExpr(rhs);
    if (!isValidConversion(rhs, rhsType, lhsType)) {
        error(rhs.getSrcLoc(), "cannot assign '", rhsType, "' to variable of type '", lhsType, "'");
    }
    if (!lhsType.isMutable() && !inInitializer) {
        if (lhs.isVarExpr()) {
            error(srcLoc, "cannot assign to immutable variable '",
                  lhs.getVarExpr().identifier, "'");
        } else {
            error(srcLoc, "cannot assign to immutable expression");
        }
    }
}

void TypeChecker::typecheckAssignStmt(AssignStmt& stmt) const {
    typecheckAssignment(*stmt.lhs, *stmt.rhs, stmt.srcLoc);
}

void TypeChecker::typecheckAugAssignStmt(AugAssignStmt& stmt) const {
    // FIXME: Don't create temporary BinaryExpr.
    BinaryExpr expr(stmt.op, std::unique_ptr<Expr>(stmt.lhs.get()),
                    std::unique_ptr<Expr>(stmt.rhs.get()), stmt.srcLoc);
    typecheckAssignment(*stmt.lhs, expr, stmt.srcLoc);
    expr.args[0].value.release();
    expr.args[1].value.release();
}

void TypeChecker::typecheckStmt(Stmt& stmt) const {
    switch (stmt.getKind()) {
        case StmtKind::ReturnStmt: typecheckReturnStmt(stmt.getReturnStmt()); break;
        case StmtKind::VarStmt: typecheckVarStmt(stmt.getVarStmt()); break;
        case StmtKind::IncrementStmt: typecheckIncrementStmt(stmt.getIncrementStmt()); break;
        case StmtKind::DecrementStmt: typecheckDecrementStmt(stmt.getDecrementStmt()); break;
        case StmtKind::ExprStmt: typecheckExpr(*stmt.getExprStmt().expr); break;
        case StmtKind::DeferStmt: typecheckExpr(*stmt.getDeferStmt().expr); break;
        case StmtKind::IfStmt: typecheckIfStmt(stmt.getIfStmt()); break;
        case StmtKind::SwitchStmt: typecheckSwitchStmt(stmt.getSwitchStmt()); break;
        case StmtKind::WhileStmt: typecheckWhileStmt(stmt.getWhileStmt()); break;
        case StmtKind::ForStmt: typecheckForStmt(stmt.getForStmt()); break;
        case StmtKind::BreakStmt: typecheckBreakStmt(stmt.getBreakStmt()); break;
        case StmtKind::AssignStmt: typecheckAssignStmt(stmt.getAssignStmt()); break;
        case StmtKind::AugAssignStmt: typecheckAugAssignStmt(stmt.getAugAssignStmt()); break;
    }
}

void TypeChecker::typecheckParamDecl(ParamDecl& decl) const {
    if (getCurrentModule()->getSymbolTable().contains(decl.name)) {
        error(decl.srcLoc, "redefinition of '", decl.name, "'");
    }
    getCurrentModule()->getSymbolTable().add(decl.name, &decl);
}

void TypeChecker::addToSymbolTable(FuncDecl& decl) const {
    if (getCurrentModule()->getSymbolTable().findWithMatchingParams(decl)) {
        error(decl.srcLoc, "redefinition of '", decl.name, "'");
    }
    getCurrentModule()->getSymbolTable().add(mangle(decl), &decl);
}

void TypeChecker::addToSymbolTable(InitDecl& decl) const {
    if (getCurrentModule()->getSymbolTable().findWithMatchingParams(decl)) {
        error(decl.srcLoc, "redefinition of '", decl.getTypeName(), "' initializer");
    }
    Decl& typeDecl = findDecl(decl.getTypeName(), decl.srcLoc);
    if (!typeDecl.isTypeDecl()) error(decl.srcLoc, "'", decl.getTypeName(), "' is not a class or struct");
    decl.typeDecl = &typeDecl.getTypeDecl();

    getCurrentModule()->getSymbolTable().add(mangle(decl), &decl);
}

void TypeChecker::addToSymbolTable(DeinitDecl& decl) const {
    if (getCurrentModule()->getSymbolTable().contains(mangle(decl))) {
        error(decl.srcLoc, "redefinition of '", decl.getTypeName(), "' deinitializer");
    }
    Decl& typeDecl = findDecl(decl.getTypeName(), decl.srcLoc);
    if (!typeDecl.isTypeDecl()) error(decl.srcLoc, "'", decl.getTypeName(), "' is not a class or struct");
    decl.typeDecl = &typeDecl.getTypeDecl();

    getCurrentModule()->getSymbolTable().add(mangle(decl), &decl);
}

void TypeChecker::addToSymbolTable(TypeDecl& decl) const {
    if (getCurrentModule()->getSymbolTable().contains(decl.name)) {
        error(decl.srcLoc, "redefinition of '", decl.name, "'");
    }
    getCurrentModule()->getSymbolTable().add(decl.name, &decl);

    for (auto& memberDecl : decl.getMemberDecls()) {
        switch (memberDecl->getKind()) {
            case DeclKind::FuncDecl:
                addToSymbolTable(memberDecl->getFuncDecl());
                break;
            case DeclKind::InitDecl:
                addToSymbolTable(memberDecl->getInitDecl());
                break;
            case DeclKind::DeinitDecl:
                addToSymbolTable(memberDecl->getDeinitDecl());
                break;
            default:
                llvm_unreachable("invalid member declaration kind");
        }
    }
}

void TypeChecker::addToSymbolTable(VarDecl& decl) const {
    if (getCurrentModule()->getSymbolTable().contains(decl.name)) {
        error(decl.srcLoc, "redefinition of '", decl.name, "'");
    }
    getCurrentModule()->getSymbolTable().add(decl.name, &decl);
}

void TypeChecker::addToSymbolTable(FuncDecl&& decl) const {
    std::string name = decl.name;
    nonASTDecls.push_back(llvm::make_unique<FuncDecl>(std::move(decl)));
    getCurrentModule()->getSymbolTable().add(std::move(name), nonASTDecls.back().get());
}

void TypeChecker::addToSymbolTable(TypeDecl&& decl) const {
    std::string name = decl.name;
    nonASTDecls.push_back(llvm::make_unique<TypeDecl>(std::move(decl)));
    getCurrentModule()->getSymbolTable().add(std::move(name), nonASTDecls.back().get());
}

void TypeChecker::addToSymbolTable(VarDecl&& decl) const {
    std::string name = decl.name;
    nonASTDecls.push_back(llvm::make_unique<VarDecl>(std::move(decl)));
    getCurrentModule()->getSymbolTable().add(std::move(name), nonASTDecls.back().get());
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
static Decl* findDeclInModules(llvm::StringRef name, SrcLoc srcLoc, const ModuleContainer& modules) {
    auto decls = findDeclsInModules(name, modules);

    switch (decls.size()) {
        case 1: return decls[0];
        case 0: return nullptr;
        default: error(srcLoc, "ambiguous reference to '", name, "'");
    }
}

llvm::ArrayRef<std::shared_ptr<Module>> getStdlibModules() {
    auto it = allImportedModules.find("std");
    if (it == allImportedModules.end()) return {};
    return it->second;
}

Decl& TypeChecker::findDecl(llvm::StringRef name, SrcLoc srcLoc, bool everywhere) const {
    assert(!name.empty());

    if (Decl* match = findDeclInModules(name, srcLoc, llvm::makeArrayRef(getCurrentModule()))) {
        return *match;
    }

    for (auto& field : currentFieldDecls) {
        if (field.name == name) {
            return field;
        }
    }

    if (Decl* match = findDeclInModules(name, srcLoc, getStdlibModules())) {
        return *match;
    }

    if (everywhere) {
        if (Decl* match = findDeclInModules(name, srcLoc, getAllImportedModules())) {
            return *match;
        }
    } else {
        if (Decl* match = findDeclInModules(name, srcLoc, getCurrentSourceFile()->getImportedModules())) {
            return *match;
        }
    }

    error(srcLoc, "unknown identifier '", name, "'");
}

llvm::SmallVector<Decl*, 1> TypeChecker::findDecls(llvm::StringRef name, bool everywhere) const {
    llvm::SmallVector<Decl*, 1> decls;

    if (currentFunc && currentFunc->isFuncDecl() && currentFunc->getFuncDecl().isMemberFunc()) {
        for (auto& decl : currentFunc->getFuncDecl().getReceiverTypeDecl()->getMemberDecls()) {
            if (decl->isFuncDecl() && decl->getFuncDecl().name == name) {
                decls.emplace_back(decl.get());
            }
        }
    }

    append(decls, findDeclsInModules(name, llvm::makeArrayRef(getCurrentModule())));
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
            auto& ifStmt = block.back()->getIfStmt();
            return allPathsReturn(ifStmt.thenBody) && allPathsReturn(ifStmt.elseBody);
        }
        case StmtKind::SwitchStmt: {
            auto& switchStmt = block.back()->getSwitchStmt();
            return llvm::all_of(switchStmt.cases,
                                [](const SwitchCase& c) { return allPathsReturn(c.stmts); })
                && allPathsReturn(switchStmt.defaultStmts);
        }
        default:
            return false;
    }
}

void TypeChecker::typecheckGenericParamDecls(llvm::ArrayRef<GenericParamDecl> genericParams) const {
    for (auto& genericParam : genericParams) {
        if (getCurrentModule()->getSymbolTable().contains(genericParam.name)) {
            error(genericParam.srcLoc, "redefinition of '", genericParam.name, "'");
        }
    }
}

std::vector<Type> getGenericArgsAsArray() {
    std::vector<Type> genericArgs;
    genericArgs.reserve(currentGenericArgs.size());
    for (auto& p : currentGenericArgs) genericArgs.emplace_back(p.second);
    return genericArgs;
}

void TypeChecker::typecheckFuncDecl(FuncDecl& decl) const {
    if (decl.isExtern()) return;

    if (decl.isGeneric() && currentGenericArgs.empty()) {
        typecheckGenericParamDecls(decl.genericParams);
        return; // Partial type-checking of uninstantiated generic functions not implemented yet.
    }

    TypeDecl* receiverTypeDecl = decl.getReceiverTypeDecl();
    if (decl.isMemberFunc() && receiverTypeDecl->isGeneric() && currentGenericArgs.empty()) {
        return; // Partial type-checking of uninstantiated generic functions not implemented yet.
    }

    getCurrentModule()->getSymbolTable().pushScope();
    auto* previousFunc = currentFunc;
    currentFunc = &decl;

    for (ParamDecl& param : decl.params) {
        if (param.type.isMutable()) error(param.srcLoc, "parameter types cannot be 'mutable'");
        typecheckParamDecl(param);
    }
    if (decl.returnType.isMutable()) error(decl.srcLoc, "return types cannot be 'mutable'");

    if (decl.body) {
        auto funcReturnTypeBackup = funcReturnType;
        funcReturnType = decl.returnType;
        llvm::MutableArrayRef<FieldDecl> currentFieldDeclsBackup;
        if (receiverTypeDecl) {
            currentFieldDeclsBackup = currentFieldDecls;
            currentFieldDecls = receiverTypeDecl->fields;
            Type thisType = receiverTypeDecl->getTypeForPassing(getGenericArgsAsArray(), decl.isMutating());
            addToSymbolTable(VarDecl(thisType, "this", nullptr, getCurrentModule(), SrcLoc::invalid()));
        }

        for (auto& stmt : *decl.body) typecheckStmt(*stmt);

        if (receiverTypeDecl) {
            currentFieldDecls = currentFieldDeclsBackup;
        }
        funcReturnType = funcReturnTypeBackup;
    }

    currentFunc = previousFunc;
    getCurrentModule()->getSymbolTable().popScope();

    if (!decl.returnType.isVoid() && !allPathsReturn(*decl.body)) {
        error(decl.srcLoc, "'", decl.name, "' is missing a return statement");
    }
}

void TypeChecker::typecheckInitDecl(InitDecl& decl) const {
    getCurrentModule()->getSymbolTable().pushScope();
    auto* previousFunc = currentFunc;
    currentFunc = &decl;

    Decl& typeDecl = findDecl(decl.getTypeName(), decl.srcLoc);
    if (!typeDecl.isTypeDecl()) error(decl.srcLoc, "'", decl.getTypeName(), "' is not a class or struct");

    if (typeDecl.getTypeDecl().isGeneric() && currentGenericArgs.empty()) {
        return; // Partial type-checking of uninstantiated generic functions not implemented yet.
    }

    decl.typeDecl = &typeDecl.getTypeDecl();
    addToSymbolTable(VarDecl(typeDecl.getTypeDecl().getType(getGenericArgsAsArray(), true),
                             "this", nullptr, getCurrentModule(), SrcLoc::invalid()));
    for (ParamDecl& param : decl.params) typecheckParamDecl(param);

    inInitializer = true;
    auto currentFieldDeclsBackup = currentFieldDecls;
    currentFieldDecls = typeDecl.getTypeDecl().fields;
    for (auto& stmt : *decl.body) typecheckStmt(*stmt);
    currentFieldDecls = currentFieldDeclsBackup;
    inInitializer = false;

    currentFunc = previousFunc;
    getCurrentModule()->getSymbolTable().popScope();
}

void TypeChecker::typecheckDeinitDecl(DeinitDecl& decl) const {
    TypeDecl& typeDecl = findDecl(decl.getTypeName(), decl.srcLoc).getTypeDecl();

    if (typeDecl.isGeneric() && currentGenericArgs.empty()) {
        return; // Partial type-checking of uninstantiated generic functions not implemented yet.
    }

    FuncDecl funcDecl(mangle(decl), {}, Type::getVoid(), &typeDecl,
                      {}, getCurrentModule(), decl.getSrcLoc());
    funcDecl.body = decl.body;
    decl.typeDecl = &typeDecl;
    typecheckFuncDecl(funcDecl);
}

void TypeChecker::typecheckTypeDecl(TypeDecl& decl) const {
    for (auto& memberDecl : decl.getMemberDecls()) {
        typecheckMemberDecl(*memberDecl);
    }
}

TypeDecl* TypeChecker::getTypeDecl(const BasicType& type) const {
    auto decls = findDecls(type.name);
    if (decls.empty()) return nullptr;
    assert(decls.size() == 1);
    return &decls[0]->getTypeDecl();
}

void TypeChecker::typecheckVarDecl(VarDecl& decl, bool isGlobal) const {
    if (!isGlobal && getCurrentModule()->getSymbolTable().contains(decl.name)) {
        error(decl.srcLoc, "redefinition of '", decl.name, "'");
    }
    Type initType = nullptr;
    if (decl.initializer) {
        initType = typecheckExpr(*decl.initializer);
        if (initType.isFuncType()) {
            error(decl.initializer->getSrcLoc(), "function pointers not implemented yet");
        }
    } else if (isGlobal) {
        error(decl.srcLoc, "global variables cannot be uninitialized");
    }

    if (auto declaredType = decl.getType()) {
        if (initType && !isValidConversion(*decl.initializer, initType, declaredType)) {
            error(decl.initializer->getSrcLoc(), "cannot initialize variable of type '", declaredType,
                "' with '", initType, "'");
        }
    } else {
        if (initType.isNull()) {
            error(decl.srcLoc, "couldn't infer type of '", decl.name, "', add a type annotation");
        }

        initType.setMutable(decl.getType().isMutable());
        decl.type = initType;
    }

    if (!isGlobal) addToSymbolTable(decl);
}

void typecheckFieldDecl(FieldDecl&) {
}

llvm::ErrorOr<const Module&> importDeltaModule(SourceFile* importer,
                                               llvm::ArrayRef<llvm::StringRef> importSearchPaths,
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

    for (llvm::StringRef importPath : importSearchPaths) {
        llvm::sys::fs::directory_iterator it(importPath, error), end;
        for (; it != end; it.increment(error)) {
            if (error) goto done;
            if (!llvm::sys::fs::is_directory(it->path())) continue;
            if (llvm::sys::path::filename(it->path()) != moduleExternalName) continue;

            it = llvm::sys::fs::directory_iterator(it->path(), error);
            for (; it != end; it.increment(error)) {
                if (error) goto done;
                if (llvm::sys::path::extension(it->path()) == ".delta") {
                    parse(it->path(), *module);
                }
            }
            goto done;
        }
    }

done:
    if (error || module->getSourceFiles().empty()) {
        return error;
    }

    if (importer) importer->addImportedModule(module);
    allImportedModules[module->getName()] = module;
    typecheckModule(*module, importSearchPaths, parse);
    return *module;
}

void TypeChecker::typecheckImportDecl(ImportDecl& decl, llvm::ArrayRef<llvm::StringRef> importSearchPaths,
                                      ParserFunction& parse) const {
    if (importDeltaModule(currentSourceFile, importSearchPaths, parse, decl.target)) return;

    if (!importCHeader(*currentSourceFile, decl.target, importSearchPaths)) {
        llvm::errs() << "error: couldn't find module or C header '" << decl.target << "'\n";
        abort();
    }
}

void TypeChecker::typecheckTopLevelDecl(Decl& decl, llvm::ArrayRef<llvm::StringRef> importSearchPaths,
                                        ParserFunction& parse) const {
    switch (decl.getKind()) {
        case DeclKind::ParamDecl: typecheckParamDecl(decl.getParamDecl()); break;
        case DeclKind::FuncDecl: typecheckFuncDecl(decl.getFuncDecl()); break;
        case DeclKind::GenericParamDecl: typecheckGenericParamDecls(decl.getGenericParamDecl()); break;
        case DeclKind::InitDecl: typecheckInitDecl(decl.getInitDecl()); break;
        case DeclKind::DeinitDecl: typecheckDeinitDecl(decl.getDeinitDecl()); break;
        case DeclKind::TypeDecl: typecheckTypeDecl(decl.getTypeDecl()); break;
        case DeclKind::VarDecl: typecheckVarDecl(decl.getVarDecl(), true); break;
        case DeclKind::FieldDecl: typecheckFieldDecl(decl.getFieldDecl()); break;
        case DeclKind::ImportDecl: typecheckImportDecl(decl.getImportDecl(), importSearchPaths, parse); break;
    }
}

void TypeChecker::typecheckMemberDecl(Decl& decl) const {
    switch (decl.getKind()) {
        case DeclKind::FuncDecl: typecheckFuncDecl(decl.getFuncDecl()); break;
        case DeclKind::InitDecl: typecheckInitDecl(decl.getInitDecl()); break;
        case DeclKind::DeinitDecl: typecheckDeinitDecl(decl.getDeinitDecl()); break;
        case DeclKind::TypeDecl: typecheckTypeDecl(decl.getTypeDecl()); break;
        case DeclKind::VarDecl: typecheckVarDecl(decl.getVarDecl(), true); break;
        case DeclKind::FieldDecl: typecheckFieldDecl(decl.getFieldDecl()); break;
        default: llvm_unreachable("invalid member declaration kind");
    }
}

void delta::typecheckModule(Module& module, llvm::ArrayRef<llvm::StringRef> importSearchPaths,
                            ParserFunction& parse) {
    auto stdlibModule = importDeltaModule(nullptr, importSearchPaths, parse, "stdlib", "std");
    if (!stdlibModule) {
        printErrorAndExit("couldn't import the standard library: ", stdlibModule.getError().message());
    }

    // Infer the types of global variables for use before their declaration.
    for (auto& sourceFile : module.getSourceFiles()) {
        TypeChecker typeChecker(&module, &sourceFile);

        for (auto& decl : sourceFile.getTopLevelDecls()) {
            if (decl->isVarDecl()) {
                typeChecker.typecheckVarDecl(decl->getVarDecl(), true);
            }
        }
    }

    for (auto& sourceFile : module.getSourceFiles()) {
        TypeChecker typeChecker(&module, &sourceFile);

        for (auto& decl : sourceFile.getTopLevelDecls()) {
            if (!decl->isVarDecl()) {
                typeChecker.typecheckTopLevelDecl(*decl, importSearchPaths, parse);
            }
        }
    }
}
