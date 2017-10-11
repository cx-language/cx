#pragma once

#include <vector>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include "../ast/expr.h"
#include "../ast/decl.h"
#include "../ast/stmt.h"
#include "../sema/typecheck.h"

namespace delta {

class Module;
struct Type;
class TypeChecker;
class IRGenerator;

namespace irgen {
llvm::Type* getBuiltinType(llvm::StringRef typeName);
llvm::LLVMContext& getContext();
}

struct Scope {
    Scope(IRGenerator& irGenerator) : irGenerator(irGenerator) {}
    void addDeferredExpr(const Expr& expr) { deferredExprs.emplace_back(&expr); }
    void addDeinitToCall(llvm::Function* deinit, llvm::Value* value, Type type, const Decl* decl) {
        deinitsToCall.emplace_back(DeferredDeinit{deinit, value, type, decl});
    }
    void addLocalValue(std::string&& name, llvm::Value* value) {
        bool didInsert = localValues.emplace(std::move(name), value).second;
        ASSERT(didInsert);
    }
    const std::unordered_map<std::string, llvm::Value*>& getLocalValues() const { return localValues; }
    void onScopeEnd();
    void clear();

private:
    struct DeferredDeinit {
        llvm::Function* function;
        llvm::Value* value;
        Type type;
        const Decl* decl;
    };

    llvm::SmallVector<const Expr*, 8> deferredExprs;
    llvm::SmallVector<DeferredDeinit, 8> deinitsToCall;
    std::unordered_map<std::string, llvm::Value*> localValues;
    IRGenerator& irGenerator;
};

class IRGenerator : public TypeResolver {
public:
    IRGenerator();

    const TypeChecker& getTypeChecker() const { return *currentTypeChecker; }
    void setTypeChecker(TypeChecker&& typeChecker) { currentTypeChecker = std::move(typeChecker); }
    llvm::Module& compile(const Module& sourceModule);
    llvm::Value* codegenExpr(const Expr& expr);
    llvm::Type* toIR(Type type);
    llvm::IRBuilder<>& getBuilder() { return builder; }
    Type resolveTypePlaceholder(llvm::StringRef name) const override;

private:
    friend struct Scope;

    using UnaryCreate = llvm::Value* (llvm::IRBuilder<>::*)(llvm::Value*, const llvm::Twine&, bool, bool);
    using BinaryCreate0 = llvm::Value* (llvm::IRBuilder<>::*)(llvm::Value*, llvm::Value*, const llvm::Twine&);
    using BinaryCreate1 = llvm::Value* (llvm::IRBuilder<>::*)(llvm::Value*, llvm::Value*, const llvm::Twine&, bool);
    using BinaryCreate2 = llvm::Value* (llvm::IRBuilder<>::*)(llvm::Value*, llvm::Value*, const llvm::Twine&, bool, bool);

    void setCurrentGenericArgs(llvm::ArrayRef<GenericParamDecl> genericParams,
                               llvm::ArrayRef<Type> genericArgs);
    void codegenFunctionBody(const FunctionDecl& decl, llvm::Function& function);
    void createDeinitCall(llvm::Function* deinit, llvm::Value* valueToDeinit, Type type, const Decl* decl);
    llvm::Module& getIRModule() { return module; }

    llvm::Function* getFunction(Type  receiverType, llvm::StringRef functionName,
                                llvm::ArrayRef<Type> functionGenericArgs);
    /// @param type The Delta type of the variable, or null if the variable is 'this'.
    void setLocalValue(Type type, std::string name, llvm::Value* value, const Decl* decl);
    llvm::Value* findValue(llvm::StringRef name, const Decl* decl);

    llvm::Value* codegenVarExpr(const VarExpr& expr);
    llvm::Value* codegenLvalueVarExpr(const VarExpr& expr);
    llvm::Value* codegenStringLiteralExpr(const StringLiteralExpr& expr);
    llvm::Value* codegenIntLiteralExpr(const IntLiteralExpr& expr);
    llvm::Value* codegenFloatLiteralExpr(const FloatLiteralExpr& expr);
    llvm::Value* codegenBoolLiteralExpr(const BoolLiteralExpr& expr);
    llvm::Value* codegenNullLiteralExpr(const NullLiteralExpr& expr);
    llvm::Value* codegenArrayLiteralExpr(const ArrayLiteralExpr& expr);
    llvm::Value* codegenNot(const PrefixExpr& expr);
    llvm::Value* codegenPrefixExpr(const PrefixExpr& expr);
    llvm::Value* codegenLvaluePrefixExpr(const PrefixExpr& expr);
    llvm::Value* codegenBinaryOp(llvm::Value* lhs, llvm::Value* rhs, BinaryCreate0 create);
    llvm::Value* codegenBinaryOp(llvm::Value* lhs, llvm::Value* rhs, BinaryCreate1 create);
    llvm::Value* codegenBinaryOp(llvm::Value* lhs, llvm::Value* rhs, BinaryCreate2 create);
    llvm::Value* codegenLogicalAnd(const Expr& left, const Expr& right);
    llvm::Value* codegenLogicalOr(const Expr& left, const Expr& right);
    llvm::Value* codegenBinaryOp(BinaryOperator op, llvm::Value* lhs, llvm::Value* rhs, const Expr& leftExpr);
    llvm::Value* codegenShortCircuitBinaryOp(BinaryOperator op, const Expr& lhs, const Expr& rhs);
    llvm::Value* codegenBinaryExpr(const BinaryExpr& expr);
    llvm::Value* codegenExprForPassing(const Expr& expr, llvm::Type* targetType);
    llvm::Value* codegenBuiltinConversion(const Expr& expr, Type type);
    llvm::Value* codegenCallExpr(const CallExpr& expr);
    llvm::Value* codegenCastExpr(const CastExpr& expr);
    llvm::Value* codegenMemberAccess(llvm::Value* baseValue, Type memberType, llvm::StringRef memberName);
    llvm::Value* codegenLvalueMemberExpr(const MemberExpr& expr);
    llvm::Value* codegenMemberExpr(const MemberExpr& expr);
    llvm::Value* codegenLvalueSubscriptExpr(const SubscriptExpr& expr);
    llvm::Value* codegenSubscriptExpr(const SubscriptExpr& expr);
    llvm::Value* codegenUnwrapExpr(const UnwrapExpr& expr);
    llvm::Value* codegenLvalueExpr(const Expr& expr);

    void codegenDeferredExprsAndDeinitCallsForReturn();
    void codegenBlock(llvm::ArrayRef<std::unique_ptr<Stmt>> stmts,
                      llvm::BasicBlock* destination, llvm::BasicBlock* continuation);
    void codegenReturnStmt(const ReturnStmt& stmt);
    void codegenVarStmt(const VarStmt& stmt);
    void codegenIncrementStmt(const IncrementStmt& stmt);
    void codegenDecrementStmt(const DecrementStmt& stmt);
    void codegenIfStmt(const IfStmt& ifStmt);
    void codegenSwitchStmt(const SwitchStmt& switchStmt);
    void codegenWhileStmt(const WhileStmt& whileStmt);
    void codegenForStmt(const ForStmt& forStmt);
    void codegenBreakStmt(const BreakStmt&);
    void codegenAssignStmt(const AssignStmt& stmt);
    void codegenStmt(const Stmt& stmt);

    void codegenDecl(const Decl& decl);
    void codegenFunctionDecl(const FunctionDecl& decl);
    void codegenInitDecl(const InitDecl& decl, llvm::ArrayRef<Type> typeGenericArgs = {});
    void codegenTypeDecl(const TypeDecl& decl);
    void codegenVarDecl(const VarDecl& decl);

    llvm::Function* getFunctionForCall(const CallExpr& call);
    llvm::Function* getFunctionProto(const FunctionDecl& decl, llvm::ArrayRef<Type> functionGenericArgs = {},
                                     Type receiverType = nullptr, std::string&& mangledName = {});
    llvm::AllocaInst* createEntryBlockAlloca(Type type, const Decl* decl, llvm::Value* arraySize = nullptr,
                                             const llvm::Twine& name = "");
    std::vector<llvm::Type*> getFieldTypes(const TypeDecl& decl);
    llvm::Type* getLLVMTypeForPassing(const TypeDecl& typeDecl, llvm::ArrayRef<Type> genericArgs,
                                      bool isMutating);
    llvm::Type* codegenGenericTypeInstantiation(const TypeDecl& decl, llvm::ArrayRef<Type> genericArgs);
    llvm::Value* getArrayDataPointer(const Expr& object, Type objectType);
    llvm::Value* getArrayLength(const Expr& object, Type objectType);
    llvm::Value* codegenOffsetUnsafely(const CallExpr& call);

    void beginScope();
    void endScope();
    void deferEvaluationOf(const Expr& expr);
    void deferDeinitCall(llvm::Function* deinit, llvm::Value* valueToDeinit, Type type, const Decl* decl);
    Scope& globalScope() { return scopes.front(); }

private:
    class FunctionInstantiation {
    public:
        FunctionInstantiation(const FunctionDecl& decl, llvm::ArrayRef<Type> receiverTypeGenericArgs,
                              std::vector<Type> genericArgs, llvm::Function* function)
        : decl(decl), receiverTypeGenericArgs(receiverTypeGenericArgs), genericArgs(std::move(genericArgs)),
          function(function) {}
        const FunctionDecl& getDecl() const { return decl; }
        llvm::ArrayRef<Type> getReceiverTypeGenericArgs() const { return receiverTypeGenericArgs; }
        llvm::ArrayRef<Type> getGenericArgs() const { return genericArgs; }
        llvm::Function* getFunction() const { return function; }

    private:
        const FunctionDecl& decl;
        llvm::ArrayRef<Type> receiverTypeGenericArgs;
        std::vector<Type> genericArgs;
        llvm::Function* function;
    };

private:
    llvm::Optional<TypeChecker> currentTypeChecker;
    llvm::SmallVector<Scope, 4> scopes;

    llvm::IRBuilder<> builder;
    llvm::Module module;

    std::unordered_map<std::string, FunctionInstantiation> functionInstantiations;
    std::vector<std::unique_ptr<FunctionDecl>> helperDecls;
    std::unordered_map<std::string, std::pair<llvm::StructType*, const TypeDecl*>> structs;
    std::unordered_map<std::string, Type> currentGenericArgs;
    const Decl* currentDecl;

    /// The basic blocks to branch to on a 'break' statement, one element per scope.
    llvm::SmallVector<llvm::BasicBlock*, 4> breakTargets;
};

}
