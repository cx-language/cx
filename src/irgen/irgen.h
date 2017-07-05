#pragma once

#include <llvm/IR/IRBuilder.h>
#include "../ast/expr.h"
#include "../ast/decl.h"
#include "../ast/stmt.h"

namespace delta {

class Module;
struct Type;
class TypeChecker;
class IRGenerator;

namespace irgen {
    llvm::Module& compile(const Module& module);
    llvm::Type* getBuiltinType(llvm::StringRef typeName);
    llvm::Module& getIRModule();
    llvm::IRBuilder<>& getBuilder();
    llvm::LLVMContext& getContext();
}

struct Scope {
    Scope(IRGenerator& irGenerator) : irGenerator(irGenerator) { }

    llvm::SmallVector<const Expr*, 8> deferredExprs;
    llvm::SmallVector<std::pair<llvm::Function*, llvm::Value*>, 8> deinitsToCall;
    std::unordered_map<std::string, llvm::Value*> localValues;

    void onScopeEnd();
    void clear();
    
private:
    IRGenerator& irGenerator;
};

class IRGenerator {
public:
    IRGenerator(const TypeChecker* typeChecker);

    void codegenDecl(const Decl& decl);
    llvm::Value* codegenExpr(const Expr& expr);
    void setCurrentGenericArgs(llvm::ArrayRef<GenericParamDecl> genericParams,
                               llvm::ArrayRef<Type> genericArgs);
    void codegenFuncBody(const FuncDecl& decl, llvm::Function& func);
    void createDeinitCall(llvm::Function* deinit, llvm::Value* valueToDeinit);
    llvm::Type* toIR(Type type);

private:
    using UnaryCreate = llvm::Value* (llvm::IRBuilder<>::*)(llvm::Value*, const llvm::Twine&, bool, bool);
    using BinaryCreate0 = llvm::Value* (llvm::IRBuilder<>::*)(llvm::Value*, llvm::Value*, const llvm::Twine&);
    using BinaryCreate1 = llvm::Value* (llvm::IRBuilder<>::*)(llvm::Value*, llvm::Value*, const llvm::Twine&, bool);
    using BinaryCreate2 = llvm::Value* (llvm::IRBuilder<>::*)(llvm::Value*, llvm::Value*, const llvm::Twine&, bool, bool);

    llvm::Function* getDeinitializerFor(Type type);
    /// @param type The Delta type of the variable, or null if the variable is 'this'.
    void setLocalValue(Type type, std::string name, llvm::Value* value);
    llvm::Value* findValue(llvm::StringRef name, const Decl* decl);

    llvm::Value* codegenVarExpr(const VarExpr& expr);
    llvm::Value* codegenLvalueVarExpr(const VarExpr& expr);
    llvm::Value* codegenStrLiteralExpr(const StrLiteralExpr& expr);
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
    void codegenAugAssignStmt(const AugAssignStmt& stmt);
    void codegenStmt(const Stmt& stmt);
    void codegenFuncDecl(const FuncDecl& decl);
    void codegenInitDecl(const InitDecl& decl, llvm::ArrayRef<Type> typeGenericArgs = {});
    void codegenDeinitDecl(const DeinitDecl& decl);
    void codegenTypeDecl(const TypeDecl& decl);
    void codegenVarDecl(const VarDecl& decl);

    llvm::Function* getFuncForCall(const CallExpr& call);
    llvm::Function* getFuncProto(const FuncDecl& decl, llvm::ArrayRef<Type> funcGenericArgs = {},
                                 Expr* receiver = nullptr, std::string&& mangledName = {});
    llvm::Function* getInitProto(const InitDecl& decl, llvm::ArrayRef<Type> typeGenericArgs = {},
                                 llvm::ArrayRef<Type> funcGenericArgs = {});
    llvm::Function* codegenDeinitializerProto(const DeinitDecl& decl);
    llvm::AllocaInst* createEntryBlockAlloca(Type type, llvm::Value* arraySize = nullptr,
                                             const llvm::Twine& name = "");
    std::vector<llvm::Type*> getFieldTypes(const TypeDecl& decl);
    llvm::Type* codegenGenericTypeInstantiation(const TypeDecl& decl, llvm::ArrayRef<Type> genericArgs);
    llvm::Value* getArrayOrStringDataPtr(const Expr& object, Type objectType);
    llvm::Value* getArrayOrStringLength(const Expr& object, Type objectType);

    void beginScope();
    void endScope();
    void deferEvaluationOf(const Expr& expr);
    void deferDeinitCall(llvm::Function* deinit, llvm::Value* valueToDeinit);
    Scope& globalScope() { return scopes.front(); }

private:
    const TypeChecker* const currentTypeChecker;
    llvm::SmallVector<Scope, 4> scopes;
};

}
