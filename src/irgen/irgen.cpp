#include <unordered_map>
#include <vector>
#include <memory>
#include <cassert>
#include <llvm/ADT/STLExtras.h>
#include <llvm/Support/ErrorHandling.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/CFG.h>
#include "irgen.h"
#include "../ast/mangle.h"
#include "../ast/expr.h"
#include "../ast/decl.h"
#include "../ast/module.h"
#include "../ast/token.h"
#include "../sema/typecheck.h"
#include "../support/utility.h"

using namespace delta;

namespace {

using irgen::codegenExpr;
using irgen::toIR;

llvm::Value* codegenCallExpr(const CallExpr& expr);
llvm::Value* codegenLvalueExpr(const Expr& expr);
void codegenInitDecl(const InitDecl& decl, llvm::ArrayRef<Type> typeGenericArgs = {});
llvm::Function* codegenDeinitializerProto(const DeinitDecl& decl);
llvm::Type* codegenGenericTypeInstantiation(const TypeDecl& decl, llvm::ArrayRef<Type> genericArgs);
void deferDeinitCall(llvm::Function* deinit, llvm::Value* valueToDeinit);
void createDeinitCall(llvm::Function* deinit, llvm::Value* valueToDeinit);

struct Scope {
    llvm::SmallVector<const Expr*, 8> deferredExprs;
    llvm::SmallVector<std::pair<llvm::Function*, llvm::Value*>, 8> deinitsToCall;
    std::unordered_map<std::string, llvm::Value*> localValues;

    void onScopeEnd() {
        for (const Expr* expr : llvm::reverse(deferredExprs)) codegenExpr(*expr);
        for (auto& p : llvm::reverse(deinitsToCall)) createDeinitCall(p.first, p.second);
    }

    void clear() {
        deferredExprs.clear();
        deinitsToCall.clear();
    }
};

class FuncInstantiation {
public:
    const FuncDecl& decl;
    llvm::ArrayRef<Type> genericArgs;
    llvm::Function* func;
};

llvm::LLVMContext ctx;
llvm::IRBuilder<> builder(ctx);
llvm::Module module("", ctx);

/// Helper for storing parameter name info in 'funcs' key strings.
template<typename T>
std::string mangleWithParams(const T& decl, llvm::ArrayRef<Type> typeGenericArgs = {},
                             llvm::ArrayRef<Type> funcGenericArgs = {}) {
    std::string result = mangle(decl, typeGenericArgs, funcGenericArgs);
    for (const ParamDecl& param : decl.params) result.append("$").append(param.name);
    return result;
}

std::unordered_map<std::string, FuncInstantiation> funcInstantiations;
std::unordered_map<std::string, std::pair<llvm::StructType*, const TypeDecl*>> structs;
std::unordered_map<std::string, llvm::Type*> currentGenericArgs;
const Decl* currentDecl;
llvm::SmallVector<Scope, 4> scopes(1);
Scope& globalScope() { return scopes.front(); }
llvm::BasicBlock::iterator lastAlloca;

/// The basic blocks to branch to on a 'break' statement, one element per scope.
llvm::SmallVector<llvm::BasicBlock*, 4> breakTargets;

llvm::Function* getDeinitializerFor(Type type) {
    std::string mangledName = mangleDeinitDecl(type.getName());
    auto it = funcInstantiations.find(mangledName);
    if (it == funcInstantiations.end()) {
        auto decls = findDecls(mangledName, /*everywhere*/ true);
        if (!decls.empty())
            return codegenDeinitializerProto(decls[0]->getDeinitDecl());
        return nullptr;
    }
    return it->second.func;
}

/// @param type The Delta type of the variable, or null if the variable is 'this'.
void setLocalValue(Type type, std::string name, llvm::Value* value) {
    bool wasInserted = scopes.back().localValues.emplace(std::move(name), value).second;
    (void) wasInserted;
    assert(wasInserted);

    if (type && type.isBasicType()) {
        llvm::Function* deinit = getDeinitializerFor(type);
        if (deinit) deferDeinitCall(deinit, value);
    }
}

void codegenDecl(const Decl& decl);

llvm::Value* findValue(llvm::StringRef name) {
    llvm::Value* value = nullptr;

    for (const auto& scope : llvm::reverse(scopes)) {
        auto it = scope.localValues.find(name);
        if (it == scope.localValues.end()) continue;
        value = it->second;
        break;
    }

    if (value == nullptr) {
        // FIXME: It would probably be better to not access the symbol table here.
        codegenDecl(findDecl(name, SrcLoc::invalid(), /*everywhere*/ true));
        return globalScope().localValues.find(name)->second;
    }
    return value;
}

void codegenTypeDecl(const TypeDecl& decl);

const std::unordered_map<std::string, llvm::Type*> builtinTypes = {
    { "void", llvm::Type::getVoidTy(ctx) },
    { "bool", llvm::Type::getInt1Ty(ctx) },
    { "char", llvm::Type::getInt8Ty(ctx) },
    { "int", llvm::Type::getInt32Ty(ctx) },
    { "int8", llvm::Type::getInt8Ty(ctx) },
    { "int16", llvm::Type::getInt16Ty(ctx) },
    { "int32", llvm::Type::getInt32Ty(ctx) },
    { "int64", llvm::Type::getInt64Ty(ctx) },
    { "uint", llvm::Type::getInt32Ty(ctx) },
    { "uint8", llvm::Type::getInt8Ty(ctx) },
    { "uint16", llvm::Type::getInt16Ty(ctx) },
    { "uint32", llvm::Type::getInt32Ty(ctx) },
    { "uint64", llvm::Type::getInt64Ty(ctx) },
    { "float", llvm::Type::getFloatTy(ctx) },
    { "float32", llvm::Type::getFloatTy(ctx) },
    { "float64", llvm::Type::getDoubleTy(ctx) },
    { "float80", llvm::Type::getX86_FP80Ty(ctx) },
    { "string", llvm::StructType::get(llvm::Type::getInt8PtrTy(ctx),
                                      llvm::Type::getInt32Ty(ctx), nullptr) },
};

} // anonymous namespace

llvm::Type* irgen::toIR(Type type) {
    switch (type.getKind()) {
        case TypeKind::BasicType: {
            llvm::StringRef name = type.getName();

            auto builtinType = builtinTypes.find(name);
            if (builtinType != builtinTypes.end()) return builtinType->second;

            auto it = structs.find(name);
            if (it == structs.end()) {
                // Is it a generic parameter?
                auto genericArg = currentGenericArgs.find(name);
                if (genericArg != currentGenericArgs.end()) return genericArg->second;

                // Is it a generic type?
                auto genericArgs = llvm::cast<BasicType>(*type).getGenericArgs();
                if (!genericArgs.empty()) {
                    auto& decl = findDecl(name, SrcLoc::invalid(), /*everywhere*/ true).getTypeDecl();
                    return codegenGenericTypeInstantiation(decl, genericArgs);
                }

                // Custom type that has not been declared yet, search for it in the symbol table.
                codegenTypeDecl(findDecl(name, SrcLoc::invalid(), /*everywhere*/ true).getTypeDecl());
                it = structs.find(name);
            }
            return it->second.first;
        }
        case TypeKind::ArrayType:
            assert(type.getArraySize() != ArrayType::unsized && "unimplemented");
            return llvm::ArrayType::get(toIR(type.getElementType()), type.getArraySize());
        case TypeKind::RangeType:
            llvm_unreachable("IRGen doesn't support range types yet");
        case TypeKind::TupleType:
            llvm_unreachable("IRGen doesn't support tuple types yet");
        case TypeKind::FuncType:
            llvm_unreachable("IRGen doesn't support function types yet");
        case TypeKind::PtrType: {
            if (type.getPointee().isUnsizedArrayType())
                return llvm::StructType::get(toIR(type.getPointee().getElementType())->getPointerTo(),
                                             llvm::Type::getInt32Ty(ctx), NULL);
            auto* pointeeType = toIR(type.getPointee());
            if (!pointeeType->isVoidTy()) return llvm::PointerType::get(pointeeType, 0);
            else return llvm::PointerType::get(llvm::Type::getInt8Ty(ctx), 0);
        }
    }
    llvm_unreachable("all cases handled");
}

namespace {

llvm::Value* codegenVarExpr(const VariableExpr& expr) {
    auto* value = findValue(expr.identifier);
    if (auto* arg = llvm::dyn_cast<llvm::Argument>(value)) return arg;
    if (auto* constant = llvm::dyn_cast<llvm::Constant>(value)) return constant;
    return builder.CreateLoad(value, expr.identifier);
}

llvm::Value* codegenLvalueVarExpr(const VariableExpr& expr) {
    return findValue(expr.identifier);
}

llvm::Value* codegenStrLiteralExpr(const StrLiteralExpr& expr) {
    if (expr.getType().isString()) {
        auto* stringPtr = builder.CreateGlobalStringPtr(expr.value);
        auto* string = builder.CreateInsertValue(llvm::UndefValue::get(toIR(Type::getString())),
                                                 stringPtr, 0);
        auto* size = llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), expr.value.size());
        return builder.CreateInsertValue(string, size, 1);
    } else {
        // Passing as C-string, i.e. char pointer.
        return builder.CreateGlobalStringPtr(expr.value);
    }
}

llvm::Value* codegenIntLiteralExpr(const IntLiteralExpr& expr) {
    // Integer literals may be typed as floating-point when used in a context
    // that requires a floating-point value. It might make sense to combine
    // IntLiteralExpr and FloatLiteralExpr into a single class.
    if (expr.getType().isFloatingPoint())
        return llvm::ConstantFP::get(toIR(expr.getType()), expr.value);

    return llvm::ConstantInt::getSigned(toIR(expr.getType()), expr.value);
}

llvm::Value* codegenFloatLiteralExpr(const FloatLiteralExpr& expr) {
    return llvm::ConstantFP::get(toIR(expr.getType()), expr.value);
}

llvm::Value* codegenBoolLiteralExpr(const BoolLiteralExpr& expr) {
    return expr.value ? llvm::ConstantInt::getTrue(ctx) : llvm::ConstantInt::getFalse(ctx);
}

llvm::Value* codegenNullLiteralExpr(const NullLiteralExpr& expr) {
    if (expr.getType().getPointee().isUnsizedArrayType()) {
        return llvm::ConstantStruct::getAnon({
            llvm::ConstantPointerNull::get(toIR(expr.getType().getPointee().getElementType())->getPointerTo()),
            llvm::ConstantInt::getSigned(llvm::Type::getInt32Ty(ctx), 0)
        });
    }
    return llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(toIR(expr.getType())));
}

llvm::Value* codegenArrayLiteralExpr(const ArrayLiteralExpr& expr) {
    auto* arrayType = llvm::ArrayType::get(toIR(expr.elements[0]->getType()), expr.elements.size());
    std::vector<llvm::Constant*> values;
    values.reserve(expr.elements.size());
    for (auto& e : expr.elements)
        values.emplace_back(llvm::cast<llvm::Constant>(codegenExpr(*e)));
    return llvm::ConstantArray::get(arrayType, values);
}

using UnaryCreate   = llvm::Value* (llvm::IRBuilder<>::*)(llvm::Value*, const llvm::Twine&, bool, bool);
using BinaryCreate0 = llvm::Value* (llvm::IRBuilder<>::*)(llvm::Value*, llvm::Value*, const llvm::Twine&);
using BinaryCreate1 = llvm::Value* (llvm::IRBuilder<>::*)(llvm::Value*, llvm::Value*, const llvm::Twine&, bool);
using BinaryCreate2 = llvm::Value* (llvm::IRBuilder<>::*)(llvm::Value*, llvm::Value*, const llvm::Twine&, bool, bool);

llvm::Value* codegenNot(const PrefixExpr& expr) {
    return builder.CreateNot(codegenExpr(expr.getOperand()), "");
}

llvm::Value* codegenPrefixExpr(const PrefixExpr& expr) {
    switch (expr.op) {
        case PLUS: return codegenExpr(expr.getOperand());
        case MINUS:
            if (expr.getOperand().getType().isFloatingPoint()) {
                return builder.CreateFNeg(codegenExpr(expr.getOperand()));
            } else {
                return builder.CreateNeg(codegenExpr(expr.getOperand()));
            }
        case STAR: return builder.CreateLoad(codegenExpr(expr.getOperand()));
        case AND: return codegenLvalueExpr(expr.getOperand());
        case NOT: return codegenNot(expr);
        case COMPL: return codegenNot(expr);
        default: llvm_unreachable("invalid prefix operator");
    }
}

llvm::Value* codegenLvaluePrefixExpr(const PrefixExpr& expr) {
    switch (expr.op) {
        case STAR: return codegenExpr(expr.getOperand());
        default: llvm_unreachable("invalid lvalue prefix operator");
    }
}

llvm::Value* codegenBinaryOp(llvm::Value* lhs, llvm::Value* rhs, BinaryCreate0 create) {
    return (builder.*create)(lhs, rhs, "");
}

llvm::Value* codegenBinaryOp(llvm::Value* lhs, llvm::Value* rhs, BinaryCreate1 create) {
    return (builder.*create)(lhs, rhs, "", false);
}

llvm::Value* codegenBinaryOp(llvm::Value* lhs, llvm::Value* rhs, BinaryCreate2 create) {
    return (builder.*create)(lhs, rhs, "", false, false);
}

llvm::Value* codegenLogicalAnd(const Expr& left, const Expr& right) {
    auto* lhsBlock = builder.GetInsertBlock();
    auto* rhsBlock = llvm::BasicBlock::Create(ctx, "andRHS", builder.GetInsertBlock()->getParent());
    auto* endBlock = llvm::BasicBlock::Create(ctx, "andEnd", builder.GetInsertBlock()->getParent());

    llvm::Value* lhs = codegenExpr(left);
    builder.CreateCondBr(lhs, rhsBlock, endBlock);

    builder.SetInsertPoint(rhsBlock);
    llvm::Value* rhs = codegenExpr(right);
    builder.CreateBr(endBlock);

    builder.SetInsertPoint(endBlock);
    llvm::PHINode* phi = builder.CreatePHI(llvm::IntegerType::getInt1Ty(ctx), 2, "and");
    phi->addIncoming(lhs, lhsBlock);
    phi->addIncoming(rhs, rhsBlock);
    return phi;
}

llvm::Value* codegenLogicalOr(const Expr& left, const Expr& right) {
    auto* lhsBlock = builder.GetInsertBlock();
    auto* rhsBlock = llvm::BasicBlock::Create(ctx, "orRHS", builder.GetInsertBlock()->getParent());
    auto* endBlock = llvm::BasicBlock::Create(ctx, "orEnd", builder.GetInsertBlock()->getParent());

    llvm::Value* lhs = codegenExpr(left);
    builder.CreateCondBr(lhs, endBlock, rhsBlock);

    builder.SetInsertPoint(rhsBlock);
    llvm::Value* rhs = codegenExpr(right);
    builder.CreateBr(endBlock);

    builder.SetInsertPoint(endBlock);
    llvm::PHINode* phi = builder.CreatePHI(llvm::IntegerType::getInt1Ty(ctx), 2, "or");
    phi->addIncoming(lhs, lhsBlock);
    phi->addIncoming(rhs, rhsBlock);
    return phi;
}

llvm::Value* codegenBinaryOp(BinaryOperator op, llvm::Value* lhs, llvm::Value* rhs, const Expr& leftExpr) {
    if (lhs->getType()->isFloatingPointTy()) {
        switch (op) {
            case EQ:    return builder.CreateFCmpOEQ(lhs, rhs);
            case NE:    return builder.CreateFCmpONE(lhs, rhs);
            case LT:    return builder.CreateFCmpOLT(lhs, rhs);
            case LE:    return builder.CreateFCmpOLE(lhs, rhs);
            case GT:    return builder.CreateFCmpOGT(lhs, rhs);
            case GE:    return builder.CreateFCmpOGE(lhs, rhs);
            case PLUS:  return builder.CreateFAdd(lhs, rhs);
            case MINUS: return builder.CreateFSub(lhs, rhs);
            case STAR:  return builder.CreateFMul(lhs, rhs);
            case SLASH: return builder.CreateFDiv(lhs, rhs);
            case MOD:   return builder.CreateFRem(lhs, rhs);
            default:    llvm_unreachable("all cases handled");
        }
    }

    switch (op) {
        case EQ:    return codegenBinaryOp(lhs, rhs, &llvm::IRBuilder<>::CreateICmpEQ);
        case NE:    return codegenBinaryOp(lhs, rhs, &llvm::IRBuilder<>::CreateICmpNE);
        case LT:    return codegenBinaryOp(lhs, rhs, leftExpr.getType().isSigned() ?
                                           &llvm::IRBuilder<>::CreateICmpSLT :
                                           &llvm::IRBuilder<>::CreateICmpULT);
        case LE:    return codegenBinaryOp(lhs, rhs, leftExpr.getType().isSigned() ?
                                           &llvm::IRBuilder<>::CreateICmpSLE :
                                           &llvm::IRBuilder<>::CreateICmpULE);
        case GT:    return codegenBinaryOp(lhs, rhs, leftExpr.getType().isSigned() ?
                                           &llvm::IRBuilder<>::CreateICmpSGT :
                                           &llvm::IRBuilder<>::CreateICmpUGT);
        case GE:    return codegenBinaryOp(lhs, rhs, leftExpr.getType().isSigned() ?
                                           &llvm::IRBuilder<>::CreateICmpSGE :
                                           &llvm::IRBuilder<>::CreateICmpUGE);
        case PLUS:  return codegenBinaryOp(lhs, rhs, &llvm::IRBuilder<>::CreateAdd);
        case MINUS: return codegenBinaryOp(lhs, rhs, &llvm::IRBuilder<>::CreateSub);
        case STAR:  return codegenBinaryOp(lhs, rhs, &llvm::IRBuilder<>::CreateMul);
        case SLASH: return codegenBinaryOp(lhs, rhs, leftExpr.getType().isSigned() ?
                                           &llvm::IRBuilder<>::CreateSDiv :
                                           &llvm::IRBuilder<>::CreateUDiv);
        case MOD:   return codegenBinaryOp(lhs, rhs, leftExpr.getType().isSigned() ?
                                           &llvm::IRBuilder<>::CreateSRem :
                                           &llvm::IRBuilder<>::CreateURem);
        case AND:   return codegenBinaryOp(lhs, rhs, &llvm::IRBuilder<>::CreateAnd);
        case OR:    return codegenBinaryOp(lhs, rhs, &llvm::IRBuilder<>::CreateOr);
        case XOR:   return codegenBinaryOp(lhs, rhs, &llvm::IRBuilder<>::CreateXor);
        case LSHIFT:return codegenBinaryOp(lhs, rhs, &llvm::IRBuilder<>::CreateShl);
        case RSHIFT:return codegenBinaryOp(lhs, rhs, leftExpr.getType().isSigned() ?
                                           (BinaryCreate1) &llvm::IRBuilder<>::CreateAShr :
                                           (BinaryCreate1) &llvm::IRBuilder<>::CreateLShr);
        default:    llvm_unreachable("all cases handled");
    }
}

llvm::Value* codegenShortCircuitBinaryOp(BinaryOperator op, const Expr& lhs, const Expr& rhs) {
    switch (op) {
        case AND_AND: return codegenLogicalAnd(lhs, rhs);
        case OR_OR:   return codegenLogicalOr(lhs, rhs);
        default:      llvm_unreachable("invalid short-circuit binary operator");
    }
}

llvm::Value* codegenBinaryExpr(const BinaryExpr& expr) {
    if (!expr.isBuiltinOp()) return codegenCallExpr((const CallExpr&) expr);

    assert(expr.getLHS().getType().isImplicitlyConvertibleTo(expr.getRHS().getType())
        || expr.getRHS().getType().isImplicitlyConvertibleTo(expr.getLHS().getType()));

    switch (expr.op) {
        case AND_AND: case OR_OR:
            return codegenShortCircuitBinaryOp(expr.op, expr.getLHS(), expr.getRHS());
        default:
            llvm::Value* lhs = codegenExpr(expr.getLHS());
            llvm::Value* rhs = codegenExpr(expr.getRHS());
            return codegenBinaryOp(expr.op, lhs, rhs, expr.getLHS());
    }
}

llvm::Function* getFuncForCall(const CallExpr&);

bool isSizedArrayToUnsizedArrayRefConversion(Type sourceType, llvm::Type* targetType) {
    return sourceType.isPtrType() && sourceType.getPointee().isSizedArrayType()
        && targetType->isStructTy() && targetType->getStructNumElements() == 2
        && targetType->getStructElementType(0)->isPointerTy()
        && targetType->getStructElementType(1)->isIntegerTy(32);
}

llvm::Value* codegenExprForPassing(const Expr& expr, llvm::Type* targetType) {
    if (isSizedArrayToUnsizedArrayRefConversion(expr.getType(), targetType)) {
        assert(expr.getType().getPointee().getArraySize() != ArrayType::unsized);
        auto* elementPtr = builder.CreateConstGEP2_32(nullptr, codegenExpr(expr), 0, 0);
        auto* arrayRef = builder.CreateInsertValue(llvm::UndefValue::get(targetType),
                                                   elementPtr, 0);
        auto size = llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx),
                                           expr.getType().getPointee().getArraySize());
        return builder.CreateInsertValue(arrayRef, size, 1);
    }

    Type exprType = expr.getType();
    if (exprType.isPtrType()) exprType = exprType.getPointee();

    if (expr.isRvalue() || !exprType.isBasicType())
        return codegenExpr(expr);

    auto it = structs.find(exprType.getName());
    if (it == structs.end() || it->second.second->passByValue()) {
        if (expr.getType().isPtrType() && !targetType->isPointerTy()) {
            return builder.CreateLoad(codegenExpr(expr));
        }
    } else if (!expr.getType().isPtrType()) {
        return codegenLvalueExpr(expr);
    }
    return codegenExpr(expr);
}

llvm::Value* codegenBuiltinConversion(const Expr& expr, Type type) {
    if (expr.getType().isUnsigned() && type.isInteger()) {
        return builder.CreateZExtOrTrunc(codegenExpr(expr), toIR(type));
    } else if (expr.getType().isSigned() && type.isInteger()) {
        return builder.CreateSExtOrTrunc(codegenExpr(expr), toIR(type));
    } else if (expr.getType().isFloatingPoint()) {
        if (type.isSigned()) return builder.CreateFPToSI(codegenExpr(expr), toIR(type));
        if (type.isUnsigned()) return builder.CreateFPToUI(codegenExpr(expr), toIR(type));
        if (type.isFloatingPoint()) return builder.CreateFPCast(codegenExpr(expr), toIR(type));
    } else if (type.isFloatingPoint()) {
        if (expr.getType().isSigned()) return builder.CreateSIToFP(codegenExpr(expr), toIR(type));
        if (expr.getType().isUnsigned()) return builder.CreateUIToFP(codegenExpr(expr), toIR(type));
    }
    error(expr.getSrcLoc(), "conversion from '", expr.getType(), "' to '", type, "' not supported");
}

llvm::Value* codegenCallExpr(const CallExpr& expr) {
    if (expr.isBuiltinConversion())
        return codegenBuiltinConversion(*expr.args.front().value, expr.getType());

    llvm::Function* func = getFuncForCall(expr);
    assert(func);
    auto param = func->arg_begin();
    llvm::SmallVector<llvm::Value*, 16> args;
    if (expr.isMemberFuncCall()) args.emplace_back(codegenExprForPassing(*expr.getReceiver(), param++->getType()));
    for (const auto& arg : expr.args) args.emplace_back(codegenExprForPassing(*arg.value, param++->getType()));

    return builder.CreateCall(func, args);
}

llvm::Value* codegenCastExpr(const CastExpr& expr) {
    auto* value = codegenExpr(*expr.expr);
    auto* type = toIR(expr.type);
    if (value->getType()->isIntegerTy() && type->isIntegerTy()) {
        return builder.CreateIntCast(value, type, expr.expr->getType().isSigned());
    }
    if (expr.expr->getType().isPtrType() && expr.expr->getType().getPointee().isUnsizedArrayType()) {
        value = builder.CreateExtractValue(value, 0);
    }
    return builder.CreateBitOrPointerCast(value, type);
}

llvm::Value* codegenLvalueMemberExpr(const MemberExpr& expr) {
    Type base = expr.base->getType();
    if (base.isPtrType() && base.isRef()) base = base.getPointee();
    if (base.isArrayType() || base.isString()) {
        assert(expr.member == "count");
        if (base.isUnsizedArrayType() || base.isString())
            return builder.CreateExtractValue(codegenExpr(*expr.base), 1, "count");
        else
            return llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), base.getArraySize());
    }

    auto* value = codegenLvalueExpr(*expr.base);
    auto baseType = value->getType();
    if (baseType->isPointerTy()) {
        baseType = baseType->getPointerElementType();
        if (baseType->isPointerTy()) {
            baseType = baseType->getPointerElementType();
            value = builder.CreateLoad(value);
        }
        auto& baseTypeDecl = *structs.find(baseType->getStructName())->second.second;
        auto index = baseTypeDecl.isUnion() ? 0 : baseTypeDecl.getFieldIndex(expr.member);
        auto* gep = builder.CreateStructGEP(nullptr, value, index);
        if (baseTypeDecl.isUnion()) {
            return builder.CreateBitCast(gep, toIR(expr.getType())->getPointerTo(), expr.member);
        }
        return gep;
    } else {
        auto& baseTypeDecl = *structs.find(baseType->getStructName())->second.second;
        auto index = baseTypeDecl.isUnion() ? 0 : baseTypeDecl.getFieldIndex(expr.member);
        return builder.CreateExtractValue(value, index);
    }
}

llvm::Value* codegenMemberExpr(const MemberExpr& expr) {
    auto* value = codegenLvalueMemberExpr(expr);
    return value->getType()->isPointerTy() ? builder.CreateLoad(value) : value;
}

llvm::Value* codegenLvalueSubscriptExpr(const SubscriptExpr& expr) {
    auto* value = codegenLvalueExpr(*expr.array);

    if (expr.array->getType().isPtrType()
    && expr.array->getType().getPointee().isUnsizedArrayType())
        return builder.CreateGEP(builder.CreateExtractValue(value, 0), codegenExpr(*expr.index));

    if (value->getType()->getPointerElementType()->isPointerTy()) value = builder.CreateLoad(value);
    return builder.CreateGEP(value,
                             {llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), 0), codegenExpr(*expr.index)});
}

llvm::Value* codegenSubscriptExpr(const SubscriptExpr& expr) {
    return builder.CreateLoad(codegenLvalueSubscriptExpr(expr));
}

llvm::Value* codegenUnwrapExpr(const UnwrapExpr& expr) {
    // TODO: Assert that the operand is non-null.
    return codegenExpr(*expr.operand);
}

} // anonymous namespace

llvm::Value* irgen::codegenExpr(const Expr& expr) {
    switch (expr.getKind()) {
        case ExprKind::VariableExpr: return codegenVarExpr(expr.getVariableExpr());
        case ExprKind::StrLiteralExpr: return codegenStrLiteralExpr(expr.getStrLiteralExpr());
        case ExprKind::IntLiteralExpr: return codegenIntLiteralExpr(expr.getIntLiteralExpr());
        case ExprKind::FloatLiteralExpr: return codegenFloatLiteralExpr(expr.getFloatLiteralExpr());
        case ExprKind::BoolLiteralExpr: return codegenBoolLiteralExpr(expr.getBoolLiteralExpr());
        case ExprKind::NullLiteralExpr: return codegenNullLiteralExpr(expr.getNullLiteralExpr());
        case ExprKind::ArrayLiteralExpr: return codegenArrayLiteralExpr(expr.getArrayLiteralExpr());
        case ExprKind::PrefixExpr: return codegenPrefixExpr(expr.getPrefixExpr());
        case ExprKind::BinaryExpr: return codegenBinaryExpr(expr.getBinaryExpr());
        case ExprKind::CallExpr: return codegenCallExpr(expr.getCallExpr());
        case ExprKind::CastExpr: return codegenCastExpr(expr.getCastExpr());
        case ExprKind::MemberExpr: return codegenMemberExpr(expr.getMemberExpr());
        case ExprKind::SubscriptExpr: return codegenSubscriptExpr(expr.getSubscriptExpr());
        case ExprKind::UnwrapExpr: return codegenUnwrapExpr(expr.getUnwrapExpr());
    }
    llvm_unreachable("all cases handled");
}

namespace {

llvm::Value* codegenLvalueExpr(const Expr& expr) {
    switch (expr.getKind()) {
        case ExprKind::VariableExpr: return codegenLvalueVarExpr(expr.getVariableExpr());
        case ExprKind::StrLiteralExpr: llvm_unreachable("no lvalue string literals");
        case ExprKind::IntLiteralExpr: llvm_unreachable("no lvalue integer literals");
        case ExprKind::FloatLiteralExpr: llvm_unreachable("no lvalue float literals");
        case ExprKind::BoolLiteralExpr: llvm_unreachable("no lvalue boolean literals");
        case ExprKind::NullLiteralExpr: llvm_unreachable("no lvalue null literals");
        case ExprKind::ArrayLiteralExpr: llvm_unreachable("no lvalue array literals");
        case ExprKind::PrefixExpr: return codegenLvaluePrefixExpr(expr.getPrefixExpr());
        case ExprKind::BinaryExpr: llvm_unreachable("no lvalue binary expressions");
        case ExprKind::CallExpr: llvm_unreachable("IRGen doesn't support lvalue call expressions yet");
        case ExprKind::CastExpr: llvm_unreachable("IRGen doesn't support lvalue cast expressions yet");
        case ExprKind::MemberExpr: return codegenLvalueMemberExpr(expr.getMemberExpr());
        case ExprKind::SubscriptExpr: return codegenLvalueSubscriptExpr(expr.getSubscriptExpr());
        case ExprKind::UnwrapExpr: return codegenUnwrapExpr(expr.getUnwrapExpr());
    }
    llvm_unreachable("all cases handled");
}

void beginScope() {
    scopes.emplace_back();
}

void endScope() {
    scopes.back().onScopeEnd();
    scopes.pop_back();
}

void deferEvaluationOf(const Expr& expr) {
    scopes.back().deferredExprs.emplace_back(&expr);
}

void deferDeinitCall(llvm::Function* deinit, llvm::Value* valueToDeinit) {
    scopes.back().deinitsToCall.emplace_back(deinit, valueToDeinit);
}

void codegenDeferredExprsAndDeinitCallsForReturn() {
    for (auto& scope : llvm::reverse(scopes)) scope.onScopeEnd();
    scopes.back().clear();
}

void codegenReturnStmt(const ReturnStmt& stmt) {
    assert(stmt.values.size() < 2 && "IRGen doesn't support multiple return values yet");

    codegenDeferredExprsAndDeinitCallsForReturn();

    if (stmt.values.empty()) {
        if (currentDecl->getFuncDecl().name != "main") builder.CreateRetVoid();
        else builder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), 0));
    } else {
        builder.CreateRet(codegenExpr(*stmt.values[0]));
    }
}

llvm::AllocaInst* createEntryBlockAlloca(Type type, llvm::Value* arraySize = nullptr,
                                         const llvm::Twine& name = "") {
    auto* insertBlock = builder.GetInsertBlock();
    if (lastAlloca == llvm::BasicBlock::iterator()) {
        if (insertBlock->getParent()->getEntryBlock().empty()) {
            builder.SetInsertPoint(&insertBlock->getParent()->getEntryBlock());
        } else {
            builder.SetInsertPoint(&insertBlock->getParent()->getEntryBlock().front());
        }
    } else {
        builder.SetInsertPoint(&insertBlock->getParent()->getEntryBlock(), std::next(lastAlloca));
    }
    auto* alloca = builder.CreateAlloca(toIR(type), arraySize, name);
    lastAlloca = alloca->getIterator();
    setLocalValue(type, name.str(), alloca);
    builder.SetInsertPoint(insertBlock);
    return alloca;
}

void codegenVarStmt(const VariableStmt& stmt) {
    auto* alloca = createEntryBlockAlloca(stmt.decl->getType(), nullptr, stmt.decl->name);
    if (auto initializer = stmt.decl->initializer) {
        builder.CreateStore(codegenExprForPassing(*initializer, alloca->getAllocatedType()), alloca);
    }
}

void codegenIncrementStmt(const IncrementStmt& stmt) {
    auto* alloca = codegenLvalueExpr(*stmt.operand);
    auto* value = builder.CreateLoad(alloca);
    auto* result = builder.CreateAdd(value, llvm::ConstantInt::get(value->getType(), 1));
    builder.CreateStore(result, alloca);
}

void codegenDecrementStmt(const DecrementStmt& stmt) {
    auto* alloca = codegenLvalueExpr(*stmt.operand);
    auto* value = builder.CreateLoad(alloca);
    auto* result = builder.CreateSub(value, llvm::ConstantInt::get(value->getType(), 1));
    builder.CreateStore(result, alloca);
}

void codegenStmt(const Stmt& stmt);

void codegenBlock(llvm::ArrayRef<std::unique_ptr<Stmt>> stmts,
                  llvm::BasicBlock* destination, llvm::BasicBlock* continuation) {
    builder.SetInsertPoint(destination);

    beginScope();
    for (const auto& stmt : stmts) {
        codegenStmt(*stmt);
        if (stmt->isReturnStmt() || stmt->isBreakStmt()) break;
    }
    endScope();

    llvm::BasicBlock* insertBlock = builder.GetInsertBlock();
    if (insertBlock->empty() || (!llvm::isa<llvm::ReturnInst>(insertBlock->back())
                              && !llvm::isa<llvm::BranchInst>(insertBlock->back())))
        builder.CreateBr(continuation);
}

void codegenIfStmt(const IfStmt& ifStmt) {
    auto* condition = codegenExpr(*ifStmt.condition);
    auto* func = builder.GetInsertBlock()->getParent();
    auto* thenBlock = llvm::BasicBlock::Create(ctx, "then", func);
    auto* elseBlock = llvm::BasicBlock::Create(ctx, "else", func);
    auto* endIfBlock = llvm::BasicBlock::Create(ctx, "endif", func);
    builder.CreateCondBr(condition, thenBlock, elseBlock);
    codegenBlock(ifStmt.thenBody, thenBlock, endIfBlock);
    codegenBlock(ifStmt.elseBody, elseBlock, endIfBlock);
    builder.SetInsertPoint(endIfBlock);
}

void codegenSwitchStmt(const SwitchStmt& switchStmt) {
    auto* condition = codegenExpr(*switchStmt.condition);
    auto* func = builder.GetInsertBlock()->getParent();
    auto* insertBlockBackup = builder.GetInsertBlock();

    std::vector<std::pair<llvm::ConstantInt*, llvm::BasicBlock*>> cases;
    for (const SwitchCase& switchCase : switchStmt.cases) {
        auto* value = llvm::cast<llvm::ConstantInt>(codegenExpr(*switchCase.value));
        auto* block = llvm::BasicBlock::Create(ctx, "", func);
        cases.emplace_back(value, block);
    }

    builder.SetInsertPoint(insertBlockBackup);
    auto* defaultBlock = llvm::BasicBlock::Create(ctx, "default", func);
    auto* end = llvm::BasicBlock::Create(ctx, "endswitch", func);
    breakTargets.push_back(end);
    auto* switchInst = builder.CreateSwitch(condition, defaultBlock);

    auto casesIterator = cases.begin();
    for (const SwitchCase& switchCase : switchStmt.cases) {
        auto* value = casesIterator->first;
        auto* block = casesIterator->second;
        codegenBlock(switchCase.stmts, block, end);
        switchInst->addCase(value, block);
        ++casesIterator;
    }

    codegenBlock(switchStmt.defaultStmts, defaultBlock, end);
    breakTargets.pop_back();
    builder.SetInsertPoint(end);
}

void codegenWhileStmt(const WhileStmt& whileStmt) {
    auto* func = builder.GetInsertBlock()->getParent();
    auto* cond = llvm::BasicBlock::Create(ctx, "while", func);
    auto* body = llvm::BasicBlock::Create(ctx, "body", func);
    auto* end = llvm::BasicBlock::Create(ctx, "endwhile", func);
    breakTargets.push_back(end);
    builder.CreateBr(cond);

    builder.SetInsertPoint(cond);
    builder.CreateCondBr(codegenExpr(*whileStmt.condition), body, end);
    codegenBlock(whileStmt.body, body, cond);

    breakTargets.pop_back();
    builder.SetInsertPoint(end);
}

// This transforms 'for (id in x...y) { ... }' (where 'x' and 'y' are integers) into:
//
//  var counter = x;
//  while (counter <= y) {
//      const id = counter;
//      ...
//      counter++;
//  }
void codegenForStmt(const ForStmt& forStmt) {
    if (!forStmt.range->getType().isRangeType())
        error(forStmt.range->getSrcLoc(),
              "IRGen doesn't support 'for'-loops over non-range iterables yet");

    if (!forStmt.range->getType().getIterableElementType().isInteger())
        error(forStmt.range->getSrcLoc(),
              "IRGen doesn't support 'for'-loops over non-integer ranges yet");

    beginScope();
    auto& range = forStmt.range->getBinaryExpr();
    auto* counterAlloca = createEntryBlockAlloca(forStmt.range->getType().getIterableElementType(),
                                                 nullptr, forStmt.id);
    builder.CreateStore(codegenExpr(range.getLHS()), counterAlloca);
    auto* lastValue = codegenExpr(range.getRHS());

    auto* func = builder.GetInsertBlock()->getParent();
    auto* cond = llvm::BasicBlock::Create(ctx, "for", func);
    auto* body = llvm::BasicBlock::Create(ctx, "body", func);
    auto* end = llvm::BasicBlock::Create(ctx, "endfor", func);
    breakTargets.push_back(end);
    builder.CreateBr(cond);

    builder.SetInsertPoint(cond);
    auto* counter = builder.CreateLoad(counterAlloca, forStmt.id);

    llvm::Value* cmp;
    if (llvm::cast<RangeType>(*forStmt.range->getType()).isExclusive()) {
        if (range.getLHS().getType().isSigned())
            cmp = builder.CreateICmpSLT(counter, lastValue);
        else
            cmp = builder.CreateICmpULT(counter, lastValue);
    } else {
        if (range.getLHS().getType().isSigned())
            cmp = builder.CreateICmpSLE(counter, lastValue);
        else
            cmp = builder.CreateICmpULE(counter, lastValue);
    }
    builder.CreateCondBr(cmp, body, end);

    codegenBlock(forStmt.body, body, cond);

    builder.SetInsertPoint(&builder.GetInsertBlock()->back());
    auto* newCounter = builder.CreateAdd(counter, llvm::ConstantInt::get(counter->getType(), 1));
    builder.CreateStore(newCounter, counterAlloca);

    breakTargets.pop_back();
    builder.SetInsertPoint(end);
    endScope();
}

void codegenBreakStmt(const BreakStmt&) {
    assert(!breakTargets.empty());
    builder.CreateBr(breakTargets.back());
}

void codegenAssignStmt(const AssignStmt& stmt) {
    auto* lhs = codegenLvalueExpr(*stmt.lhs);
    builder.CreateStore(codegenExpr(*stmt.rhs), lhs);
}

void codegenAugAssignStmt(const AugAssignStmt& stmt) {
    switch (stmt.op) {
        case AND_AND: fatalError("'&&=' not implemented yet");
        case OR_OR:   fatalError("'||=' not implemented yet");
        default:      break;
    }
    auto* lhs = codegenLvalueExpr(*stmt.lhs);
    auto* rhs = codegenExpr(*stmt.rhs);
    auto* result = codegenBinaryOp(stmt.op, builder.CreateLoad(lhs), rhs, *stmt.lhs);
    builder.CreateStore(result, lhs);
}

void codegenStmt(const Stmt& stmt) {
    switch (stmt.getKind()) {
        case StmtKind::ReturnStmt: codegenReturnStmt(stmt.getReturnStmt()); break;
        case StmtKind::VariableStmt: codegenVarStmt(stmt.getVariableStmt()); break;
        case StmtKind::IncrementStmt: codegenIncrementStmt(stmt.getIncrementStmt()); break;
        case StmtKind::DecrementStmt: codegenDecrementStmt(stmt.getDecrementStmt()); break;
        case StmtKind::ExprStmt: codegenExpr(*stmt.getExprStmt().expr); break;
        case StmtKind::DeferStmt: deferEvaluationOf(*stmt.getDeferStmt().expr); break;
        case StmtKind::IfStmt: codegenIfStmt(stmt.getIfStmt()); break;
        case StmtKind::SwitchStmt: codegenSwitchStmt(stmt.getSwitchStmt()); break;
        case StmtKind::WhileStmt: codegenWhileStmt(stmt.getWhileStmt()); break;
        case StmtKind::ForStmt: codegenForStmt(stmt.getForStmt()); break;
        case StmtKind::BreakStmt: codegenBreakStmt(stmt.getBreakStmt()); break;
        case StmtKind::AssignStmt: codegenAssignStmt(stmt.getAssignStmt()); break;
        case StmtKind::AugAssignStmt: codegenAugAssignStmt(stmt.getAugAssignStmt()); break;
    }
}

void createDeinitCall(llvm::Function* deinit, llvm::Value* valueToDeinit) {
    // Prevent recursively destroying the argument in struct deinitializers.
    if (llvm::isa<llvm::Argument>(valueToDeinit)
        && builder.GetInsertBlock()->getParent()->getName().endswith(".deinit")) return;

    if (valueToDeinit->getType()->isPointerTy() && !deinit->arg_begin()->getType()->isPointerTy()) {
        builder.CreateCall(deinit, builder.CreateLoad(valueToDeinit));
    } else if (!valueToDeinit->getType()->isPointerTy() && deinit->arg_begin()->getType()->isPointerTy()) {
        llvm::errs() << "deinitialization of by-value class parameters not implemented yet\n";
        abort();
    } else {
        builder.CreateCall(deinit, valueToDeinit);
    }
}

llvm::Type* getLLVMTypeForPassing(llvm::StringRef typeName, bool isMutating) {
    assert(structs.count(typeName));
    auto structTypeAndDecl = structs.find(typeName)->second;

    if (!isMutating && structTypeAndDecl.second->passByValue()) {
        return structTypeAndDecl.first;
    } else {
        return llvm::PointerType::get(structTypeAndDecl.first, 0);
    }
}

void setCurrentGenericArgs(llvm::ArrayRef<GenericParamDecl> genericParams,
                           llvm::ArrayRef<Type> genericArgs) {
    for (auto tuple : llvm::zip_first(genericParams, genericArgs)) {
        currentGenericArgs.emplace(std::get<0>(tuple).name, toIR(std::get<1>(tuple)));
    }
}

llvm::Function* getFuncProto(const FuncDecl& decl, llvm::ArrayRef<Type> funcGenericArgs = {},
                             Expr* receiver = nullptr, std::string&& mangledName = {}) {
    std::vector<Type> receiverTypeGenericArgs;
    if (receiver) {
        receiverTypeGenericArgs = llvm::cast<BasicType>(*receiver->getType().removePtr()).getGenericArgs();
    }

    auto it = funcInstantiations.find(mangleWithParams(decl, receiverTypeGenericArgs, funcGenericArgs));
    if (it != funcInstantiations.end()) return it->second.func;

    setCurrentGenericArgs(decl.genericParams, funcGenericArgs);

    auto* funcType = decl.getFuncType();

    assert(!funcType->returnType.isTupleType() && "IRGen doesn't support tuple return values yet");
    auto* returnType = toIR(funcType->returnType);
    if (decl.name == "main" && returnType->isVoidTy()) returnType = llvm::Type::getInt32Ty(ctx);

    llvm::SmallVector<llvm::Type*, 16> paramTypes;

    if (decl.isMemberFunc()) {
        auto& receiverTypeDecl = findDecl(decl.getReceiverTypeName(), SrcLoc::invalid()).getTypeDecl();
        std::string receiverTypeName;

        if (receiverTypeDecl.isGeneric()) {
            receiverTypeName = mangle(receiverTypeDecl, receiverTypeGenericArgs);
            if (structs.count(receiverTypeName) == 0) {
                codegenGenericTypeInstantiation(receiverTypeDecl, receiverTypeGenericArgs);
            }
            mangledName = mangle(decl, receiverTypeGenericArgs, funcGenericArgs);
        } else {
            receiverTypeName = decl.getReceiverTypeName();
        }
        paramTypes.emplace_back(getLLVMTypeForPassing(receiverTypeName, decl.isMutating()));
    }

    for (const auto& t : funcType->paramTypes) paramTypes.emplace_back(toIR(t));

    auto* llvmFuncType = llvm::FunctionType::get(returnType, paramTypes, false);
    if (mangledName.empty()) mangledName = mangle(decl, receiverTypeGenericArgs, funcGenericArgs);
    auto* func = llvm::Function::Create(llvmFuncType, llvm::Function::ExternalLinkage,
                                        mangledName, &module);

    auto arg = func->arg_begin(), argsEnd = func->arg_end();
    if (decl.isMemberFunc()) arg++->setName("this");
    for (auto param = decl.params.begin(); arg != argsEnd; ++param, ++arg) arg->setName(param->name);

    currentGenericArgs.clear();

    auto mangled = mangleWithParams(decl, receiverTypeGenericArgs, funcGenericArgs);
    FuncInstantiation funcInstantiation{decl, funcGenericArgs, func};
    return funcInstantiations.emplace(std::move(mangled), std::move(funcInstantiation)).first->second.func;
}

llvm::Function* getInitProto(const InitDecl& decl, llvm::ArrayRef<Type> typeGenericArgs = {},
                             llvm::ArrayRef<Type> funcGenericArgs = {}) {
    auto it = funcInstantiations.find(mangleWithParams(decl, typeGenericArgs, funcGenericArgs));
    if (it != funcInstantiations.end()) return it->second.func;

    FuncDecl funcDecl(mangle(decl, typeGenericArgs), std::vector<ParamDecl>(decl.params),
                      decl.getTypeDecl().getType(typeGenericArgs), "",
                      /* genericParams */ {}, nullptr, decl.srcLoc);
    return getFuncProto(funcDecl, funcGenericArgs, nullptr);
}

llvm::Function* codegenDeinitializerProto(const DeinitDecl& decl) {
    FuncDecl funcDecl("deinit", {}, Type::getVoid(), decl.getTypeName().str(),
                      /* genericParams */ {}, nullptr, decl.srcLoc);
    return getFuncProto(funcDecl);
}

llvm::Function* getFuncForCall(const CallExpr& call) {
    if (!call.callsNamedFunc()) fatalError("anonymous function calls not implemented yet");

    const Decl& decl = *call.getCalleeDecl();

    if (auto* funcDecl = llvm::dyn_cast<FuncDecl>(&decl)) {
        return getFuncProto(*funcDecl, call.genericArgs, call.getReceiver());
    } else if (auto* initDecl = llvm::dyn_cast<InitDecl>(&decl)) {
        llvm::Function* func = getInitProto(*initDecl, call.genericArgs);
        if (func->empty() && !call.genericArgs.empty()) {
            auto backup = builder.GetInsertBlock();
            codegenInitDecl(*initDecl, call.genericArgs);
            builder.SetInsertPoint(backup);
        }
        return func;
    } else {
        llvm_unreachable("invalid callee decl");
    }
}

void codegenFuncBody(const FuncDecl& decl, llvm::Function& func) {
    lastAlloca = llvm::BasicBlock::iterator();
    builder.SetInsertPoint(llvm::BasicBlock::Create(ctx, "", &func));
    beginScope();
    auto arg = func.arg_begin();
    if (decl.isMemberFunc()) setLocalValue(nullptr, "this", &*arg++);
    for (const auto& param : decl.params) setLocalValue(param.type, param.name, &*arg++);
    for (const auto& stmt : *decl.body) codegenStmt(*stmt);
    endScope();

    auto* insertBlock = builder.GetInsertBlock();
    if (insertBlock != &func.getEntryBlock() && llvm::pred_empty(insertBlock)) {
        insertBlock->eraseFromParent();
        return;
    }

    if (insertBlock->empty() || !llvm::isa<llvm::ReturnInst>(insertBlock->back())) {
        if (func.getName() != "main") {
            builder.CreateRetVoid();
        } else {
            builder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), 0));
        }
    }
}

void codegenFuncDecl(const FuncDecl& decl) {
    if (decl.isGeneric()) return;

    if (!decl.getReceiverTypeName().empty()) {
        auto& receiverTypeDecl = findDecl(decl.getReceiverTypeName(), SrcLoc::invalid()).getTypeDecl();
        if (receiverTypeDecl.isGeneric()) {
            return;
        }
    }

    llvm::Function* func = getFuncProto(decl);
    if (!decl.isExtern()) codegenFuncBody(decl, *func);
    assert(!llvm::verifyFunction(*func, &llvm::errs()));
}

void codegenInitDecl(const InitDecl& decl, llvm::ArrayRef<Type> typeGenericArgs) {
    if (decl.getTypeDecl().isGeneric() && typeGenericArgs.empty()) return;

    llvm::Function* func = getInitProto(decl, typeGenericArgs);

    builder.SetInsertPoint(llvm::BasicBlock::Create(ctx, "", func));

    auto* type = llvm::cast<llvm::StructType>(toIR(decl.getTypeDecl().getType(typeGenericArgs)));
    auto* alloca = builder.CreateAlloca(type);

    beginScope();
    setLocalValue(nullptr, "this", alloca);
    auto param = decl.params.begin();
    for (auto& arg : func->args()) setLocalValue(param++->type, arg.getName(), &arg);
    for (const auto& stmt : *decl.body) codegenStmt(*stmt);
    builder.CreateRet(builder.CreateLoad(alloca));
    endScope();

    assert(!llvm::verifyFunction(*func, &llvm::errs()));
}

void codegenDeinitDecl(const DeinitDecl& decl) {
    if (decl.getTypeDecl().isGeneric()) return;

    FuncDecl funcDecl("deinit", {}, Type::getVoid(), decl.getTypeName(),
                      /* genericParams */ {}, nullptr, decl.srcLoc);
    funcDecl.body = decl.body;

    llvm::Function* func = getFuncProto(funcDecl);
    codegenFuncBody(funcDecl, *func);
    assert(!llvm::verifyFunction(*func, &llvm::errs()));
}

void codegenTypeDecl(const TypeDecl& decl) {
    if (decl.isGeneric()) return;

    if (decl.fields.empty()) {
        structs.emplace(decl.name, std::make_pair(llvm::StructType::get(ctx), &decl));
        return;
    }

    auto elements = map(decl.fields, *[](const FieldDecl& f) { return toIR(f.type); });
    structs.emplace(decl.name, std::make_pair(llvm::StructType::create(elements, decl.name), &decl));
}

llvm::Type* codegenGenericTypeInstantiation(const TypeDecl& decl, llvm::ArrayRef<Type> genericArgs) {
    auto name = mangle(decl, genericArgs);

    if (decl.fields.empty()) {
        auto value = std::make_pair(llvm::StructType::get(ctx), &decl);
        return structs.emplace(name, std::move(value)).first->second.first;
    }

    setCurrentGenericArgs(decl.genericParams, genericArgs);
    auto elements = map(decl.fields, *[](const FieldDecl& f) { return toIR(f.type); });
    currentGenericArgs.clear();

    auto value = std::make_pair(llvm::StructType::create(elements, name), &decl);
    return structs.emplace(name, std::move(value)).first->second.first;
}

void codegenVarDecl(const VarDecl& decl) {
    assert(decl.initializer && "global variables must have initializers");
    if (globalScope().localValues.find(decl.name) != globalScope().localValues.end()) return;

    llvm::Value* value = codegenExpr(*decl.initializer);

    if (decl.getType().isMutable() /* || decl.isPublic() */) {
        value = new llvm::GlobalVariable(module, toIR(decl.getType()),
                                         !decl.getType().isMutable(),
                                         llvm::GlobalValue::PrivateLinkage,
                                         llvm::cast<llvm::Constant>(value),
                                         decl.name);
    }

    globalScope().localValues.emplace(decl.name, value);
}

void codegenDecl(const Decl& decl) {
    switch (decl.getKind()) {
        case DeclKind::ParamDecl: llvm_unreachable("handled via FuncDecl");
        case DeclKind::FuncDecl: codegenFuncDecl(decl.getFuncDecl()); break;
        case DeclKind::GenericParamDecl: llvm_unreachable("cannot codegen generic parameter declaration");
        case DeclKind::InitDecl: codegenInitDecl(decl.getInitDecl()); break;
        case DeclKind::DeinitDecl: codegenDeinitDecl(decl.getDeinitDecl()); break;
        case DeclKind::TypeDecl: codegenTypeDecl(decl.getTypeDecl()); break;
        case DeclKind::VarDecl: codegenVarDecl(decl.getVarDecl()); break;
        case DeclKind::FieldDecl: llvm_unreachable("handled via TypeDecl");
        case DeclKind::ImportDecl: break;
    }
}

} // anonymous namespace

llvm::Module& irgen::compile(const Module& sourceModule) {
    for (const auto& sourceFile : sourceModule.getSourceFiles()) {
        for (const auto& decl : sourceFile.getTopLevelDecls()) {
            currentDecl = decl.get();
            codegenDecl(*decl);
        }
    }

    for (auto& p : funcInstantiations) {
        if (p.second.decl.isExtern() || !p.second.func->empty()) continue;

        setCurrentGenericArgs(p.second.decl.genericParams, p.second.genericArgs);
        codegenFuncBody(p.second.decl, *p.second.func);
        currentGenericArgs.clear();
        assert(!llvm::verifyFunction(*p.second.func, &llvm::errs()));
    }

    assert(!llvm::verifyModule(module, &llvm::errs()));
    return module;
}

llvm::Module& irgen::getIRModule() {
    return module;
}

llvm::IRBuilder<>& irgen::getBuilder() {
    return builder;
}

llvm::LLVMContext& irgen::getContext() {
    return ctx;
}
