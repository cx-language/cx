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

llvm::LLVMContext ctx;
llvm::IRBuilder<> builder(ctx);
llvm::Module module("", ctx);
std::unordered_map<std::string, llvm::Value*> globalValues;
std::unordered_map<std::string, llvm::Value*> localValues;
std::unordered_map<std::string, llvm::Function*> funcs;
std::unordered_map<std::string, std::pair<llvm::StructType*, const TypeDecl*>> structs;
const std::vector<Decl>* globalDecls;
const Decl* currentDecl;

void setLocalValue(std::string name, llvm::Value* value) {
    bool wasInserted = localValues.emplace(std::move(name), value).second;
    assert(wasInserted);
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

llvm::Value* codegen(const Expr& expr);
llvm::Value* codegenLvalue(const Expr& expr);

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

llvm::Value* codegen(const PrefixExpr& expr) {
    switch (expr.op.rawValue) {
        case PLUS:  return codegen(*expr.operand);
        case MINUS: return codegenPrefixOp(expr, &llvm::IRBuilder<>::CreateNeg);
        case STAR:  return builder.CreateLoad(codegen(*expr.operand));
        case AND:   return codegenLvalue(*expr.operand);
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
        default: assert(false);
    }
}

llvm::Function* getFunc(llvm::StringRef name);

llvm::Value* codegenForPassing(const Expr& expr) {
    if (expr.isRvalue() || expr.isStrLiteralExpr()) return codegen(expr);
    const Type* thisType = &expr.getType();
    if (thisType->isPtrType()) thisType = thisType->getPtrType().pointeeType.get();
    auto it = structs.find(thisType->getBasicType().name);
    if (it == structs.end() || it->second.second->passByValue()) return codegen(expr);
    return codegenLvalue(expr);
}

llvm::Value* codegen(const CallExpr& expr) {
    llvm::Function* func;
    if (expr.isInitializerCall) {
        func = module.getFunction("__init_" + expr.funcName);
    } else {
        func = getFunc(expr.funcName);
    }

    llvm::SmallVector<llvm::Value*, 16> args;
    if (expr.isMemberFuncCall()) args.emplace_back(codegenForPassing(*expr.receiver));
    for (const auto& arg : expr.args) args.emplace_back(codegenForPassing(*arg.value));

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

void codegen(const ReturnStmt& stmt) {
    assert(stmt.values.size() < 2 && "IRGen doesn't support multuple return values yet");

    if (stmt.values.empty()) {
        if (currentDecl->getFuncDecl().name != "main") builder.CreateRetVoid();
        else builder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), 0));
    } else {
        builder.CreateRet(codegen(stmt.values[0]));
    }
}

void codegen(const VariableStmt& stmt) {
    auto* alloca = builder.CreateAlloca(toIR(stmt.decl->getType()), nullptr, stmt.decl->name);
    setLocalValue(stmt.decl->name, alloca);

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
    for (const auto& stmt : ifStmt.thenBody) {
        codegen(stmt);
        if (stmt.isReturnStmt()) break;
    }
    if (thenBlock->empty() || !llvm::isa<llvm::ReturnInst>(thenBlock->back()))
        builder.CreateBr(endIfBlock);

    builder.SetInsertPoint(elseBlock);
    for (const auto& stmt : ifStmt.elseBody) {
        codegen(stmt);
        if (stmt.isReturnStmt()) break;
    }
    if (elseBlock->empty() || !llvm::isa<llvm::ReturnInst>(elseBlock->back()))
        builder.CreateBr(endIfBlock);

    builder.SetInsertPoint(endIfBlock);
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
    for (const auto& stmt : whileStmt.body) {
        codegen(stmt);
        if (stmt.isReturnStmt()) break;
    }
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
        case StmtKind::IfStmt:        codegen(stmt.getIfStmt()); break;
        case StmtKind::WhileStmt:     codegen(stmt.getWhileStmt()); break;
        case StmtKind::AssignStmt:    codegen(stmt.getAssignStmt()); break;
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
    builder.SetInsertPoint(llvm::BasicBlock::Create(ctx, "", &func));
    for (auto& arg : func.args()) setLocalValue(arg.getName(), &arg);
    for (const auto& stmt : body) codegen(stmt);

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
