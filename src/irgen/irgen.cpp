#include "irgen.h"
#include <iterator>
#include <memory>
#include <vector>
#pragma warning(push, 0)
#include <llvm/ADT/STLExtras.h>
#include <llvm/ADT/StringMap.h>
#include <llvm/ADT/StringSwitch.h>
#include <llvm/IR/CFG.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/ErrorHandling.h>
#pragma warning(pop)
#include "../ast/decl.h"
#include "../ast/expr.h"
#include "../ast/mangle.h"
#include "../ast/module.h"
#include "../ast/token.h"
#include "../sema/typecheck.h"
#include "../support/utility.h"

using namespace delta;

namespace {

/// Helper for storing parameter name info in 'functionInstantiations' key strings.
template<typename T>
std::string mangleWithParams(const T& decl) {
    std::string result;

    if (decl.isMutating() && !decl.isInitDecl() && !decl.isDeinitDecl()) {
        result += "mutating ";
    }

    result += mangle(decl);

    for (auto& param : decl.getParams()) {
        result.append("$").append(param.getName());
        result.append(":").append(param.getType().toString());
    }

    return result;
}

} // namespace

void Scope::onScopeEnd() {
    for (const Expr* expr : llvm::reverse(deferredExprs)) {
        irGenerator->codegenExpr(*expr);
    }

    for (auto& p : llvm::reverse(deinitsToCall)) {
        if (p.decl && p.decl->hasBeenMoved()) continue;
        irGenerator->createDeinitCall(p.function, p.value, p.type, p.decl);
    }
}

void Scope::clear() {
    deferredExprs.clear();
    deinitsToCall.clear();
}

IRGenerator::IRGenerator() : builder(ctx), module("", ctx) {
    scopes.push_back(Scope(*this));
}

/// @param type The Delta type of the variable, or null if the variable is 'this'.
void IRGenerator::setLocalValue(Type type, std::string name, llvm::Value* value, const Decl* decl) {
    scopes.back().addLocalValue(std::move(name), value);

    if (type) {
        deferDeinitCall(value, type, decl);
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

    if (value) {
        return value;
    }

    ASSERT(decl);

    switch (decl->getKind()) {
        case DeclKind::VarDecl:
            return codegenVarDecl(*llvm::cast<VarDecl>(decl));

        case DeclKind::FieldDecl:
            return codegenMemberAccess(findValue("this", nullptr), llvm::cast<FieldDecl>(decl)->getType(),
                                       llvm::cast<FieldDecl>(decl)->getName());

        case DeclKind::FunctionDecl:
            return getFunctionProto(*llvm::cast<FunctionDecl>(decl));

        default:
            llvm_unreachable("all cases handled");
    }
}

llvm::Type* IRGenerator::getBuiltinType(llvm::StringRef name) {
    return llvm::StringSwitch<llvm::Type*>(name)
        .Case("void", llvm::Type::getVoidTy(ctx))
        .Case("bool", llvm::Type::getInt1Ty(ctx))
        .Case("char", llvm::Type::getInt8Ty(ctx))
        .Case("int", llvm::Type::getInt32Ty(ctx))
        .Case("int8", llvm::Type::getInt8Ty(ctx))
        .Case("int16", llvm::Type::getInt16Ty(ctx))
        .Case("int32", llvm::Type::getInt32Ty(ctx))
        .Case("int64", llvm::Type::getInt64Ty(ctx))
        .Case("uint", llvm::Type::getInt32Ty(ctx))
        .Case("uint8", llvm::Type::getInt8Ty(ctx))
        .Case("uint16", llvm::Type::getInt16Ty(ctx))
        .Case("uint32", llvm::Type::getInt32Ty(ctx))
        .Case("uint64", llvm::Type::getInt64Ty(ctx))
        .Case("float", llvm::Type::getFloatTy(ctx))
        .Case("float32", llvm::Type::getFloatTy(ctx))
        .Case("float64", llvm::Type::getDoubleTy(ctx))
        .Case("float80", llvm::Type::getX86_FP80Ty(ctx))
        .Default(nullptr);
}

llvm::Type* IRGenerator::toIR(Type type, SourceLocation location) {
    switch (type.getKind()) {
        case TypeKind::BasicType: {
            if (auto* builtinType = getBuiltinType(type.getName())) return builtinType;

            auto name = mangleTypeDecl(type.getName(), type.getGenericArgs());
            auto it = structs.find(name);
            if (it != structs.end()) return it->second.first;

            auto& basicType = llvm::cast<BasicType>(*type);
            if (auto* enumDecl = llvm::dyn_cast<EnumDecl>(basicType.getDecl())) {
                return toIR(enumDecl->getUnderlyingType());
            }

            return codegenTypeDecl(*basicType.getDecl());
        }
        case TypeKind::ArrayType:
            ASSERT(type.isArrayWithConstantSize(), "unimplemented");
            return llvm::ArrayType::get(toIR(type.getElementType(), location), type.getArraySize());
        case TypeKind::TupleType: {
            auto elementTypes = map(type.getTupleElements(),
                                    [&](const TupleElement& element) { return toIR(element.type); });
            return llvm::StructType::get(ctx, elementTypes);
        }
        case TypeKind::FunctionType: {
            auto paramTypes = map(type.getParamTypes(), [&](Type type) { return toIR(type); });
            auto* returnType = toIR(type.getReturnType());
            return llvm::FunctionType::get(returnType, paramTypes, false)->getPointerTo();
        }
        case TypeKind::PointerType: {
            if (type.getPointee().isArrayWithRuntimeSize()) {
                return toIR(BasicType::get("ArrayRef", type.getPointee().getElementType()), location);
            } else if (type.getPointee().isArrayWithUnknownSize()) {
                return llvm::PointerType::get(toIR(type.getPointee().getElementType(), location), 0);
            }

            auto* pointeeType = toIR(type.getPointee(), location);
            return llvm::PointerType::get(pointeeType->isVoidTy() ? llvm::Type::getInt8Ty(ctx) : pointeeType, 0);
        }
        case TypeKind::OptionalType:
            if (type.getWrappedType().isPointerType()) {
                return toIR(type.getWrappedType());
            }
            llvm_unreachable("IRGen doesn't support non-pointer optional types yet");
    }
    llvm_unreachable("all cases handled");
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

/// Returns a deinitializer that only calls the deinitializers of the member variables, or null if
/// no such deinitializer is needed because none of the member variables have deinitializers.
DeinitDecl* IRGenerator::getDefaultDeinitializer(const TypeDecl& typeDecl) {
    ASSERT(typeDecl.getDeinitializer() == nullptr);

    for (auto& field : typeDecl.getFields()) {
        if (field.getType().getDeinitializer() != nullptr) {
            auto deinitializer = llvm::make_unique<DeinitDecl>(const_cast<TypeDecl&>(typeDecl), typeDecl.getLocation());
            deinitializer->setBody({});
            helperDecls.push_back(std::move(deinitializer));
            return llvm::cast<DeinitDecl>(helperDecls.back().get());
        }
    }

    return nullptr;
}

void IRGenerator::deferDeinitCall(llvm::Value* valueToDeinit, Type type, const Decl* decl) {
    llvm::Function* proto = nullptr;

    if (auto* deinitializer = type.getDeinitializer()) {
        proto = getFunctionProto(*deinitializer);
    } else if (auto* typeDecl = type.getDecl()) {
        if (auto deinitializer = getDefaultDeinitializer(*typeDecl)) {
            proto = getFunctionProto(*deinitializer);
        }
    }

    if (proto) {
        scopes.back().addDeinitToCall(proto, valueToDeinit, type, decl);
    }
}

void IRGenerator::codegenDeferredExprsAndDeinitCallsForReturn() {
    for (auto& scope : llvm::reverse(scopes)) {
        scope.onScopeEnd();
    }
    scopes.back().clear();
}

void IRGenerator::codegenReturnStmt(const ReturnStmt& stmt) {
    // TODO: Emit deferred expressions and deinitializer calls after the evaluation of the return
    // value, but before emitting the return instruction. The return value expression may depend on
    // the values that the deferred expressions and/or deinitializer calls could deallocate.
    codegenDeferredExprsAndDeinitCallsForReturn();

    if (auto* returnValue = stmt.getReturnValue()) {
        builder.CreateRet(codegenExprForPassing(*returnValue, builder.getCurrentFunctionReturnType()));
    } else {
        if (!currentDecl->isMain()) {
            builder.CreateRetVoid();
        } else {
            builder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), 0));
        }
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

    auto* llvmType = toIR(type, decl ? decl->getLocation() : SourceLocation());
    auto* alloca = builder.CreateAlloca(llvmType, arraySize, name);
    lastAlloca = alloca->getIterator();
    auto nameString = name.str();
    if (!nameString.empty()) {
        setLocalValue(type, std::move(nameString), alloca, decl);
    }
    builder.SetInsertPoint(insertBlock);
    return alloca;
}

void IRGenerator::codegenVarStmt(const VarStmt& stmt) {
    auto* alloca = createEntryBlockAlloca(stmt.getDecl().getType(), &stmt.getDecl(), nullptr, stmt.getDecl().getName());
    auto* initializer = stmt.getDecl().getInitializer();

    if (auto* callExpr = llvm::dyn_cast<CallExpr>(initializer)) {
        if (callExpr->getCalleeDecl()) {
            if (auto* initDecl = llvm::dyn_cast<InitDecl>(callExpr->getCalleeDecl())) {
                if (initDecl->getTypeDecl()->getType() == stmt.getDecl().getType().asImmutable()) {
                    codegenCallExpr(*callExpr, alloca);
                    return;
                }
            }
        }
    }

    if (!initializer->isUndefinedLiteralExpr()) {
        builder.CreateStore(codegenExprForPassing(*initializer, alloca->getAllocatedType()), alloca);
    }
}

void IRGenerator::codegenBlock(llvm::ArrayRef<std::unique_ptr<Stmt>> stmts, llvm::BasicBlock* destination,
                               llvm::BasicBlock* continuation) {
    builder.SetInsertPoint(destination);

    beginScope();
    for (const auto& stmt : stmts) {
        codegenStmt(*stmt);
        if (stmt->isReturnStmt() || stmt->isBreakStmt()) break;
    }
    endScope();

    llvm::BasicBlock* insertBlock = builder.GetInsertBlock();
    if (insertBlock->empty() ||
        (!llvm::isa<llvm::ReturnInst>(insertBlock->back()) && !llvm::isa<llvm::BranchInst>(insertBlock->back())))
        builder.CreateBr(continuation);
}

void IRGenerator::codegenIfStmt(const IfStmt& ifStmt) {
    auto* condition = codegenExpr(ifStmt.getCondition());
    if (condition->getType()->isPointerTy()) {
        condition = codegenImplicitNullComparison(condition);
    }
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
        auto* value = codegenExpr(*switchCase.getValue());
        auto* block = llvm::BasicBlock::Create(ctx, "", function);
        return std::make_pair(llvm::cast<llvm::ConstantInt>(value), block);
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
    continueTargets.push_back(condition);
    builder.CreateBr(condition);

    builder.SetInsertPoint(condition);
    auto* conditionValue = codegenExpr(whileStmt.getCondition());
    if (conditionValue->getType()->isPointerTy()) {
        conditionValue = codegenImplicitNullComparison(conditionValue);
    }
    builder.CreateCondBr(conditionValue, body, end);
    codegenBlock(whileStmt.getBody(), body, condition);

    breakTargets.pop_back();
    continueTargets.pop_back();
    builder.SetInsertPoint(end);
}

void IRGenerator::codegenBreakStmt(const BreakStmt&) {
    ASSERT(!breakTargets.empty());
    builder.CreateBr(breakTargets.back());
}

void IRGenerator::codegenContinueStmt(const ContinueStmt&) {
    ASSERT(!continueTargets.empty());
    builder.CreateBr(continueTargets.back());
}

llvm::Value* IRGenerator::codegenAssignmentLHS(const Expr* lhs, const Expr* rhs) {
    if (auto* initDecl = llvm::dyn_cast<InitDecl>(currentDecl)) {
        if (auto* varExpr = llvm::dyn_cast<VarExpr>(lhs)) {
            if (auto* fieldDecl = llvm::dyn_cast<FieldDecl>(varExpr->getDecl())) {
                if (fieldDecl->getParent() == initDecl->getTypeDecl()) {
                    return rhs->isUndefinedLiteralExpr() ? nullptr : codegenLvalueExpr(*lhs);
                }
            }
        }
    }

    if (auto* basicType = llvm::dyn_cast<BasicType>(lhs->getType().getBase())) {
        if (auto* typeDecl = basicType->getDecl()) {
            if (auto* deinit = typeDecl->getDeinitializer()) {
                llvm::Value* value = codegenLvalueExpr(*lhs);
                createDeinitCall(getFunctionProto(*deinit), value, lhs->getType(), typeDecl);
                return rhs->isUndefinedLiteralExpr() ? nullptr : value;
            }
        }
    }

    return rhs->isUndefinedLiteralExpr() ? nullptr : codegenLvalueExpr(*lhs);
}

void IRGenerator::codegenCompoundStmt(const CompoundStmt& stmt) {
    beginScope();

    for (auto& substmt : stmt.getBody()) {
        codegenStmt(*substmt);
    }

    endScope();
}

void IRGenerator::codegenStmt(const Stmt& stmt) {
    switch (stmt.getKind()) {
        case StmtKind::ReturnStmt:
            codegenReturnStmt(llvm::cast<ReturnStmt>(stmt));
            break;
        case StmtKind::VarStmt:
            codegenVarStmt(llvm::cast<VarStmt>(stmt));
            break;
        case StmtKind::ExprStmt:
            codegenExpr(llvm::cast<ExprStmt>(stmt).getExpr());
            break;
        case StmtKind::DeferStmt:
            deferEvaluationOf(llvm::cast<DeferStmt>(stmt).getExpr());
            break;
        case StmtKind::IfStmt:
            codegenIfStmt(llvm::cast<IfStmt>(stmt));
            break;
        case StmtKind::SwitchStmt:
            codegenSwitchStmt(llvm::cast<SwitchStmt>(stmt));
            break;
        case StmtKind::WhileStmt:
            codegenWhileStmt(llvm::cast<WhileStmt>(stmt));
            break;
        case StmtKind::ForStmt:
            llvm_unreachable("ForStmt should be lowered into a WhileStmt");
            break;
        case StmtKind::BreakStmt:
            codegenBreakStmt(llvm::cast<BreakStmt>(stmt));
            break;
        case StmtKind::ContinueStmt:
            codegenContinueStmt(llvm::cast<ContinueStmt>(stmt));
            break;
        case StmtKind::CompoundStmt:
            codegenCompoundStmt(llvm::cast<CompoundStmt>(stmt));
            break;
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

llvm::Type* IRGenerator::getLLVMTypeForPassing(const TypeDecl& typeDecl, bool isMutating) {
    auto* structType = toIR(typeDecl.getType());

    if (!isMutating && typeDecl.passByValue()) {
        return structType;
    } else {
        return llvm::PointerType::get(structType, 0);
    }
}

llvm::Function* IRGenerator::getFunctionProto(const FunctionDecl& decl, std::string&& mangledName) {
    auto it = functionInstantiations.find(mangleWithParams(decl));
    if (it != functionInstantiations.end()) return it->second.getFunction();

    auto* functionType = decl.getFunctionType();
    llvm::SmallVector<llvm::Type*, 16> paramTypes;

    if (decl.isMethodDecl()) {
        paramTypes.emplace_back(getLLVMTypeForPassing(*decl.getTypeDecl(), decl.isMutating()));
    }

    for (auto& paramType : functionType->getParamTypes()) {
        paramTypes.emplace_back(toIR(paramType));
    }

    auto* returnType = toIR(functionType->getReturnType());
    if (decl.isMain() && returnType->isVoidTy()) returnType = llvm::Type::getInt32Ty(ctx);

    auto* llvmFunctionType = llvm::FunctionType::get(returnType, paramTypes, decl.isVariadic());
    if (mangledName.empty()) mangledName = mangle(decl);

    if (decl.isExtern()) {
        if (auto* function = module.getFunction(mangledName)) return function;
    }

    auto* function = llvm::Function::Create(llvmFunctionType, llvm::Function::ExternalLinkage, mangledName, &module);

    auto arg = function->arg_begin(), argsEnd = function->arg_end();
    if (decl.isMethodDecl()) arg++->setName("this");

    ASSERT(decl.getParams().size() == size_t(std::distance(arg, argsEnd)));
    for (auto param = decl.getParams().begin(); arg != argsEnd; ++param, ++arg) {
        arg->setName(param->getName());
    }

    auto mangled = mangleWithParams(decl);
    return functionInstantiations.try_emplace(std::move(mangled), FunctionInstantiation(decl, function)).first->second.getFunction();
}

llvm::Value* IRGenerator::getFunctionForCall(const CallExpr& call) {
    if (!call.callsNamedFunction()) {
        error(call.getLocation(), "anonymous function calls not implemented yet");
    }

    const Decl* decl = call.getCalleeDecl();
    if (!decl) return nullptr;

    switch (decl->getKind()) {
        case DeclKind::FunctionDecl:
        case DeclKind::MethodDecl:
        case DeclKind::InitDecl:
        case DeclKind::DeinitDecl:
            return getFunctionProto(*llvm::cast<FunctionDecl>(decl));
        case DeclKind::VarDecl:
            return findValue(llvm::cast<VarDecl>(decl)->getName(), decl);
        case DeclKind::ParamDecl:
            return findValue(llvm::cast<ParamDecl>(decl)->getName(), decl);
        case DeclKind::FieldDecl:
            if (call.getReceiver()) {
                return codegenMemberAccess(codegenLvalueExpr(*call.getReceiver()), llvm::cast<FieldDecl>(decl)->getType(),
                                           llvm::cast<FieldDecl>(decl)->getName());
            } else {
                return findValue(llvm::cast<FieldDecl>(decl)->getName(), decl);
            }
        default:
            llvm_unreachable("invalid callee decl");
    }
}

void IRGenerator::codegenFunctionBody(const FunctionDecl& decl, llvm::Function& function) {
    builder.SetInsertPoint(llvm::BasicBlock::Create(ctx, "", &function));
    beginScope();
    auto arg = function.arg_begin();
    if (decl.getTypeDecl() != nullptr) setLocalValue(Type(), "this", &*arg++, nullptr);
    for (auto& param : decl.getParams()) {
        setLocalValue(param.getType(), param.getName(), &*arg++, &param);
    }
    if (decl.isDeinitDecl()) {
        for (auto& field : decl.getTypeDecl()->getFields()) {
            if (field.getType().getDeinitializer() == nullptr) continue;
            auto* fieldValue = codegenMemberAccess(function.arg_begin(), field.getType(), field.getName());
            deferDeinitCall(fieldValue, field.getType(), &field);
        }
    }
    for (auto& stmt : decl.getBody()) {
        codegenStmt(*stmt);
    }
    endScope();

    auto* insertBlock = builder.GetInsertBlock();
    if (insertBlock != &function.getEntryBlock() && llvm::pred_empty(insertBlock)) {
        insertBlock->eraseFromParent();
        return;
    }

    if (insertBlock->empty() || !llvm::isa<llvm::ReturnInst>(insertBlock->back())) {
        if (!decl.isMain()) {
            builder.CreateRetVoid();
        } else {
            builder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), 0));
        }
    }
}

void IRGenerator::codegenFunctionDecl(const FunctionDecl& decl) {
    llvm::Function* function = getFunctionProto(decl);
    if (!decl.isExtern()) codegenFunctionBody(decl, *function);
    ASSERT(!llvm::verifyFunction(*function, &llvm::errs()));
}

std::vector<llvm::Type*> IRGenerator::getFieldTypes(const TypeDecl& decl) {
    return map(decl.getFields(), [&](const FieldDecl& field) { return toIR(field.getType(), field.getLocation()); });
}

llvm::StructType* IRGenerator::codegenTypeDecl(const TypeDecl& decl) {
    if (decl.isInterface()) return nullptr;

    llvm::StructType* structType;
    auto it = structs.find(mangle(decl));

    if (it != structs.end()) {
        structType = it->second.first;
    } else {
        if (decl.getFields().empty()) {
            structType = llvm::StructType::get(ctx);
        } else {
            structType = llvm::StructType::create(ctx, mangle(decl));
        }

        structs.try_emplace(mangle(decl), std::make_pair(structType, &decl));
    }

    auto fieldTypes = getFieldTypes(decl);

    if (!structType->isOpaque()) {
        return structType;
    }

    structType->setBody(std::move(fieldTypes));

    auto insertBlockBackup = builder.GetInsertBlock();
    auto insertPointBackup = builder.GetInsertPoint();
    auto scopesBackup = std::move(scopes);

    if (!decl.getFields().empty()) {
        for (llvm::Type* element : structType->elements()) {
            if (auto* elementStruct = llvm::dyn_cast<llvm::StructType>(element)) {
                if (elementStruct->isLiteral()) continue;
                auto it = structs.find(elementStruct->getName());
                if (it != structs.end()) {
                    codegenTypeDecl(*it->second.second);
                }
            }
        }
    }

    for (auto& memberDecl : decl.getMemberDecls()) {
        codegenDecl(*memberDecl);
    }

    if (decl.getDeinitializer() == nullptr) {
        if (auto deinitializer = getDefaultDeinitializer(decl)) {
            codegenDecl(*deinitializer);
        }
    }

    scopes = std::move(scopesBackup);
    if (insertBlockBackup) builder.SetInsertPoint(insertBlockBackup, insertPointBackup);

    return structType;
}

llvm::Value* IRGenerator::codegenVarDecl(const VarDecl& decl) {
    if (auto* value = module.getGlobalVariable(decl.getName(), true)) return value;

    auto it = globalScope().getLocalValues().find(decl.getName());
    if (it != globalScope().getLocalValues().end()) return it->second;

    llvm::Value* value = codegenExpr(*decl.getInitializer());

    if (decl.getType().isMutable() /* || decl.isPublic() */) {
        auto linkage = value ? llvm::GlobalValue::PrivateLinkage : llvm::GlobalValue::ExternalLinkage;
        auto initializer = value ? llvm::cast<llvm::Constant>(value) : nullptr;
        value = new llvm::GlobalVariable(module, toIR(decl.getType()), !decl.getType().isMutable(), linkage,
                                         initializer, decl.getName());
    }

    globalScope().addLocalValue(decl.getName(), value);
    return value;
}

void IRGenerator::codegenDecl(const Decl& decl) {
    SAVE_STATE(currentDecl);
    currentDecl = &decl;

    switch (decl.getKind()) {
        case DeclKind::ParamDecl:
            llvm_unreachable("handled via FunctionDecl");
        case DeclKind::FunctionDecl:
        case DeclKind::MethodDecl:
            codegenFunctionDecl(llvm::cast<FunctionDecl>(decl));
            break;
        case DeclKind::GenericParamDecl:
            llvm_unreachable("cannot codegen generic parameter declaration");
        case DeclKind::InitDecl:
            codegenFunctionDecl(llvm::cast<InitDecl>(decl));
            break;
        case DeclKind::DeinitDecl:
            codegenFunctionDecl(llvm::cast<DeinitDecl>(decl));
            break;
        case DeclKind::FunctionTemplate:
            break;
        case DeclKind::TypeDecl:
            codegenTypeDecl(llvm::cast<TypeDecl>(decl));
            break;
        case DeclKind::TypeTemplate:
            break;
        case DeclKind::EnumDecl:
            break;
        case DeclKind::VarDecl:
            codegenVarDecl(llvm::cast<VarDecl>(decl));
            break;
        case DeclKind::FieldDecl:
            llvm_unreachable("handled via TypeDecl");
        case DeclKind::ImportDecl:
            break;
    }
}

llvm::Module& IRGenerator::compile(const Module& sourceModule) {
    for (const auto& sourceFile : sourceModule.getSourceFiles()) {
        for (const auto& decl : sourceFile.getTopLevelDecls()) {
            codegenDecl(*decl);
        }
    }

    while (true) {
        auto currentFunctionInstantiations = functionInstantiations;

        for (auto& p : currentFunctionInstantiations) {
            if (p.second.getDecl().isExtern() || !p.second.getFunction()->empty()) continue;

            currentDecl = &p.second.getDecl();
            codegenFunctionBody(p.second.getDecl(), *p.second.getFunction());
            ASSERT(!llvm::verifyFunction(*p.second.getFunction(), &llvm::errs()));
        }

        if (functionInstantiations.size() == currentFunctionInstantiations.size()) break;
    }

    ASSERT(!llvm::verifyModule(module, &llvm::errs()));
    return module;
}
