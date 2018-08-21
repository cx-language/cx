#pragma once

#include <unordered_map>
#include <vector>
#pragma warning(push, 0)
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#pragma warning(pop)
#include "../ast/decl.h"
#include "../ast/expr.h"
#include "../ast/stmt.h"
#include "../sema/typecheck.h"

namespace delta {

class Module;
struct Type;
class Typechecker;
class IRGenerator;

struct Scope {
    Scope(IRGenerator& irGenerator) : irGenerator(&irGenerator) {}
    void addDeferredExpr(const Expr& expr) { deferredExprs.emplace_back(&expr); }
    void addDeinitToCall(llvm::Function* deinit, llvm::Value* value, Type type, const Decl* decl) {
        deinitsToCall.emplace_back(DeferredDeinit{ deinit, value, type, decl });
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
    llvm::Type* toIR(Type type, SourceLocation location = SourceLocation());
    llvm::LLVMContext& getLLVMContext() { return ctx; }
    llvm::IRBuilder<>& getBuilder() { return builder; }
    std::vector<std::unique_ptr<llvm::Module>> getGeneratedModules() { return std::move(generatedModules); }

private:
    friend struct Scope;

    using UnaryCreate = llvm::Value* (llvm::IRBuilder<>::*) (llvm::Value*, const llvm::Twine&, bool, bool);
    using BinaryCreate0 = llvm::Value* (llvm::IRBuilder<>::*) (llvm::Value*, llvm::Value*, const llvm::Twine&);
    using BinaryCreate1 = llvm::Value* (llvm::IRBuilder<>::*) (llvm::Value*, llvm::Value*, const llvm::Twine&, bool);
    using BinaryCreate2 = llvm::Value* (llvm::IRBuilder<>::*) (llvm::Value*, llvm::Value*, const llvm::Twine&, bool, bool);

    void codegenFunctionBody(const FunctionDecl& decl, llvm::Function& function);
    void createDeinitCall(llvm::Function* deinit, llvm::Value* valueToDeinit, Type type, const Decl* decl);

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
    llvm::Value* codegenUndefinedLiteralExpr(const UndefinedLiteralExpr& expr);
    llvm::Value* codegenArrayLiteralExpr(const ArrayLiteralExpr& expr);
    llvm::Value* codegenTupleExpr(const TupleExpr& expr);
    llvm::Value* codegenImplicitNullComparison(llvm::Value* operand);
    llvm::Value* codegenNot(const UnaryExpr& expr);
    llvm::Value* codegenUnaryExpr(const UnaryExpr& expr);
    llvm::Value* codegenLvalueUnaryExpr(const UnaryExpr& expr);
    llvm::Value* codegenIncrementExpr(const UnaryExpr& expr);
    llvm::Value* codegenDecrementExpr(const UnaryExpr& expr);
    llvm::Value* codegenBinaryOp(llvm::Value* lhs, llvm::Value* rhs, BinaryCreate0 create);
    llvm::Value* codegenBinaryOp(llvm::Value* lhs, llvm::Value* rhs, BinaryCreate1 create);
    llvm::Value* codegenBinaryOp(llvm::Value* lhs, llvm::Value* rhs, BinaryCreate2 create);
    llvm::Value* codegenLogicalAnd(const Expr& left, const Expr& right);
    llvm::Value* codegenLogicalOr(const Expr& left, const Expr& right);
    llvm::Value* codegenBinaryOp(Token::Kind op, llvm::Value* lhs, llvm::Value* rhs, Type lhsType);
    llvm::Value* codegenShortCircuitBinaryOp(BinaryOperator op, const Expr& lhs, const Expr& rhs);
    llvm::Value* codegenBinaryExpr(const BinaryExpr& expr);
    void codegenAssignment(const BinaryExpr& expr);
    llvm::Value* codegenExprForPassing(const Expr& expr, llvm::Type* targetType);
    llvm::Value* codegenBuiltinConversion(const Expr& expr, Type type);
    void codegenAssert(llvm::Value* condition, SourceLocation location);
    llvm::Value* codegenCallExpr(const CallExpr& expr, llvm::AllocaInst* thisAllocaForInit = nullptr);
    llvm::Value* codegenBuiltinCast(const CallExpr& expr);
    llvm::Value* codegenSizeofExpr(const SizeofExpr& expr);
    llvm::Value* codegenAddressofExpr(const AddressofExpr& expr);
    llvm::Value* codegenMemberAccess(llvm::Value* baseValue, Type memberType, llvm::StringRef memberName);
    llvm::Value* codegenLvalueMemberExpr(const MemberExpr& expr);
    llvm::Value* codegenMemberExpr(const MemberExpr& expr);
    llvm::Value* codegenTupleElementAccess(const MemberExpr& expr);
    llvm::Value* codegenLvalueSubscriptExpr(const SubscriptExpr& expr);
    llvm::Value* codegenSubscriptExpr(const SubscriptExpr& expr);
    llvm::Value* codegenUnwrapExpr(const UnwrapExpr& expr);
    llvm::Value* codegenLambdaExpr(const LambdaExpr& expr);
    llvm::Value* codegenIfExpr(const IfExpr& expr);
    llvm::Value* codegenLvalueExpr(const Expr& expr);

    void codegenDeferredExprsAndDeinitCallsForReturn();
    void codegenBlock(llvm::ArrayRef<std::unique_ptr<Stmt>> stmts, llvm::BasicBlock* destination,
                      llvm::BasicBlock* continuation);
    void codegenReturnStmt(const ReturnStmt& stmt);
    void codegenVarStmt(const VarStmt& stmt);
    void codegenIfStmt(const IfStmt& ifStmt);
    void codegenSwitchStmt(const SwitchStmt& switchStmt);
    void codegenWhileStmt(const WhileStmt& whileStmt);
    void codegenBreakStmt(const BreakStmt&);
    void codegenContinueStmt(const ContinueStmt&);
    llvm::Value* codegenAssignmentLHS(const Expr* lhs, const Expr* rhs);
    void codegenCompoundStmt(const CompoundStmt& stmt);
    void codegenStmt(const Stmt& stmt);

    void codegenDecl(const Decl& decl);
    void codegenFunctionDecl(const FunctionDecl& decl);
    llvm::StructType* codegenTypeDecl(const TypeDecl& decl);
    llvm::Value* codegenVarDecl(const VarDecl& decl);

    llvm::Value* getFunctionForCall(const CallExpr& call);
    llvm::Function* getFunctionProto(const FunctionDecl& decl);
    llvm::AllocaInst* createEntryBlockAlloca(Type type, const Decl* decl, llvm::Value* arraySize = nullptr,
                                             const llvm::Twine& name = "");
    llvm::Value* loadIfAlloca(llvm::Value* value);
    std::vector<llvm::Type*> getFieldTypes(const TypeDecl& decl);
    llvm::Type* getBuiltinType(llvm::StringRef name);
    llvm::Type* getLLVMTypeForPassing(const TypeDecl& typeDecl, bool isMutating);
    llvm::Value* getArrayLength(const Expr& object, Type objectType);
    llvm::Value* codegenPointerOffset(const BinaryExpr& expr);

    void beginScope();
    void endScope();
    void deferEvaluationOf(const Expr& expr);
    DeinitDecl* getDefaultDeinitializer(const TypeDecl& typeDecl);
    void deferDeinitCall(llvm::Value* valueToDeinit, Type type, const Decl* decl);
    Scope& globalScope() { return scopes.front(); }

private:
    class FunctionInstantiation {
    public:
        FunctionInstantiation(const FunctionDecl& decl, llvm::Function* function) : decl(decl), function(function) {}
        const FunctionDecl& getDecl() const { return decl; }
        llvm::Function* getFunction() const { return function; }

    private:
        const FunctionDecl& decl;
        llvm::Function* function;
    };

private:
    std::vector<Scope> scopes;

    llvm::LLVMContext ctx;
    llvm::IRBuilder<> builder;
    std::unique_ptr<llvm::Module> module;
    std::vector<std::unique_ptr<llvm::Module>> generatedModules;
    llvm::BasicBlock::iterator lastAlloca;

    llvm::StringMap<FunctionInstantiation> functionInstantiations;
    std::vector<std::unique_ptr<Decl>> helperDecls;
    llvm::StringMap<std::pair<llvm::StructType*, const TypeDecl*>> structs;
    const Decl* currentDecl;

    /// The basic blocks to branch to on a 'break'/'continue' statement.
    llvm::SmallVector<llvm::BasicBlock*, 4> breakTargets;
    llvm::SmallVector<llvm::BasicBlock*, 4> continueTargets;
};

} // namespace delta
