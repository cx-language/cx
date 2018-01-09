#pragma once

#include <unordered_map>
#include <vector>
#pragma warning(push, 0)
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#pragma warning(pop)
#include "../ast/expr.h"
#include "../ast/decl.h"
#include "../ast/stmt.h"
#include "../sema/typecheck.h"

namespace delta {

class Module;
struct Type;
class Typechecker;
class IRGenerator;

namespace irgen {
llvm::Type* getBuiltinType(llvm::StringRef typeName);
llvm::LLVMContext& getContext();
}

struct Scope {
    Scope(IRGenerator& irGenerator) : irGenerator(&irGenerator) {}
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
    IRGenerator* irGenerator;
};

class IRGenerator {
public:
    IRGenerator();

    llvm::Module& compile(const Module& sourceModule);
    llvm::Value* codegenExpr(const Expr& expr);
    llvm::Type* toIR(Type type, SourceLocation location = SourceLocation::invalid());
    llvm::IRBuilder<>& getBuilder() { return builder; }

private:
    friend struct Scope;

    using UnaryCreate = llvm::Value* (llvm::IRBuilder<>::*)(llvm::Value*, const llvm::Twine&, bool, bool);
    using BinaryCreate0 = llvm::Value* (llvm::IRBuilder<>::*)(llvm::Value*, llvm::Value*, const llvm::Twine&);
    using BinaryCreate1 = llvm::Value* (llvm::IRBuilder<>::*)(llvm::Value*, llvm::Value*, const llvm::Twine&, bool);
    using BinaryCreate2 = llvm::Value* (llvm::IRBuilder<>::*)(llvm::Value*, llvm::Value*, const llvm::Twine&, bool, bool);

    void codegenFunctionBody(const FunctionDecl& decl, llvm::Function& function);
    void createDeinitCall(llvm::Function* deinit, llvm::Value* valueToDeinit, Type type, const Decl* decl);
    llvm::Module& getIRModule() { return module; }

    /// @param type The Delta type of the variable, or null if the variable is 'this'.
    void setLocalValue(Type type, std::string name, llvm::Value* value, const Decl* decl);
    llvm::Value* findValue(llvm::StringRef name, const Decl* decl);

    llvm::Value* codegenVarExpr(const VarExpr& expr);
    llvm::Value* codegenLvalueVarExpr(const VarExpr& expr);
    llvm::Value* codegenStringLiteralExpr(const StringLiteralExpr& expr);
    llvm::Value* codegenCharacterLiteralExpr(const CharacterLiteralExpr& expr);
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
    llvm::Value* codegenCallExpr(const CallExpr& expr, llvm::AllocaInst* thisAllocaForInit = nullptr);
    llvm::Value* codegenCastExpr(const CastExpr& expr);
    llvm::Value* codegenSizeofExpr(const SizeofExpr& expr);
    llvm::Value* codegenAddressofExpr(const AddressofExpr& expr);
    llvm::Value* codegenMemberAccess(llvm::Value* baseValue, Type memberType, llvm::StringRef memberName);
    llvm::Value* codegenLvalueMemberExpr(const MemberExpr& expr);
    llvm::Value* codegenMemberExpr(const MemberExpr& expr);
    llvm::Value* codegenLvalueSubscriptExpr(const SubscriptExpr& expr);
    llvm::Value* codegenSubscriptExpr(const SubscriptExpr& expr);
    llvm::Value* codegenUnwrapExpr(const UnwrapExpr& expr);
    llvm::Value* codegenLambdaExpr(const LambdaExpr& expr);
    llvm::Value* codegenIfExpr(const IfExpr& expr);
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
    void codegenBreakStmt(const BreakStmt&);
    llvm::Value* codegenAssignmentLHS(const Expr* lhs, const Expr* rhs);
    void codegenAssignStmt(const AssignStmt& stmt);
    void codegenCompoundStmt(const CompoundStmt& stmt);
    void codegenStmt(const Stmt& stmt);

    void codegenDecl(const Decl& decl);
    void codegenFunctionDecl(const FunctionDecl& decl);
    llvm::StructType* codegenTypeDecl(const TypeDecl& decl);
    llvm::Value* codegenVarDecl(const VarDecl& decl);

    llvm::Value* getFunctionForCall(const CallExpr& call);
    llvm::Function* getFunctionProto(const FunctionDecl& decl, std::string&& mangledName = {});
    llvm::AllocaInst* createEntryBlockAlloca(Type type, const Decl* decl, llvm::Value* arraySize = nullptr,
                                             const llvm::Twine& name = "");
    std::vector<llvm::Type*> getFieldTypes(const TypeDecl& decl);
    llvm::Type* getLLVMTypeForPassing(const TypeDecl& typeDecl, bool isMutating);
    llvm::Value* getArrayLength(const Expr& object, Type objectType);
    llvm::Value* codegenPointerOffset(const BinaryExpr& expr);

    void beginScope();
    void endScope();
    void deferEvaluationOf(const Expr& expr);
    void deferDeinitCall(llvm::Function* deinit, llvm::Value* valueToDeinit, Type type, const Decl* decl);
    Scope& globalScope() { return scopes.front(); }

private:
    class FunctionInstantiation {
    public:
        FunctionInstantiation(const FunctionDecl& decl, llvm::Function* function)
        : decl(decl), function(function) {}
        const FunctionDecl& getDecl() const { return decl; }
        llvm::Function* getFunction() const { return function; }

    private:
        const FunctionDecl& decl;
        llvm::Function* function;
    };

private:
    std::vector<Scope> scopes;

    llvm::IRBuilder<> builder;
    llvm::Module module;

    llvm::StringMap<FunctionInstantiation> functionInstantiations;
    std::vector<std::unique_ptr<Decl>> helperDecls;
    llvm::StringMap<std::pair<llvm::StructType*, const TypeDecl*>> structs;
    const Decl* currentDecl;

    /// The basic blocks to branch to on a 'break' statement, one element per scope.
    llvm::SmallVector<llvm::BasicBlock*, 4> breakTargets;
};

}
