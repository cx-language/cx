#include <iterator>
#include <unordered_map>
#include <vector>
#include <memory>
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

namespace delta {
llvm::LLVMContext ctx;
}

namespace {

/// Helper for storing parameter name info in 'functionInstantiations' key strings.
template<typename T>
std::string mangleWithParams(const T& decl, llvm::ArrayRef<Type> typeGenericArgs = {},
                             llvm::ArrayRef<Type> functionGenericArgs = {}) {
    std::string result = mangle(decl, typeGenericArgs, functionGenericArgs);
    for (auto& param : decl.getParams()) {
        result.append("$").append(param.getName());
        result.append(":").append(param.getType().toString());
    }
    return result;
}

} // anonymous namespace

void Scope::onScopeEnd() {
    for (const Expr* expr : llvm::reverse(deferredExprs)) irGenerator.codegenExpr(*expr);

    for (auto& p : llvm::reverse(deinitsToCall)) {
        if (p.decl && p.decl->hasBeenMoved()) continue;
        irGenerator.createDeinitCall(p.function, p.value, p.type, p.decl);
    }
}

void Scope::clear() {
    deferredExprs.clear();
    deinitsToCall.clear();
}

IRGenerator::IRGenerator()
: builder(ctx), module("", ctx) {
    scopes.push_back(Scope(*this));
}

llvm::Function* IRGenerator::getFunction(Type receiverType, llvm::StringRef functionName,
                                         llvm::ArrayRef<Type> functionGenericArgs) {
    auto mangledName = mangleFunctionDecl(receiverType.getName(), functionName);
    auto it = functionInstantiations.find(mangledName);
    if (it == functionInstantiations.end()) {
        auto decls = currentTypeChecker->findDecls(mangledName, /*everywhere*/ true);
        if (!decls.empty()) {
            auto& decl = llvm::cast<FunctionLikeDecl>(*decls[0]);
            return getFunctionProto(decl, functionGenericArgs, receiverType);
        }
        return nullptr;
    }
    return it->second.getFunction();
}

/// @param type The Delta type of the variable, or null if the variable is 'this'.
void IRGenerator::setLocalValue(Type type, std::string name, llvm::Value* value, const Decl* decl) {
    scopes.back().addLocalValue(std::move(name), value);

    if (type && type.isBasicType()) {
        llvm::Function* deinit = getFunction(type, "deinit", {});
        if (deinit) deferDeinitCall(deinit, value, type, decl);
    }
}

llvm::Value* IRGenerator::findValue(llvm::StringRef name, const Decl* decl) {
    llvm::Value* value = nullptr;

    for (const auto& scope : llvm::reverse(scopes)) {
        auto it = scope.getLocalValues().find(name);
        if (it == scope.getLocalValues().end()) continue;
        value = it->second;
        break;
    }

    if (!value) {
        ASSERT(decl);
        if (auto fieldDecl = llvm::dyn_cast<FieldDecl>(decl)) {
            return codegenMemberAccess(findValue("this", nullptr), fieldDecl->getType(), fieldDecl->getName());
        }
        codegenDecl(*decl);
        if (auto* varDecl = llvm::dyn_cast<VarDecl>(decl)) {
            name = varDecl->getName();
        }
        return globalScope().getLocalValues().find(name)->second;
    }
    return value;
}

static const std::unordered_map<std::string, llvm::Type*> builtinTypes = {
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
};

llvm::Type* IRGenerator::toIR(Type type) {
    switch (type.getKind()) {
        case TypeKind::BasicType: {
            llvm::StringRef name = type.getName();

            auto builtinType = builtinTypes.find(name);
            if (builtinType != builtinTypes.end()) return builtinType->second;

            auto it = structs.find(name);
            if (it == structs.end()) {
                // Is it a generic parameter?
                auto genericArg = currentGenericArgs.find(name);
                if (genericArg != currentGenericArgs.end()) return toIR(genericArg->second);

                auto& decl = currentTypeChecker->findDecl(name, SourceLocation::invalid(),
                                                          /* everywhere */ true);

                // Is it a generic type?
                auto genericArgs = llvm::cast<BasicType>(*type).getGenericArgs();
                if (!genericArgs.empty()) {
                    return codegenGenericTypeInstantiation(llvm::cast<TypeDecl>(decl), genericArgs);
                }

                // Custom type that has not been defined yet.
                codegenTypeDecl(llvm::cast<TypeDecl>(decl));
                it = structs.find(name);
            }
            return it->second.first;
        }
        case TypeKind::ArrayType:
            ASSERT(type.getArraySize() != ArrayType::unsized, "unimplemented");
            return llvm::ArrayType::get(toIR(type.getElementType()), type.getArraySize());
        case TypeKind::TupleType:
            llvm_unreachable("IRGen doesn't support tuple types yet");
        case TypeKind::FunctionType:
            return llvm::IntegerType::getInt8Ty(ctx)->getPointerTo(); // FIXME: Temporary.
        case TypeKind::PointerType: {
            if (type.getPointee().isUnsizedArrayType()) {
                return toIR(BasicType::get("ArrayRef", type.getPointee().getElementType()));
            }
            auto* pointeeType = toIR(type.getPointee());
            if (!pointeeType->isVoidTy()) return llvm::PointerType::get(pointeeType, 0);
            else return llvm::PointerType::get(llvm::Type::getInt8Ty(ctx), 0);
        }
    }
    llvm_unreachable("all cases handled");
}

Type IRGenerator::resolveTypePlaceholder(llvm::StringRef name) const {
    auto it = currentGenericArgs.find(name);
    if (it == currentGenericArgs.end()) return nullptr;
    return it->second;
}

void IRGenerator::beginScope() {
    scopes.push_back(Scope(*this));
}

void IRGenerator::endScope() {
    scopes.back().onScopeEnd();
    scopes.pop_back();
}

void IRGenerator::deferEvaluationOf(const Expr& expr) {
    scopes.back().addDeferredExpr(expr);
}

void IRGenerator::deferDeinitCall(llvm::Function* deinit, llvm::Value* valueToDeinit, Type type, const Decl* decl) {
    scopes.back().addDeinitToCall(deinit, valueToDeinit, type, decl);
}

void IRGenerator::codegenDeferredExprsAndDeinitCallsForReturn() {
    for (auto& scope : llvm::reverse(scopes)) scope.onScopeEnd();
    scopes.back().clear();
}

void IRGenerator::codegenReturnStmt(const ReturnStmt& stmt) {
    ASSERT(stmt.getValues().size() < 2, "IRGen doesn't support multiple return values yet");

    codegenDeferredExprsAndDeinitCallsForReturn();

    if (stmt.getValues().empty()) {
        if (llvm::cast<FunctionLikeDecl>(currentDecl)->getName() != "main") builder.CreateRetVoid();
        else builder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), 0));
    } else {
        builder.CreateRet(codegenExpr(*stmt.getValues()[0]));
    }
}

llvm::AllocaInst* IRGenerator::createEntryBlockAlloca(Type type, const Decl* decl, llvm::Value* arraySize,
                                                      const llvm::Twine& name) {
    static llvm::BasicBlock::iterator lastAlloca;
    auto* insertBlock = builder.GetInsertBlock();
    auto* entryBlock = &insertBlock->getParent()->getEntryBlock();

    if (lastAlloca == llvm::BasicBlock::iterator() || lastAlloca->getParent() != entryBlock) {
        if (entryBlock->empty()) {
            builder.SetInsertPoint(entryBlock);
        } else {
            builder.SetInsertPoint(&entryBlock->front());
        }
    } else {
        builder.SetInsertPoint(entryBlock, std::next(lastAlloca));
    }

    auto* alloca = builder.CreateAlloca(toIR(type), arraySize, name);
    lastAlloca = alloca->getIterator();
    setLocalValue(type, name.str(), alloca, decl);
    builder.SetInsertPoint(insertBlock);
    return alloca;
}

void IRGenerator::codegenVarStmt(const VarStmt& stmt) {
    auto* alloca = createEntryBlockAlloca(stmt.getDecl().getType(), &stmt.getDecl(), nullptr,
                                          stmt.getDecl().getName());
    if (auto initializer = stmt.getDecl().getInitializer()) {
        builder.CreateStore(codegenExprForPassing(*initializer, alloca->getAllocatedType()), alloca);
    }
}

void IRGenerator::codegenIncrementStmt(const IncrementStmt& stmt) {
    auto* alloca = codegenLvalueExpr(stmt.getOperand());
    auto* value = builder.CreateLoad(alloca);
    auto* result = builder.CreateAdd(value, llvm::ConstantInt::get(value->getType(), 1));
    builder.CreateStore(result, alloca);
}

void IRGenerator::codegenDecrementStmt(const DecrementStmt& stmt) {
    auto* alloca = codegenLvalueExpr(stmt.getOperand());
    auto* value = builder.CreateLoad(alloca);
    auto* result = builder.CreateSub(value, llvm::ConstantInt::get(value->getType(), 1));
    builder.CreateStore(result, alloca);
}

void IRGenerator::codegenBlock(llvm::ArrayRef<std::unique_ptr<Stmt>> stmts,
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

void IRGenerator::codegenIfStmt(const IfStmt& ifStmt) {
    auto* condition = codegenExpr(ifStmt.getCondition());
    auto* function = builder.GetInsertBlock()->getParent();
    auto* thenBlock = llvm::BasicBlock::Create(ctx, "then", function);
    auto* elseBlock = llvm::BasicBlock::Create(ctx, "else", function);
    auto* endIfBlock = llvm::BasicBlock::Create(ctx, "endif", function);
    builder.CreateCondBr(condition, thenBlock, elseBlock);
    codegenBlock(ifStmt.getThenBody(), thenBlock, endIfBlock);
    codegenBlock(ifStmt.getElseBody(), elseBlock, endIfBlock);
    builder.SetInsertPoint(endIfBlock);
}

void IRGenerator::codegenSwitchStmt(const SwitchStmt& switchStmt) {
    auto* condition = codegenExpr(switchStmt.getCondition());
    auto* function = builder.GetInsertBlock()->getParent();
    auto* insertBlockBackup = builder.GetInsertBlock();

    auto cases = map(switchStmt.getCases(), [&](const SwitchCase& switchCase) {
        auto* value = llvm::cast<llvm::ConstantInt>(codegenExpr(*switchCase.getValue()));
        auto* block = llvm::BasicBlock::Create(ctx, "", function);
        return std::make_pair(value, block);
    });

    builder.SetInsertPoint(insertBlockBackup);
    auto* defaultBlock = llvm::BasicBlock::Create(ctx, "default", function);
    auto* end = llvm::BasicBlock::Create(ctx, "endswitch", function);
    breakTargets.push_back(end);
    auto* switchInst = builder.CreateSwitch(condition, defaultBlock);

    auto casesIterator = cases.begin();
    for (auto& switchCase : switchStmt.getCases()) {
        auto* value = casesIterator->first;
        auto* block = casesIterator->second;
        codegenBlock(switchCase.getStmts(), block, end);
        switchInst->addCase(value, block);
        ++casesIterator;
    }

    codegenBlock(switchStmt.getDefaultStmts(), defaultBlock, end);
    breakTargets.pop_back();
    builder.SetInsertPoint(end);
}

void IRGenerator::codegenWhileStmt(const WhileStmt& whileStmt) {
    auto* function = builder.GetInsertBlock()->getParent();
    auto* condition = llvm::BasicBlock::Create(ctx, "while", function);
    auto* body = llvm::BasicBlock::Create(ctx, "body", function);
    auto* end = llvm::BasicBlock::Create(ctx, "endwhile", function);
    breakTargets.push_back(end);
    builder.CreateBr(condition);

    builder.SetInsertPoint(condition);
    builder.CreateCondBr(codegenExpr(whileStmt.getCondition()), body, end);
    codegenBlock(whileStmt.getBody(), body, condition);

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
void IRGenerator::codegenForStmt(const ForStmt& forStmt) {
    auto rangeType = forStmt.getRangeExpr().getType();

    if (!rangeType.isRangeType()) {
        error(forStmt.getRangeExpr().getLocation(),
              "IRGen doesn't support 'for'-loops over non-range iterables yet");
    }

    if (!rangeType.getIterableElementType().isInteger()) {
        error(forStmt.getRangeExpr().getLocation(),
              "IRGen doesn't support 'for'-loops over non-integer ranges yet");
    }

    beginScope();

    Type elementType = rangeType.getIterableElementType();
    auto* rangeExpr = codegenExpr(forStmt.getRangeExpr());
    auto* firstValue = codegenMemberAccess(rangeExpr, elementType, "start");
    auto* lastValue = codegenMemberAccess(rangeExpr, elementType, "end");

    auto* counterAlloca = createEntryBlockAlloca(rangeType.getIterableElementType(), nullptr,
                                                 nullptr, forStmt.getLoopVariableName());
    builder.CreateStore(firstValue, counterAlloca);

    auto* function = builder.GetInsertBlock()->getParent();
    auto* condition = llvm::BasicBlock::Create(ctx, "for", function);
    auto* body = llvm::BasicBlock::Create(ctx, "body", function);
    auto* end = llvm::BasicBlock::Create(ctx, "endfor", function);
    breakTargets.push_back(end);
    builder.CreateBr(condition);

    builder.SetInsertPoint(condition);
    auto* counter = builder.CreateLoad(counterAlloca, forStmt.getLoopVariableName());

    llvm::Value* cmp;
    if (rangeType.getName() == "Range") {
        if (elementType.isSigned())
            cmp = builder.CreateICmpSLT(counter, lastValue);
        else
            cmp = builder.CreateICmpULT(counter, lastValue);
    } else {
        ASSERT(rangeType.getName() == "ClosedRange");

        if (elementType.isSigned())
            cmp = builder.CreateICmpSLE(counter, lastValue);
        else
            cmp = builder.CreateICmpULE(counter, lastValue);
    }
    builder.CreateCondBr(cmp, body, end);

    codegenBlock(forStmt.getBody(), body, condition);

    builder.SetInsertPoint(&builder.GetInsertBlock()->back());
    auto* newCounter = builder.CreateAdd(counter, llvm::ConstantInt::get(counter->getType(), 1));
    builder.CreateStore(newCounter, counterAlloca);

    breakTargets.pop_back();
    builder.SetInsertPoint(end);
    endScope();
}

void IRGenerator::codegenBreakStmt(const BreakStmt&) {
    ASSERT(!breakTargets.empty());
    builder.CreateBr(breakTargets.back());
}

void IRGenerator::codegenAssignStmt(const AssignStmt& stmt) {
    auto* lhsLvalue = codegenLvalueExpr(*stmt.getLHS());

    if (stmt.isCompoundAssignment()) {
        auto& binaryExpr = llvm::cast<BinaryExpr>(*stmt.getRHS());

        if (!binaryExpr.isBuiltinOp(*this)) {
            builder.CreateStore(codegenCallExpr((const CallExpr&) binaryExpr), lhsLvalue);
            return;
        }

        switch (binaryExpr.getOperator()) {
            case AND_AND: fatalError("'&&=' not implemented yet");
            case OR_OR: fatalError("'||=' not implemented yet");
            default: break;
        }

        auto* lhsValue = builder.CreateLoad(lhsLvalue);
        auto* rhsValue = codegenExpr(binaryExpr.getRHS());
        builder.CreateStore(codegenBinaryOp(binaryExpr.getOperator(), lhsValue, rhsValue, *stmt.getLHS()), lhsLvalue);
    } else {
        builder.CreateStore(codegenExpr(*stmt.getRHS()), lhsLvalue);
    }
}

void IRGenerator::codegenStmt(const Stmt& stmt) {
    switch (stmt.getKind()) {
        case StmtKind::ReturnStmt: codegenReturnStmt(llvm::cast<ReturnStmt>(stmt)); break;
        case StmtKind::VarStmt: codegenVarStmt(llvm::cast<VarStmt>(stmt)); break;
        case StmtKind::IncrementStmt: codegenIncrementStmt(llvm::cast<IncrementStmt>(stmt)); break;
        case StmtKind::DecrementStmt: codegenDecrementStmt(llvm::cast<DecrementStmt>(stmt)); break;
        case StmtKind::ExprStmt: codegenExpr(llvm::cast<ExprStmt>(stmt).getExpr()); break;
        case StmtKind::DeferStmt: deferEvaluationOf(llvm::cast<DeferStmt>(stmt).getExpr()); break;
        case StmtKind::IfStmt: codegenIfStmt(llvm::cast<IfStmt>(stmt)); break;
        case StmtKind::SwitchStmt: codegenSwitchStmt(llvm::cast<SwitchStmt>(stmt)); break;
        case StmtKind::WhileStmt: codegenWhileStmt(llvm::cast<WhileStmt>(stmt)); break;
        case StmtKind::ForStmt: codegenForStmt(llvm::cast<ForStmt>(stmt)); break;
        case StmtKind::BreakStmt: codegenBreakStmt(llvm::cast<BreakStmt>(stmt)); break;
        case StmtKind::AssignStmt: codegenAssignStmt(llvm::cast<AssignStmt>(stmt)); break;
    }
}

void IRGenerator::createDeinitCall(llvm::Function* deinit, llvm::Value* valueToDeinit, Type type, const Decl* decl) {

    if (valueToDeinit->getType()->isPointerTy() && !deinit->arg_begin()->getType()->isPointerTy()) {
        builder.CreateCall(deinit, builder.CreateLoad(valueToDeinit));
    } else if (!valueToDeinit->getType()->isPointerTy() && deinit->arg_begin()->getType()->isPointerTy()) {
        auto* alloca = createEntryBlockAlloca(type, decl);
        builder.CreateStore(valueToDeinit, alloca);
        builder.CreateCall(deinit, alloca);
    } else {
        builder.CreateCall(deinit, valueToDeinit);
    }
}

llvm::Type* IRGenerator::getLLVMTypeForPassing(const TypeDecl& typeDecl, llvm::ArrayRef<Type> genericArgs,
                                               bool isMutating) {
    std::string typeName;

    if (typeDecl.isGeneric()) {
        typeName = mangle(typeDecl, genericArgs);
        if (structs.count(typeName) == 0) {
            codegenGenericTypeInstantiation(typeDecl, genericArgs);
        }
    } else {
        typeName = typeDecl.getName();
        if (structs.count(typeName) == 0) {
            codegenTypeDecl(typeDecl);
        }
    }

    ASSERT(structs.count(typeName));
    auto structTypeAndDecl = structs.find(typeName)->second;

    if (!isMutating && structTypeAndDecl.second->passByValue()) {
        return structTypeAndDecl.first;
    } else {
        return llvm::PointerType::get(structTypeAndDecl.first, 0);
    }
}

void IRGenerator::setCurrentGenericArgs(llvm::ArrayRef<GenericParamDecl> genericParams,
                                        llvm::ArrayRef<Type> genericArgs) {
    ASSERT(genericParams.size() == genericArgs.size());
    for (auto tuple : llvm::zip_first(genericParams, genericArgs)) {
        currentGenericArgs.emplace(std::get<0>(tuple).getName(), std::get<1>(tuple));
    }
}

llvm::Function* IRGenerator::getFunctionProto(const FunctionLikeDecl& decl,
                                              llvm::ArrayRef<Type> functionGenericArgs,
                                              Type receiverType, std::string&& mangledName) {
    auto resolvedFunctionGenericArgs = map(functionGenericArgs, [&](Type type) { return resolve(type); });
    functionGenericArgs = resolvedFunctionGenericArgs;

    llvm::ArrayRef<Type> receiverTypeGenericArgs;
    if (receiverType) {
        receiverTypeGenericArgs = llvm::cast<BasicType>(*receiverType.removePointer()).getGenericArgs();
    }

    auto it = functionInstantiations.find(mangleWithParams(decl, receiverTypeGenericArgs, functionGenericArgs));
    if (it != functionInstantiations.end()) return it->second.getFunction();

    SAVE_STATE(currentGenericArgs);
    setCurrentGenericArgs(decl.getGenericParams(), functionGenericArgs);

    auto* functionType = decl.getFunctionType();
    llvm::SmallVector<llvm::Type*, 16> paramTypes;

    if (decl.isMethodDecl()) {
        auto* receiverTypeDecl = decl.getTypeDecl();
        if (receiverTypeDecl->isGeneric()) {
            mangledName = mangle(decl, receiverTypeGenericArgs, functionGenericArgs);
            setCurrentGenericArgs(receiverTypeDecl->getGenericParams(), receiverTypeGenericArgs);
        }

        if (!decl.isInitDecl()) {
            paramTypes.emplace_back(getLLVMTypeForPassing(*receiverTypeDecl, receiverTypeGenericArgs,
                                                          decl.isMutating()));
        }
    }

    for (auto& paramType : functionType->getParamTypes()) {
        paramTypes.emplace_back(toIR(paramType));
    }

    ASSERT(!functionType->getReturnType().isTupleType(), "IRGen doesn't support tuple return values yet");
    auto* returnType = toIR(decl.isInitDecl() ? receiverType : functionType->getReturnType());
    if (decl.getName() == "main" && returnType->isVoidTy()) returnType = llvm::Type::getInt32Ty(ctx);

    auto* llvmFunctionType = llvm::FunctionType::get(returnType, paramTypes, decl.isVariadic());
    if (mangledName.empty()) mangledName = mangle(decl, receiverTypeGenericArgs, functionGenericArgs);
    auto* function = llvm::Function::Create(llvmFunctionType, llvm::Function::ExternalLinkage,
                                            mangledName, &module);

    auto arg = function->arg_begin(), argsEnd = function->arg_end();
    if (decl.isMethodDecl() && !decl.isInitDecl()) arg++->setName("this");

    ASSERT(decl.getParams().size() == size_t(std::distance(arg, argsEnd)));
    for (auto param = decl.getParams().begin(); arg != argsEnd; ++param, ++arg) {
        arg->setName(param->getName());
    }

    auto mangled = mangleWithParams(decl, receiverTypeGenericArgs, functionGenericArgs);
    FunctionInstantiation functionInstantiation{decl, receiverTypeGenericArgs, functionGenericArgs, function};
    return functionInstantiations.emplace(std::move(mangled),
                                          std::move(functionInstantiation)).first->second.getFunction();
}

llvm::Function* IRGenerator::getFunctionForCall(const CallExpr& call) {
    if (!call.callsNamedFunction()) fatalError("anonymous function calls not implemented yet");

    const Decl* decl = call.getCalleeDecl();

    switch (decl->getKind()) {
        case DeclKind::FunctionDecl:
        case DeclKind::MethodDecl: {
            auto* functionDecl = llvm::cast<FunctionDecl>(decl);
            return getFunctionProto(*functionDecl, call.getGenericArgs(), resolve(call.getReceiverType()));
        }
        case DeclKind::InitDecl: {
            auto* initDecl = llvm::cast<InitDecl>(decl);
            Type receiverType = initDecl->getTypeDecl()->getType(call.getGenericArgs());
            llvm::Function* function = getFunctionProto(*initDecl, {}, receiverType);
            if (function->empty() && !call.getGenericArgs().empty()) {
                auto backup = builder.GetInsertBlock();
                codegenInitDecl(*initDecl, call.getGenericArgs());
                builder.SetInsertPoint(backup);
            }
            return function;
        }
        default:
            llvm_unreachable("invalid callee decl");
    }
}

void IRGenerator::codegenFunctionBody(const FunctionLikeDecl& decl, llvm::Function& function) {
    builder.SetInsertPoint(llvm::BasicBlock::Create(ctx, "", &function));
    beginScope();
    auto arg = function.arg_begin();
    if (decl.getTypeDecl() != nullptr) setLocalValue(nullptr, "this", &*arg++, nullptr);
    for (auto& param : decl.getParams()) {
        setLocalValue(param.getType(), param.getName(), &*arg++, &param);
    }
    for (auto& stmt : *decl.getBody()) {
        codegenStmt(*stmt);
    }
    endScope();

    auto* insertBlock = builder.GetInsertBlock();
    if (insertBlock != &function.getEntryBlock() && llvm::pred_empty(insertBlock)) {
        insertBlock->eraseFromParent();
        return;
    }

    if (insertBlock->empty() || !llvm::isa<llvm::ReturnInst>(insertBlock->back())) {
        if (function.getName() != "main") {
            builder.CreateRetVoid();
        } else {
            builder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), 0));
        }
    }
}

void IRGenerator::codegenFunctionDecl(const FunctionDecl& decl) {
    if (decl.isGeneric()) return;
    if (decl.getTypeDecl() && decl.getTypeDecl()->isGeneric()) return;

    llvm::Function* function = getFunctionProto(decl);
    if (!decl.isExtern()) codegenFunctionBody(decl, *function);
    ASSERT(!llvm::verifyFunction(*function, &llvm::errs()));
}

void IRGenerator::codegenInitDecl(const InitDecl& decl, llvm::ArrayRef<Type> typeGenericArgs) {
    if (decl.getTypeDecl()->isGeneric() && typeGenericArgs.empty()) return;
    SAVE_STATE(currentGenericArgs);
    setCurrentGenericArgs(decl.getTypeDecl()->getGenericParams(), typeGenericArgs);

    llvm::Function* function = getFunctionProto(decl, {}, decl.getTypeDecl()->getType(typeGenericArgs));

    builder.SetInsertPoint(llvm::BasicBlock::Create(ctx, "", function));

    auto* type = llvm::cast<llvm::StructType>(toIR(decl.getTypeDecl()->getType(typeGenericArgs)));
    auto* alloca = builder.CreateAlloca(type);

    beginScope();
    setLocalValue(nullptr, "this", alloca, nullptr);
    auto param = decl.getParams().begin();
    for (auto& arg : function->args()) {
        setLocalValue(param->getType(), arg.getName(), &arg, param);
        ++param;
    }
    for (auto& stmt : *decl.getBody()) {
        codegenStmt(*stmt);
    }
    builder.CreateRet(builder.CreateLoad(alloca));
    endScope();

    ASSERT(!llvm::verifyFunction(*function, &llvm::errs()));
}

std::vector<llvm::Type*> IRGenerator::getFieldTypes(const TypeDecl& decl) {
    return map(decl.getFields(), [&](const FieldDecl& field) { return toIR(field.getType()); });
}

void IRGenerator::codegenTypeDecl(const TypeDecl& decl) {
    if (decl.isGeneric()) return;
    if (structs.count(decl.getName())) return;

    if (decl.getFields().empty()) {
        structs.emplace(decl.getName(), std::make_pair(llvm::StructType::get(ctx), &decl));
    } else {
        auto* structType = llvm::StructType::create(ctx, decl.getName());
        structs.emplace(decl.getName(), std::make_pair(structType, &decl));
        structType->setBody(getFieldTypes(decl));
    }

    auto insertBlockBackup = builder.GetInsertBlock();
    auto insertPointBackup = builder.GetInsertPoint();

    for (auto& memberDecl : decl.getMemberDecls()) {
        codegenDecl(*memberDecl);
    }

    if (insertBlockBackup) builder.SetInsertPoint(insertBlockBackup, insertPointBackup);
}

llvm::Type* IRGenerator::codegenGenericTypeInstantiation(const TypeDecl& decl, llvm::ArrayRef<Type> genericArgs) {
    std::vector<Type> resolvedGenericArgs;
    resolvedGenericArgs.reserve(genericArgs.size());
    for (Type type : genericArgs) {
        resolvedGenericArgs.push_back(resolve(type));
    }
    genericArgs = resolvedGenericArgs;

    auto name = mangle(decl, genericArgs);

    if (decl.getFields().empty()) {
        auto value = std::make_pair(llvm::StructType::get(ctx), &decl);
        return structs.emplace(name, std::move(value)).first->second.first;
    }

    SAVE_STATE(currentGenericArgs);
    setCurrentGenericArgs(decl.getGenericParams(), genericArgs);
    auto elements = getFieldTypes(decl);

    auto value = std::make_pair(llvm::StructType::create(elements, name), &decl);
    return structs.emplace(name, std::move(value)).first->second.first;
}

void IRGenerator::codegenVarDecl(const VarDecl& decl) {
    if (globalScope().getLocalValues().find(decl.getName()) != globalScope().getLocalValues().end()) return;

    llvm::Value* value = decl.getInitializer() ? codegenExpr(*decl.getInitializer()) : nullptr;

    if (!value || decl.getType().isMutable() /* || decl.isPublic() */) {
        auto linkage = value ? llvm::GlobalValue::PrivateLinkage : llvm::GlobalValue::ExternalLinkage;
        auto initializer = value ? llvm::cast<llvm::Constant>(value) : nullptr;
        value = new llvm::GlobalVariable(module, toIR(decl.getType()), !decl.getType().isMutable(),
                                         linkage, initializer, decl.getName());
    }

    globalScope().addLocalValue(decl.getName(), value);
}

void IRGenerator::codegenDecl(const Decl& decl) {
    SAVE_STATE(currentDecl);
    currentDecl = &decl;

    switch (decl.getKind()) {
        case DeclKind::ParamDecl: llvm_unreachable("handled via FunctionDecl");
        case DeclKind::FunctionDecl:
        case DeclKind::MethodDecl: codegenFunctionDecl(llvm::cast<FunctionDecl>(decl)); break;
        case DeclKind::GenericParamDecl: llvm_unreachable("cannot codegen generic parameter declaration");
        case DeclKind::InitDecl: codegenInitDecl(llvm::cast<InitDecl>(decl)); break;
        case DeclKind::DeinitDecl: codegenFunctionDecl(llvm::cast<DeinitDecl>(decl)); break;
        case DeclKind::TypeDecl: codegenTypeDecl(llvm::cast<TypeDecl>(decl)); break;
        case DeclKind::VarDecl: codegenVarDecl(llvm::cast<VarDecl>(decl)); break;
        case DeclKind::FieldDecl: llvm_unreachable("handled via TypeDecl");
        case DeclKind::ImportDecl: break;
    }
}

llvm::Module& IRGenerator::compile(const Module& sourceModule) {
    for (const auto& sourceFile : sourceModule.getSourceFiles()) {
        setTypeChecker(TypeChecker(const_cast<Module*>(&sourceModule),
                                   const_cast<SourceFile*>(&sourceFile)));

        for (const auto& decl : sourceFile.getTopLevelDecls()) {
            codegenDecl(*decl);
        }
    }

    while (true) {
        auto currentFunctionInstantiations = functionInstantiations;

        for (auto& p : currentFunctionInstantiations) {
            if (p.second.getDecl().isExtern() || !p.second.getFunction()->empty()) continue;

            setTypeChecker(TypeChecker(const_cast<Module*>(&sourceModule), nullptr));

            SAVE_STATE(currentGenericArgs);
            setCurrentGenericArgs(p.second.getDecl().getGenericParams(), p.second.getGenericArgs());
            if (p.second.getDecl().getTypeDecl() != nullptr) {
                setCurrentGenericArgs(p.second.getDecl().getTypeDecl()->getGenericParams(),
                                      p.second.getReceiverTypeGenericArgs());
            }
            codegenFunctionBody(p.second.getDecl(), *p.second.getFunction());
            ASSERT(!llvm::verifyFunction(*p.second.getFunction(), &llvm::errs()));
        }

        if (functionInstantiations.size() == currentFunctionInstantiations.size()) break;
    }

    ASSERT(!llvm::verifyModule(module, &llvm::errs()));
    return module;
}

llvm::LLVMContext& irgen::getContext() {
    return ctx;
}
