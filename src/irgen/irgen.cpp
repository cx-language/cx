#include <unordered_map>
#include <vector>
#include <cassert>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Verifier.h>
#include "irgen.h"
#include "../sema/typecheck.h"
#include "../parser/parser.hpp"

using namespace delta;

namespace {

llvm::Value* codegen(const Expr& expr);
llvm::Value* codegenLvalue(const Expr& expr);
void deferDeinitCallOf(llvm::Value* value);
void createDeinitCall(llvm::Value* valueToDeinit);

struct Scope {
    llvm::SmallVector<const Expr*, 8> deferredExprs;
    llvm::SmallVector<llvm::Value*, 8> valuesToDeinit;

    void onScopeEnd() {
        for (const Expr* expr : llvm::reverse(deferredExprs)) codegen(*expr);
        for (llvm::Value* value : llvm::reverse(valuesToDeinit)) createDeinitCall(value);
    }

    void clear() {
        deferredExprs.clear();
        valuesToDeinit.clear();
    }
};

llvm::LLVMContext ctx;
llvm::IRBuilder<> builder(ctx);
llvm::Module module("", ctx);
std::unordered_map<std::string, llvm::Value*> globalValues;
std::unordered_map<std::string, llvm::Value*> localValues;
std::unordered_map<std::string, llvm::Function*> funcs;
std::unordered_map<std::string, std::pair<llvm::StructType*, const TypeDecl*>> structs;
const std::vector<Decl>* globalDecls;
const Decl* currentDecl;
llvm::SmallVector<Scope, 4> scopes;
llvm::BasicBlock::iterator lastAlloca;

void setLocalValue(std::string name, llvm::Value* value) {
    bool wasInserted = localValues.emplace(std::move(name), value).second;
    assert(wasInserted);

    if (name != "this") {
        auto* alloca = llvm::dyn_cast<llvm::AllocaInst>(value);
        auto* typeToDeinit = alloca ? alloca->getAllocatedType() : value->getType();
        if (typeToDeinit->isStructTy()) {
            llvm::StringRef typeName = typeToDeinit->getStructName();
            llvm::Function* deinit = module.getFunction(("__deinit_" + typeName).str());
            if (deinit) deferDeinitCallOf(value);
        }
    }
}

void codegen(const Decl& decl);

llvm::Value* findValue(llvm::StringRef name) {
    auto it = localValues.find(name);
    if (it == localValues.end()) {
        it = globalValues.find(name);

        // FIXME: It would probably be better to not access the symbol table here.
        if (it == globalValues.end()) {
            codegen(findInSymbolTable(name, SrcLoc::invalid()));
            it = globalValues.find(name);
            assert(it != globalValues.end());
        }
    }
    return it->second;
}

template<typename From, typename To>
std::vector<To> map(const std::vector<From>& from, To (&func)(const From&)) {
    std::vector<To> to;
    to.reserve(from.size());
    for (const auto& e : from) to.emplace_back(func(e));
    return to;
}

void codegen(const TypeDecl& decl);

llvm::Type* toIR(const Type& type) {
    switch (type.getKind()) {
        case TypeKind::BasicType: {
            const auto& name = type.getBasicType().name;
            if (name == "void") return llvm::Type::getVoidTy(ctx);
            if (name == "bool") return llvm::Type::getInt1Ty(ctx);
            if (name == "char") return llvm::Type::getInt8Ty(ctx);
            if (name == "int" || name == "uint") return llvm::Type::getInt32Ty(ctx);
            if (name == "int8" || name == "uint8") return llvm::Type::getInt8Ty(ctx);
            if (name == "int16" || name == "uint16") return llvm::Type::getInt16Ty(ctx);
            if (name == "int32" || name == "uint32") return llvm::Type::getInt32Ty(ctx);
            if (name == "int64" || name == "uint64") return llvm::Type::getInt64Ty(ctx);
            auto it = structs.find(name);
            if (it == structs.end()) {
                // Custom type that has not been declared yet, search for it in the symbol table.
                codegen(findInSymbolTable(name, SrcLoc::invalid()).getTypeDecl());
                it = structs.find(name);
            }
            return it->second.first;
        }
        case TypeKind::ArrayType: {
            const auto& array = type.getArrayType();
            return llvm::ArrayType::get(toIR(*array.elementType), array.size);
        }
        case TypeKind::TupleType:
            assert(false && "IRGen doesn't support tuple types yet");
        case TypeKind::FuncType:
            assert(false && "IRGen doesn't support function types yet");
        case TypeKind::PtrType: {
            auto* pointeeType = toIR(*type.getPtrType().pointeeType);
            if (!pointeeType->isVoidTy()) return llvm::PointerType::get(pointeeType, 0);
            else return llvm::PointerType::get(llvm::Type::getInt8Ty(ctx), 0);
        }
    }
}

llvm::Value* codegen(const VariableExpr& expr) {
    auto* value = findValue(expr.identifier);
    if (auto* arg = llvm::dyn_cast<llvm::Argument>(value)) return arg;
    return builder.CreateLoad(value, expr.identifier);
}

llvm::Value* codegenLvalue(const VariableExpr& expr) {
    return findValue(expr.identifier);
}

llvm::Value* codegen(const StrLiteralExpr& expr, const Expr& parent) {
    if (parent.getType().getPtrType().pointeeType->isArrayType()) {
        return builder.CreateGlobalString(expr.value);
    } else {
        // Passing as C-string, i.e. char pointer.
        return builder.CreateGlobalStringPtr(expr.value);
    }
}

llvm::Value* codegen(const IntLiteralExpr& expr, const Expr& parent) {
    return llvm::ConstantInt::getSigned(toIR(parent.getType()), expr.value);
}

llvm::Value* codegen(const BoolLiteralExpr& expr) {
    return expr.value ? llvm::ConstantInt::getTrue(ctx) : llvm::ConstantInt::getFalse(ctx);
}

llvm::Value* codegen(const NullLiteralExpr& expr, const Expr& parent) {
    return llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(toIR(parent.getType())));
}

using CreateNegFunc       = decltype(&llvm::IRBuilder<>::CreateNeg);
using CreateICmpFunc      = decltype(&llvm::IRBuilder<>::CreateICmpEQ);
using CreateAddSubMulFunc = decltype(&llvm::IRBuilder<>::CreateAdd);
using CreateDivFunc       = decltype(&llvm::IRBuilder<>::CreateSDiv);

llvm::Value* codegenPrefixOp(const PrefixExpr& expr, CreateNegFunc intFunc) {
    return (builder.*intFunc)(codegen(*expr.operand), "", false, false);
}

llvm::Value* codegenNot(const PrefixExpr& expr) {
    return builder.CreateNot(codegen(*expr.operand), "");
}

llvm::Value* codegen(const PrefixExpr& expr) {
    switch (expr.op.rawValue) {
        case PLUS:  return codegen(*expr.operand);
        case MINUS: return codegenPrefixOp(expr, &llvm::IRBuilder<>::CreateNeg);
        case STAR:  return builder.CreateLoad(codegen(*expr.operand));
        case AND:   return codegenLvalue(*expr.operand);
        case NOT:   return codegenNot(expr);
        case COMPL: return codegenNot(expr);
        default:    assert(false);
    }
}

llvm::Value* codegenLvalue(const PrefixExpr& expr) {
    switch (expr.op.rawValue) {
        case STAR:  return codegen(*expr.operand);
        default:    assert(false);
    }
}

llvm::Value* codegenBinaryOp(const BinaryExpr& expr, CreateICmpFunc intFunc) {
    llvm::Value* lhs = codegen(*expr.left);
    llvm::Value* rhs = codegen(*expr.right);
    return (builder.*intFunc)(lhs, rhs, "");
}

llvm::Value* codegenBinaryOp(const BinaryExpr& expr, CreateAddSubMulFunc intFunc) {
    llvm::Value* lhs = codegen(*expr.left);
    llvm::Value* rhs = codegen(*expr.right);
    return (builder.*intFunc)(lhs, rhs, "", false, false);
}

llvm::Value* codegenBinaryOp(const BinaryExpr& expr, CreateDivFunc intFunc) {
    llvm::Value* lhs = codegen(*expr.left);
    llvm::Value* rhs = codegen(*expr.right);
    return (builder.*intFunc)(lhs, rhs, "", false);
}

llvm::Value* codegenLogicalAnd(const BinaryExpr& expr) {
    auto* lhsBlock = builder.GetInsertBlock();
    auto* rhsBlock = llvm::BasicBlock::Create(ctx, "andRHS", builder.GetInsertBlock()->getParent());
    auto* endBlock = llvm::BasicBlock::Create(ctx, "andEnd", builder.GetInsertBlock()->getParent());

    llvm::Value* lhs = codegen(*expr.left);
    builder.CreateCondBr(lhs, rhsBlock, endBlock);

    builder.SetInsertPoint(rhsBlock);
    llvm::Value* rhs = codegen(*expr.right);
    builder.CreateBr(endBlock);

    builder.SetInsertPoint(endBlock);
    llvm::PHINode* phi = builder.CreatePHI(llvm::IntegerType::getInt1Ty(ctx), 2, "and");
    phi->addIncoming(lhs, lhsBlock);
    phi->addIncoming(rhs, rhsBlock);
    return phi;
}

llvm::Value* codegenLogicalOr(const BinaryExpr& expr) {
    auto* lhsBlock = builder.GetInsertBlock();
    auto* rhsBlock = llvm::BasicBlock::Create(ctx, "orRHS", builder.GetInsertBlock()->getParent());
    auto* endBlock = llvm::BasicBlock::Create(ctx, "orEnd", builder.GetInsertBlock()->getParent());

    llvm::Value* lhs = codegen(*expr.left);
    builder.CreateCondBr(lhs, endBlock, rhsBlock);

    builder.SetInsertPoint(rhsBlock);
    llvm::Value* rhs = codegen(*expr.right);
    builder.CreateBr(endBlock);

    builder.SetInsertPoint(endBlock);
    llvm::PHINode* phi = builder.CreatePHI(llvm::IntegerType::getInt1Ty(ctx), 2, "or");
    phi->addIncoming(lhs, lhsBlock);
    phi->addIncoming(rhs, rhsBlock);
    return phi;
}

llvm::Value* codegen(const BinaryExpr& expr) {
    assert(expr.left->getType().isImplicitlyConvertibleTo(expr.right->getType())
        || expr.right->getType().isImplicitlyConvertibleTo(expr.left->getType()));

    switch (expr.op.rawValue) {
        case EQ:    return codegenBinaryOp(expr, &llvm::IRBuilder<>::CreateICmpEQ);
        case NE:    return codegenBinaryOp(expr, &llvm::IRBuilder<>::CreateICmpNE);
        case LT:    return codegenBinaryOp(expr, &llvm::IRBuilder<>::CreateICmpSLT);
        case LE:    return codegenBinaryOp(expr, &llvm::IRBuilder<>::CreateICmpSLE);
        case GT:    return codegenBinaryOp(expr, &llvm::IRBuilder<>::CreateICmpSGT);
        case GE:    return codegenBinaryOp(expr, &llvm::IRBuilder<>::CreateICmpSGE);
        case PLUS:  return codegenBinaryOp(expr, &llvm::IRBuilder<>::CreateAdd);
        case MINUS: return codegenBinaryOp(expr, &llvm::IRBuilder<>::CreateSub);
        case STAR:  return codegenBinaryOp(expr, &llvm::IRBuilder<>::CreateMul);
        case SLASH: return codegenBinaryOp(expr, &llvm::IRBuilder<>::CreateSDiv);
        case AND:   return codegenBinaryOp(expr, &llvm::IRBuilder<>::CreateAnd);
        case OR:    return codegenBinaryOp(expr, &llvm::IRBuilder<>::CreateOr);
        case XOR:   return codegenBinaryOp(expr, &llvm::IRBuilder<>::CreateXor);
        case LSHIFT:return codegenBinaryOp(expr, &llvm::IRBuilder<>::CreateShl);
        case RSHIFT:
            if (expr.left->getType().isSigned())
                return codegenBinaryOp(expr, &llvm::IRBuilder<>::CreateAShr);
            else
                return codegenBinaryOp(expr, &llvm::IRBuilder<>::CreateLShr);
        case AND_AND: return codegenLogicalAnd(expr);
        case OR_OR:   return codegenLogicalOr(expr);
        default: assert(false);
    }
}

llvm::Function* getFunc(llvm::StringRef name);

llvm::Value* codegenForPassing(const Expr& expr, llvm::Type* targetType = nullptr) {
    if (expr.isRvalue() || expr.isStrLiteralExpr()) return codegen(expr);
    const Type* exprType = &expr.getType();
    if (exprType->isPtrType()) exprType = exprType->getPtrType().pointeeType.get();

    auto it = structs.find(exprType->getBasicType().name);
    if (it == structs.end() || it->second.second->passByValue()) {
        if (expr.getType().isPtrType() && !targetType->isPointerTy()) {
            return builder.CreateLoad(codegen(expr));
        }
    } else if (!expr.getType().isPtrType()) {
        return codegenLvalue(expr);
    }
    return codegen(expr);
}

llvm::Value* codegen(const CallExpr& expr) {
    llvm::Function* func;
    if (expr.isInitializerCall) {
        func = module.getFunction("__init_" + expr.funcName);
    } else {
        func = getFunc(expr.funcName);
    }

    auto param = func->arg_begin();
    llvm::SmallVector<llvm::Value*, 16> args;
    if (expr.isMemberFuncCall()) args.emplace_back(codegenForPassing(*expr.receiver, param++->getType()));
    for (const auto& arg : expr.args) args.emplace_back(codegenForPassing(*arg.value, param++->getType()));

    return builder.CreateCall(func, args);
}

llvm::Value* codegen(const CastExpr& expr) {
    auto* value = codegen(*expr.expr);
    auto* type = toIR(expr.type);
    if (value->getType()->isIntegerTy() && type->isIntegerTy()) {
        return builder.CreateIntCast(value, type, expr.expr->getType().isSigned());
    }
    return builder.CreateBitOrPointerCast(value, type);
}

llvm::Value* codegenLvalue(const MemberExpr& expr) {
    auto* value = findValue(expr.base);
    auto baseType = value->getType();
    if (baseType->isPointerTy()) {
        baseType = baseType->getPointerElementType();
        if (baseType->isPointerTy()) {
            baseType = baseType->getPointerElementType();
            value = builder.CreateLoad(value);
        }
        auto index = structs.find(baseType->getStructName())->second.second->getFieldIndex(expr.member);
        return builder.CreateStructGEP(nullptr, value, index);
    } else {
        auto index = structs.find(baseType->getStructName())->second.second->getFieldIndex(expr.member);
        return builder.CreateExtractValue(value, index);
    }
}

llvm::Value* codegen(const MemberExpr& expr) {
    auto* value = codegenLvalue(expr);
    return value->getType()->isPointerTy() ? builder.CreateLoad(value) : value;
}

llvm::Value* codegenLvalue(const SubscriptExpr& expr) {
    auto* value = codegenLvalue(*expr.array);
    if (value->getType()->getPointerElementType()->isPointerTy()) value = builder.CreateLoad(value);
    return builder.CreateGEP(value,
                             {llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), 0), codegen(*expr.index)});
}

llvm::Value* codegen(const SubscriptExpr& expr) {
    return builder.CreateLoad(codegenLvalue(expr));
}

llvm::Value* codegen(const Expr& expr) {
    switch (expr.getKind()) {
        case ExprKind::VariableExpr:    return codegen(expr.getVariableExpr());
        case ExprKind::StrLiteralExpr:  return codegen(expr.getStrLiteralExpr(), expr);
        case ExprKind::IntLiteralExpr:  return codegen(expr.getIntLiteralExpr(), expr);
        case ExprKind::BoolLiteralExpr: return codegen(expr.getBoolLiteralExpr());
        case ExprKind::NullLiteralExpr: return codegen(expr.getNullLiteralExpr(), expr);
        case ExprKind::PrefixExpr:      return codegen(expr.getPrefixExpr());
        case ExprKind::BinaryExpr:      return codegen(expr.getBinaryExpr());
        case ExprKind::CallExpr:        return codegen(expr.getCallExpr());
        case ExprKind::CastExpr:        return codegen(expr.getCastExpr());
        case ExprKind::MemberExpr:      return codegen(expr.getMemberExpr());
        case ExprKind::SubscriptExpr:   return codegen(expr.getSubscriptExpr());
    }
}

llvm::Value* codegenLvalue(const Expr& expr) {
    switch (expr.getKind()) {
        case ExprKind::VariableExpr:    return codegenLvalue(expr.getVariableExpr());
        case ExprKind::StrLiteralExpr:  assert(false);
        case ExprKind::IntLiteralExpr:  assert(false);
        case ExprKind::BoolLiteralExpr: assert(false);
        case ExprKind::NullLiteralExpr: assert(false);
        case ExprKind::PrefixExpr:      return codegenLvalue(expr.getPrefixExpr());
        case ExprKind::BinaryExpr:      assert(false);
        case ExprKind::CallExpr:        assert(false && "IRGen doesn't support lvalue call expressions yet");
        case ExprKind::CastExpr:        assert(false && "IRGen doesn't support lvalue cast expressions yet");
        case ExprKind::MemberExpr:      return codegenLvalue(expr.getMemberExpr());
        case ExprKind::SubscriptExpr:   return codegenLvalue(expr.getSubscriptExpr());
    }
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

void deferDeinitCallOf(llvm::Value* value) {
    scopes.back().valuesToDeinit.emplace_back(value);
}

void codegenDeferredExprsAndDeinitCallsForReturn() {
    for (auto& scope : llvm::reverse(scopes)) scope.onScopeEnd();
    scopes.back().clear();
}

void codegen(const ReturnStmt& stmt) {
    assert(stmt.values.size() < 2 && "IRGen doesn't support multiple return values yet");

    codegenDeferredExprsAndDeinitCallsForReturn();

    if (stmt.values.empty()) {
        if (currentDecl->getFuncDecl().name != "main") builder.CreateRetVoid();
        else builder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), 0));
    } else {
        builder.CreateRet(codegen(stmt.values[0]));
    }
}

llvm::AllocaInst* createEntryBlockAlloca(llvm::Type* type, llvm::Value* arraySize = nullptr,
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
    auto* alloca = builder.CreateAlloca(type, arraySize, name);
    lastAlloca = alloca->getIterator();
    setLocalValue(name.str(), alloca);
    builder.SetInsertPoint(insertBlock);
    return alloca;
}

void codegen(const VariableStmt& stmt) {
    auto* alloca = createEntryBlockAlloca(toIR(stmt.decl->getType()), nullptr, stmt.decl->name);

    if (auto initializer = stmt.decl->initializer) {
        builder.CreateStore(codegenForPassing(*initializer), alloca);
    }
}

void codegen(const IncrementStmt& stmt) {
    auto* alloca = codegenLvalue(stmt.operand);
    auto* value = builder.CreateLoad(alloca);
    auto* result = builder.CreateAdd(value, llvm::ConstantInt::get(value->getType(), 1));
    builder.CreateStore(result, alloca);
}

void codegen(const DecrementStmt& stmt) {
    auto* alloca = codegenLvalue(stmt.operand);
    auto* value = builder.CreateLoad(alloca);
    auto* result = builder.CreateSub(value, llvm::ConstantInt::get(value->getType(), 1));
    builder.CreateStore(result, alloca);
}

void codegen(const Stmt& stmt);

void codegen(const IfStmt& ifStmt) {
    auto* condition = codegen(ifStmt.condition);
    auto* func = builder.GetInsertBlock()->getParent();
    auto* thenBlock = llvm::BasicBlock::Create(ctx, "then", func);
    auto* elseBlock = llvm::BasicBlock::Create(ctx, "else", func);
    auto* endIfBlock = llvm::BasicBlock::Create(ctx, "endif", func);
    builder.CreateCondBr(condition, thenBlock, elseBlock);

    builder.SetInsertPoint(thenBlock);
    beginScope();
    for (const auto& stmt : ifStmt.thenBody) {
        codegen(stmt);
        if (stmt.isReturnStmt()) break;
    }
    endScope();
    if (thenBlock->empty() || !llvm::isa<llvm::ReturnInst>(thenBlock->back()))
        builder.CreateBr(endIfBlock);

    builder.SetInsertPoint(elseBlock);
    beginScope();
    for (const auto& stmt : ifStmt.elseBody) {
        codegen(stmt);
        if (stmt.isReturnStmt()) break;
    }
    endScope();
    if (elseBlock->empty() || !llvm::isa<llvm::ReturnInst>(elseBlock->back()))
        builder.CreateBr(endIfBlock);

    builder.SetInsertPoint(endIfBlock);
}

void codegen(const SwitchStmt& switchStmt) {
    auto* condition = codegen(switchStmt.condition);
    auto* func = builder.GetInsertBlock()->getParent();
    auto* insertBlockBackup = builder.GetInsertBlock();

    std::vector<std::pair<llvm::ConstantInt*, llvm::BasicBlock*>> cases;
    for (const SwitchCase& switchCase : switchStmt.cases) {
        auto* value = llvm::cast<llvm::ConstantInt>(codegen(switchCase.value));
        auto* block = llvm::BasicBlock::Create(ctx, "", func);

        builder.SetInsertPoint(block);
        beginScope();
        for (const Stmt& stmt : switchCase.stmts) {
            codegen(stmt);
            if (stmt.isReturnStmt()) break;
        }
        endScope();

        cases.emplace_back(value, block);
    }

    builder.SetInsertPoint(insertBlockBackup);
    auto* end = llvm::BasicBlock::Create(ctx, "endswitch", func);
    auto* switchInst = builder.CreateSwitch(condition, end);

    for (const auto& p : cases) {
        if (p.second->empty() || !llvm::isa<llvm::ReturnInst>(p.second->back())) {
            builder.SetInsertPoint(p.second);
            builder.CreateBr(end);
        }
        switchInst->addCase(p.first, p.second);
    }

    builder.SetInsertPoint(end);
}

void codegen(const WhileStmt& whileStmt) {
    auto* func = builder.GetInsertBlock()->getParent();
    auto* cond = llvm::BasicBlock::Create(ctx, "while", func);
    auto* body = llvm::BasicBlock::Create(ctx, "body", func);
    auto* end = llvm::BasicBlock::Create(ctx, "endwhile", func);
    builder.CreateBr(cond);

    builder.SetInsertPoint(cond);
    builder.CreateCondBr(codegen(whileStmt.condition), body, end);

    builder.SetInsertPoint(body);
    beginScope();
    for (const auto& stmt : whileStmt.body) {
        codegen(stmt);
        if (stmt.isReturnStmt()) break;
    }
    endScope();
    if (body->empty() || !llvm::isa<llvm::ReturnInst>(body->back()))
        builder.CreateBr(cond);

    builder.SetInsertPoint(end);
}

void codegen(const AssignStmt& stmt) {
    auto* lhs = codegenLvalue(stmt.lhs);
    builder.CreateStore(codegen(stmt.rhs), lhs);
}

void codegen(const Stmt& stmt) {
    switch (stmt.getKind()) {
        case StmtKind::ReturnStmt:    codegen(stmt.getReturnStmt()); break;
        case StmtKind::VariableStmt:  codegen(stmt.getVariableStmt()); break;
        case StmtKind::IncrementStmt: codegen(stmt.getIncrementStmt()); break;
        case StmtKind::DecrementStmt: codegen(stmt.getDecrementStmt()); break;
        case StmtKind::CallStmt:      codegen(stmt.getCallStmt().expr); break;
        case StmtKind::DeferStmt:     deferEvaluationOf(stmt.getDeferStmt().expr); break;
        case StmtKind::IfStmt:        codegen(stmt.getIfStmt()); break;
        case StmtKind::SwitchStmt:    codegen(stmt.getSwitchStmt()); break;
        case StmtKind::WhileStmt:     codegen(stmt.getWhileStmt()); break;
        case StmtKind::AssignStmt:    codegen(stmt.getAssignStmt()); break;
    }
}

void createDeinitCall(llvm::Value* valueToDeinit) {
    auto* alloca = llvm::dyn_cast<llvm::AllocaInst>(valueToDeinit);
    auto* typeToDeinit = alloca ? alloca->getAllocatedType() : valueToDeinit->getType();
    if (!typeToDeinit->isStructTy()) return;

    // Prevent recursively destroying the argument in struct deinitializers.
    if (llvm::isa<llvm::Argument>(valueToDeinit)
        && builder.GetInsertBlock()->getParent()->getName().startswith("__deinit_")) return;

    llvm::StringRef typeName = typeToDeinit->getStructName();
    llvm::Function* deinit = module.getFunction(("__deinit_" + typeName).str());
    if (!deinit) return;
    if (valueToDeinit->getType()->isPointerTy() && !deinit->arg_begin()->getType()->isPointerTy()) {
        builder.CreateCall(deinit, builder.CreateLoad(valueToDeinit));
    } else if (!valueToDeinit->getType()->isPointerTy() && deinit->arg_begin()->getType()->isPointerTy()) {
        llvm::errs() << "deinitialization of by-value class parameters not implemented yet\n";
        abort();
    } else {
        builder.CreateCall(deinit, valueToDeinit);
    }
}

llvm::Type* getLLVMTypeForPassing(llvm::StringRef typeName) {
    auto structTypeAndDecl = structs.find(typeName)->second;
    if (structTypeAndDecl.second->passByValue()) {
        return structTypeAndDecl.first;
    } else {
        return llvm::PointerType::get(structTypeAndDecl.first, 0);
    }
}

llvm::Function* codegenFuncProto(const FuncDecl& decl) {
    const auto& funcType = decl.getFuncType();

    assert(funcType.returnTypes.size() == 1 && "IRGen doesn't support multiple return values yet");
    auto* returnType = toIR(funcType.returnTypes[0]);
    if (decl.name == "main" && returnType->isVoidTy()) returnType = llvm::Type::getInt32Ty(ctx);

    llvm::SmallVector<llvm::Type*, 16> paramTypes;
    if (decl.isMemberFunc()) paramTypes.emplace_back(getLLVMTypeForPassing(decl.receiverType));
    for (const auto& t : funcType.paramTypes) paramTypes.emplace_back(toIR(t));

    auto* llvmFuncType = llvm::FunctionType::get(returnType, paramTypes, false);
    auto* func = llvm::Function::Create(llvmFuncType, llvm::Function::ExternalLinkage, decl.name, &module);

    auto arg = func->arg_begin(), argsEnd = func->arg_end();
    if (decl.isMemberFunc()) arg++->setName("this");
    for (auto param = decl.params.begin(); arg != argsEnd; ++param, ++arg) arg->setName(param->name);

    funcs.emplace(decl.name, func);
    return func;
}

llvm::Function* getFunc(llvm::StringRef name) {
    auto it = funcs.find(name);
    if (it == funcs.end()) {
        // Function has not been declared yet, search for it in the symbol table.
        return codegenFuncProto(findInSymbolTable(name, SrcLoc::invalid()).getFuncDecl());
    }
    return it->second;
}

void codegenFuncBody(llvm::ArrayRef<Stmt> body, llvm::Function& func) {
    lastAlloca = llvm::BasicBlock::iterator();
    builder.SetInsertPoint(llvm::BasicBlock::Create(ctx, "", &func));
    beginScope();
    for (auto& arg : func.args()) setLocalValue(arg.getName(), &arg);
    for (const auto& stmt : body) codegen(stmt);
    endScope();

    if (builder.GetInsertBlock()->empty() || !llvm::isa<llvm::ReturnInst>(builder.GetInsertBlock()->back())) {
        if (func.getName() != "main") {
            builder.CreateRetVoid();
        } else {
            builder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), 0));
        }
    }
    localValues.clear();
}

void codegen(const FuncDecl& decl) {
    auto it = funcs.find(decl.name);
    auto* func = it == funcs.end() ? codegenFuncProto(decl) : it->second;
    if (!decl.isExtern()) codegenFuncBody(*decl.body, *func);
    assert(!llvm::verifyFunction(*func, &llvm::errs()));
}

void codegen(const InitDecl& decl) {
    FuncDecl funcDecl{"__init_" + decl.getTypeDecl().name, decl.params, decl.getTypeDecl().getType(),
        "", nullptr, SrcLoc::invalid()};
    auto* func = codegenFuncProto(funcDecl);
    builder.SetInsertPoint(llvm::BasicBlock::Create(ctx, "", func));

    auto* type = llvm::cast<llvm::StructType>(toIR(decl.getTypeDecl().getType()));
    auto* alloca = builder.CreateAlloca(type);
    builder.CreateStore(llvm::UndefValue::get(type), alloca);

    setLocalValue("this", alloca);
    for (auto& arg : func->args()) setLocalValue(arg.getName(), &arg);
    for (const auto& stmt : *decl.body) codegen(stmt);
    builder.CreateRet(builder.CreateLoad(alloca));
    localValues.clear();

    assert(!llvm::verifyFunction(*func, &llvm::errs()));
}

void codegen(const DeinitDecl& decl) {
    FuncDecl funcDecl{"__deinit_" + decl.getTypeDecl().name, {}, BasicType{"void"},
        decl.getTypeDecl().name, nullptr, decl.srcLoc};
    llvm::Function* func = codegenFuncProto(funcDecl);
    codegenFuncBody(*decl.body, *func);
    assert(!llvm::verifyFunction(*func, &llvm::errs()));
}

void codegen(const TypeDecl& decl) {
    if (decl.fields.empty()) {
        structs.emplace(decl.name, std::make_pair(llvm::StructType::get(ctx), &decl));
        return;
    }
    auto elements = map(decl.fields, *[](const FieldDecl& f) { return toIR(f.type); });
    structs.emplace(decl.name, std::make_pair(llvm::StructType::create(elements, decl.name), &decl));
}

void codegen(const VarDecl& decl) {
    auto* value = new llvm::GlobalVariable(module, toIR(decl.getType()), !decl.isMutable(),
                                           llvm::GlobalValue::PrivateLinkage,
                                           llvm::cast<llvm::Constant>(codegen(*decl.initializer)),
                                           decl.name);
    globalValues.emplace(decl.name, value);
}

void codegen(const Decl& decl) {
    switch (decl.getKind()) {
        case DeclKind::ParamDecl: /* handled via FuncDecl */ assert(false); break;
        case DeclKind::FuncDecl:  codegen(decl.getFuncDecl()); break;
        case DeclKind::InitDecl:  codegen(decl.getInitDecl()); break;
        case DeclKind::DeinitDecl:codegen(decl.getDeinitDecl()); break;
        case DeclKind::TypeDecl:  codegen(decl.getTypeDecl()); break;
        case DeclKind::VarDecl:   codegen(decl.getVarDecl()); break;
        case DeclKind::FieldDecl: /* handled via TypeDecl */ assert(false); break;
        case DeclKind::ImportDecl: break;
    }
}

} // anonymous namespace

llvm::Module& irgen::compile(const std::vector<Decl>& decls) {
    globalDecls = &decls;
    for (const Decl& decl : decls) {
        currentDecl = &decl;
        codegen(decl);
    }

    assert(!llvm::verifyModule(module, &llvm::errs()));
    return module;
}
