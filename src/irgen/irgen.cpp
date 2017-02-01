#include <unordered_map>
#include <vector>
#include <cassert>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Verifier.h>
#include "irgen.h"
#include "../sema/typecheck.h"
#include "../parser/parser.hpp"

static llvm::LLVMContext ctx;
static llvm::IRBuilder<> builder(ctx);
static llvm::Module module("", ctx);
static std::unordered_map<std::string, llvm::Value*> namedValues;
static std::unordered_map<std::string, llvm::Function*> funcs;
static std::unordered_map<std::string, llvm::StructType*> structs;
static const std::vector<Decl>* globalDecls;
static const Decl* currentDecl;

template<typename From, typename To>
std::vector<To> map(const std::vector<From>& from, To (&func)(const From&)) {
    std::vector<To> to;
    to.reserve(from.size());
    for (const auto& e : from) to.emplace_back(func(e));
    return to;
}

static llvm::Type* toIR(const Type& type) {
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
            assert(it != structs.end());
            return it->second;
        }
        case TypeKind::ArrayType: {
            const auto& array = type.getArrayType();
            return llvm::ArrayType::get(toIR(*array.elementType), array.size);
        }
        case TypeKind::TupleType:
            assert(false && "IRGen doesn't support tuple types yet");
        case TypeKind::FuncType:
            assert(false && "IRGen doesn't support function types yet");
        case TypeKind::PtrType:
            return llvm::PointerType::get(toIR(*type.getPtrType().pointeeType), 0);
    }
}

static llvm::Value* codegen(const Expr& expr);

static llvm::Value* codegen(const VariableExpr& expr) {
    auto it = namedValues.find(expr.identifier);
    assert(it != namedValues.end());
    if (auto* arg = llvm::dyn_cast<llvm::Argument>(it->second)) return arg;
    return builder.CreateLoad(it->second, expr.identifier);
}

static llvm::Value* codegenLvalue(const VariableExpr& expr) {
    auto it = namedValues.find(expr.identifier);
    assert(it != namedValues.end());
    return it->second;
}

static llvm::Value* codegen(const StrLiteralExpr& expr) {
    return builder.CreateGlobalStringPtr(expr.value);
}

static llvm::Value* codegen(const IntLiteralExpr& expr) {
    return llvm::ConstantInt::getSigned(llvm::Type::getInt32Ty(ctx), expr.value);
}

static llvm::Value* codegen(const BoolLiteralExpr& expr) {
    return expr.value ? llvm::ConstantInt::getTrue(ctx) : llvm::ConstantInt::getFalse(ctx);
}

using CreateNegFunc       = decltype(&llvm::IRBuilder<>::CreateNeg);
using CreateICmpFunc      = decltype(&llvm::IRBuilder<>::CreateICmpEQ);
using CreateAddSubMulFunc = decltype(&llvm::IRBuilder<>::CreateAdd);
using CreateDivFunc       = decltype(&llvm::IRBuilder<>::CreateSDiv);

static llvm::Value* codegenPrefixOp(const PrefixExpr& expr, CreateNegFunc intFunc) {
    return (builder.*intFunc)(codegen(*expr.operand), "", false, false);
}

static llvm::Value* codegen(const PrefixExpr& expr) {
    switch (expr.op.rawValue) {
        case PLUS:  return codegen(*expr.operand);
        case MINUS: return codegenPrefixOp(expr, &llvm::IRBuilder<>::CreateNeg);
        case STAR:  assert(false && "IRGen doesn't support dereference operations yet");
        case AND:   assert(false && "IRGen doesn't support reference operations yet");
        default:    assert(false);
    }
}

static llvm::Value* codegenBinaryOp(const BinaryExpr& expr, CreateICmpFunc intFunc) {
    return (builder.*intFunc)(codegen(*expr.left), codegen(*expr.right), "");
}

static llvm::Value* codegenBinaryOp(const BinaryExpr& expr, CreateAddSubMulFunc intFunc) {
    return (builder.*intFunc)(codegen(*expr.left), codegen(*expr.right), "", false, false);
}

static llvm::Value* codegenBinaryOp(const BinaryExpr& expr, CreateDivFunc intFunc) {
    return (builder.*intFunc)(codegen(*expr.left), codegen(*expr.right), "", false);
}

static llvm::Value* codegen(const BinaryExpr& expr) {
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

static llvm::Function* getFunc(llvm::StringRef name);

static llvm::Value* codegen(const CallExpr& expr) {
    assert(!expr.isMemberFuncCall() && "IRGen doesn't support member function calls yet");

    llvm::Function* func;
    if (expr.isInitializerCall) {
        func = module.getFunction("__init_" + expr.funcName);
    } else {
        func = getFunc(expr.funcName);
    }

    auto args = map(expr.args, *[](const Arg& arg) { return codegen(*arg.value); });
    return builder.CreateCall(func, args);
}

static llvm::Value* codegen(const CastExpr& expr) {
    assert(false && "IRGen doesn't support cast expressions yet");
}

static llvm::Value* codegen(const MemberExpr& expr) {
    assert(false && "IRGen doesn't support member expressions yet");
}

static llvm::Value* codegen(const SubscriptExpr& expr) {
    assert(false && "IRGen doesn't support subscript expressions yet");
}

static llvm::Value* codegen(const Expr& expr) {
    switch (expr.getKind()) {
        case ExprKind::VariableExpr:    return codegen(expr.getVariableExpr());
        case ExprKind::StrLiteralExpr:  return codegen(expr.getStrLiteralExpr());
        case ExprKind::IntLiteralExpr:  return codegen(expr.getIntLiteralExpr());
        case ExprKind::BoolLiteralExpr: return codegen(expr.getBoolLiteralExpr());
        case ExprKind::PrefixExpr:      return codegen(expr.getPrefixExpr());
        case ExprKind::BinaryExpr:      return codegen(expr.getBinaryExpr());
        case ExprKind::CallExpr:        return codegen(expr.getCallExpr());
        case ExprKind::CastExpr:        return codegen(expr.getCastExpr());
        case ExprKind::MemberExpr:      return codegen(expr.getMemberExpr());
        case ExprKind::SubscriptExpr:   return codegen(expr.getSubscriptExpr());
    }
}

static llvm::Value* codegenLvalue(const Expr& expr) {
    switch (expr.getKind()) {
        case ExprKind::VariableExpr:    return codegenLvalue(expr.getVariableExpr());
        case ExprKind::StrLiteralExpr:  assert(false);
        case ExprKind::IntLiteralExpr:  assert(false);
        case ExprKind::BoolLiteralExpr: assert(false);
        case ExprKind::PrefixExpr:      assert(false);
        case ExprKind::BinaryExpr:      assert(false);
        case ExprKind::CallExpr:        assert(false && "IRGen doesn't support lvalue call expressions yet");
        case ExprKind::CastExpr:        assert(false && "IRGen doesn't support lvalue cast expressions yet");
        case ExprKind::MemberExpr:      assert(false && "IRGen doesn't support lvalue member expressions yet");
        case ExprKind::SubscriptExpr:   assert(false && "IRGen doesn't support lvalue subscript expressions yet");
    }
}

static void codegen(const ReturnStmt& stmt) {
    assert(stmt.values.size() < 2 && "IRGen doesn't support multuple return values yet");

    if (stmt.values.empty()) {
        builder.CreateRetVoid();
    } else {
        builder.CreateRet(codegen(stmt.values[0]));
    }
}

static void codegen(const VariableStmt& stmt) {
    auto* alloca = builder.CreateAlloca(toIR(stmt.decl->getType()), nullptr, stmt.decl->name);
    namedValues.emplace(stmt.decl->name, alloca);

    if (auto initializer = stmt.decl->initializer) {
        builder.CreateStore(codegen(*stmt.decl->initializer), alloca);
    }
}

static void codegen(const IncrementStmt& stmt) {
    auto* alloca = codegenLvalue(stmt.operand);
    auto* value = builder.CreateLoad(alloca);
    auto* result = builder.CreateAdd(value, llvm::ConstantInt::get(value->getType(), 1));
    builder.CreateStore(result, alloca);
}

static void codegen(const DecrementStmt& stmt) {
    auto* alloca = codegenLvalue(stmt.operand);
    auto* value = builder.CreateLoad(alloca);
    auto* result = builder.CreateSub(value, llvm::ConstantInt::get(value->getType(), 1));
    builder.CreateStore(result, alloca);
}

static void codegen(const Stmt& stmt);

static void codegen(const IfStmt& ifStmt) {
    auto* condition = codegen(ifStmt.condition);
    auto* func = builder.GetInsertBlock()->getParent();
    auto* thenBlock = llvm::BasicBlock::Create(ctx, "then", func);
    auto* elseBlock = llvm::BasicBlock::Create(ctx, "else", func);
    auto* endIfBlock = llvm::BasicBlock::Create(ctx, "endif", func);
    builder.CreateCondBr(condition, thenBlock, elseBlock);

    builder.SetInsertPoint(thenBlock);
    for (const auto& stmt : ifStmt.thenBody) codegen(stmt);
    builder.CreateBr(endIfBlock);

    builder.SetInsertPoint(elseBlock);
    for (const auto& stmt : ifStmt.elseBody) codegen(stmt);
    builder.CreateBr(endIfBlock);

    builder.SetInsertPoint(endIfBlock);
}

static void codegen(const WhileStmt& whileStmt) {
    auto* func = builder.GetInsertBlock()->getParent();
    auto* cond = llvm::BasicBlock::Create(ctx, "while", func);
    auto* body = llvm::BasicBlock::Create(ctx, "body", func);
    auto* end = llvm::BasicBlock::Create(ctx, "endwhile", func);
    builder.CreateBr(cond);

    builder.SetInsertPoint(cond);
    builder.CreateCondBr(codegen(whileStmt.condition), body, end);

    builder.SetInsertPoint(body);
    for (const auto& stmt : whileStmt.body) codegen(stmt);
    builder.CreateBr(cond);

    builder.SetInsertPoint(end);
}

static void codegen(const AssignStmt& stmt) {
    builder.CreateStore(codegen(stmt.rhs), codegenLvalue(stmt.lhs));
}

static void codegen(const Stmt& stmt) {
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

static llvm::Function* codegenFuncProto(const FuncDecl& decl) {
    const auto& funcType = decl.getFuncType();

    assert(!decl.isMemberFunc() && "IRGen doesn't support member functions yet");
    assert(funcType.returnTypes.size() == 1 && "IRGen doesn't support multiple return values yet");
    auto* returnType = toIR(funcType.returnTypes[0]);
    auto paramTypes = map(funcType.paramTypes, toIR);

    auto* llvmFuncType = llvm::FunctionType::get(returnType, paramTypes, false);
    auto* func = llvm::Function::Create(llvmFuncType, llvm::Function::ExternalLinkage, decl.name, &module);

    auto paramIt = decl.params.begin();
    for (auto& a : func->args()) a.setName(paramIt++->name);

    funcs.emplace(decl.name, func);
    return func;
}

static llvm::Function* getFunc(llvm::StringRef name) {
    auto it = funcs.find(name);
    if (it == funcs.end()) {
        // Function has not been declared yet, search for it in the symbol table.
        return codegenFuncProto(findInSymbolTable(name).getFuncDecl());
    }
    return it->second;
}

static void codegenFuncBody(llvm::ArrayRef<Stmt> body, llvm::Function& func) {
    builder.SetInsertPoint(llvm::BasicBlock::Create(ctx, "", &func));
    for (auto& arg : func.args()) namedValues.emplace(arg.getName(), &arg);
    for (const auto& stmt : body) codegen(stmt);

    if (builder.GetInsertBlock()->empty() || !llvm::isa<llvm::ReturnInst>(builder.GetInsertBlock()->back())) {
        builder.CreateRetVoid();
    }
}

static void codegen(const FuncDecl& decl) {
    auto it = funcs.find(decl.name);
    auto* func = it == funcs.end() ? codegenFuncProto(decl) : it->second;
    if (!decl.isExtern()) codegenFuncBody(*decl.body, *func);
    assert(!llvm::verifyFunction(*func, &llvm::errs()));
}

static void codegen(const InitDecl& decl) {
    FuncDecl funcDecl{"__init_" + decl.getTypeDecl().name, decl.params, decl.getTypeDecl().getType()};
    auto* func = codegenFuncProto(funcDecl);
    builder.SetInsertPoint(llvm::BasicBlock::Create(ctx, "", func));

    auto* type = llvm::cast<llvm::StructType>(toIR(decl.getTypeDecl().getType()));
    auto* alloca = builder.CreateAlloca(type);
    builder.CreateStore(llvm::UndefValue::get(type), alloca);

    namedValues.emplace("this", alloca);
    for (auto& arg : func->args()) namedValues.emplace(arg.getName(), &arg);
    for (const auto& stmt : *decl.body) codegen(stmt);
    builder.CreateRet(builder.CreateLoad(alloca));

    assert(!llvm::verifyFunction(*func, &llvm::errs()));
}

static void codegen(const TypeDecl& decl) {
    auto elements = map(decl.fields, *[](const FieldDecl& f) { return toIR(f.type); });
    structs.emplace(decl.name, llvm::StructType::create(elements, decl.name));
}

static void codegen(const VarDecl& decl) {
    assert(false && "IRGen doesn't support global variables yet");
}

static void codegen(const FieldDecl& decl) {
    assert(false && "IRGen doesn't support custom types yet");
}

static void codegen(const Decl& decl) {
    switch (decl.getKind()) {
        case DeclKind::ParamDecl: /* handled via FuncDecl */ assert(false); break;
        case DeclKind::FuncDecl:  codegen(decl.getFuncDecl()); break;
        case DeclKind::InitDecl:  codegen(decl.getInitDecl()); break;
        case DeclKind::TypeDecl:  codegen(decl.getTypeDecl()); break;
        case DeclKind::VarDecl:   codegen(decl.getVarDecl()); break;
        case DeclKind::FieldDecl: codegen(decl.getFieldDecl()); break;
        case DeclKind::ImportDecl: break;
    }
}

void irgen::compile(const std::vector<Decl>& decls, llvm::StringRef outputPath) {
    globalDecls = &decls;
    for (const Decl& decl : decls) {
        currentDecl = &decl;
        codegen(decl);
    }

    assert(!llvm::verifyModule(module, &llvm::errs()));

    if (outputPath == "stdout") {
        module.print(llvm::outs(), nullptr);
    }
}
