#pragma once

#include <map>
#include <vector>
#pragma warning(push, 0)
#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/StringMap.h>
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

struct IRGenScope {
    IRGenScope(IRGenerator& irGenerator) : irGenerator(&irGenerator) {}
    void onScopeEnd();
    void clear();

    struct DeferredDeinit {
        llvm::Function* function;
        llvm::Value* value;
        const Decl* decl;
    };

    llvm::SmallVector<const Expr*, 8> deferredExprs;
    llvm::SmallVector<DeferredDeinit, 8> deinitsToCall;
    llvm::DenseMap<const Decl*, llvm::Value*> valuesByDecl;
    IRGenerator* irGenerator;
};

class IRGenerator {
public:
    IRGenerator();
    llvm::Module& codegenModule(const Module& sourceModule);
    llvm::LLVMContext& getLLVMContext() { return ctx; }
    std::vector<llvm::Module*> getGeneratedModules() { return std::move(generatedModules); }

private:
    friend struct IRGenScope;

    void codegenFunctionBody(const FunctionDecl& decl, llvm::Function& function);
    void createDeinitCall(llvm::Function* deinit, llvm::Value* valueToDeinit);

    /// 'decl' is null if this is the 'this' value.
    void setLocalValue(llvm::Value* value, const VariableDecl* decl);
    llvm::Value* getValueOrNull(const Decl* decl);
    llvm::Value* getValue(const Decl* decl);
    llvm::Value* getThis();

    /// Emits and loads value.
    llvm::Value* codegenExpr(const Expr& expr);
    /// Emits value without loading.
    llvm::Value* codegenLvalueExpr(const Expr& expr);
    /// Emits value as a pointer, storing it in a temporary alloca if needed.
    llvm::Value* codegenExprAsPointer(const Expr& expr);
    llvm::Value* codegenExprOrEnumTag(const Expr& expr, llvm::Value** enumValue);
    llvm::Value* codegenExprWithoutAutoCast(const Expr& expr);
    llvm::Value* codegenAutoCast(llvm::Value* value, const Expr& expr);
    llvm::Value* codegenVarExpr(const VarExpr& expr);
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
    llvm::Value* codegenIncrementExpr(const UnaryExpr& expr);
    llvm::Value* codegenDecrementExpr(const UnaryExpr& expr);
    llvm::Value* codegenLogicalAnd(const Expr& left, const Expr& right);
    llvm::Value* codegenLogicalOr(const Expr& left, const Expr& right);
    llvm::Value* codegenBinaryOp(Token::Kind op, llvm::Value* lhs, llvm::Value* rhs, Type lhsType);
    llvm::Value* codegenShortCircuitBinaryOp(BinaryOperator op, const Expr& lhs, const Expr& rhs);
    llvm::Value* codegenBinaryExpr(const BinaryExpr& expr);
    void codegenAssignment(const BinaryExpr& expr);
    llvm::Value* codegenExprForPassing(const Expr& expr, llvm::Type* targetType);
    llvm::Value* codegenBuiltinConversion(const Expr& expr, Type type);
    void codegenAssert(llvm::Value* condition, SourceLocation location, llvm::StringRef message = "Assertion failed");
    llvm::Value* codegenEnumCase(const EnumCase& enumCase, llvm::ArrayRef<NamedValue> associatedValueElements);
    llvm::Value* codegenCallExpr(const CallExpr& expr, llvm::AllocaInst* thisAllocaForInit = nullptr);
    llvm::Value* codegenBuiltinCast(const CallExpr& expr);
    llvm::Value* codegenSizeofExpr(const SizeofExpr& expr);
    llvm::Value* codegenAddressofExpr(const AddressofExpr& expr);
    llvm::Value* codegenMemberAccess(llvm::Value* baseValue, Type memberType, llvm::StringRef memberName);
    llvm::Value* codegenMemberExpr(const MemberExpr& expr);
    llvm::Value* codegenTupleElementAccess(const MemberExpr& expr);
    llvm::Value* codegenSubscriptExpr(const SubscriptExpr& expr);
    llvm::Value* codegenUnwrapExpr(const UnwrapExpr& expr);
    llvm::Value* codegenLambdaExpr(const LambdaExpr& expr);
    llvm::Value* codegenIfExpr(const IfExpr& expr);

    void codegenDeferredExprsAndDeinitCallsForReturn();
    void codegenBlock(llvm::ArrayRef<Stmt*> stmts, llvm::BasicBlock* continuation);
    void codegenReturnStmt(const ReturnStmt& stmt);
    void codegenVarStmt(const VarStmt& stmt);
    void codegenIfStmt(const IfStmt& ifStmt);
    void codegenSwitchStmt(const SwitchStmt& switchStmt);
    void codegenWhileStmt(const WhileStmt& whileStmt);
    void codegenBreakStmt(const BreakStmt&);
    void codegenContinueStmt(const ContinueStmt&);
    llvm::Value* codegenAssignmentLHS(const Expr& lhs);
    void codegenCompoundStmt(const CompoundStmt& stmt);
    void codegenStmt(const Stmt& stmt);

    void codegenDecl(const Decl& decl);
    void codegenFunctionDecl(const FunctionDecl& decl);
    llvm::StructType* codegenTypeDecl(const TypeDecl& decl);
    llvm::Value* codegenVarDecl(const VarDecl& decl);

    llvm::Value* getFunctionForCall(const CallExpr& call);
    llvm::Function* getFunctionProto(const FunctionDecl& decl);
    llvm::AllocaInst* createEntryBlockAlloca(llvm::Type* type, llvm::Value* arraySize = nullptr, const llvm::Twine& name = "");
    llvm::AllocaInst* createTempAlloca(llvm::Value* value, const llvm::Twine& name = "");
    llvm::Value* createLoad(llvm::Value* value);
    std::vector<llvm::Type*> getFieldTypes(const TypeDecl& decl);
    llvm::Type* getBuiltinType(llvm::StringRef name);
    llvm::Type* getEnumType(const EnumDecl& enumDecl);
    llvm::Type* toIR(Type type, SourceLocation location = SourceLocation());
    llvm::Type* getLLVMTypeForPassing(const TypeDecl& typeDecl);
    llvm::Value* getArrayLength(const Expr& object, Type objectType);
    llvm::Value* getArrayIterator(const Expr& object, Type objectType);

    void beginScope();
    void endScope();
    void deferEvaluationOf(const Expr& expr);
    DeinitDecl* getDefaultDeinitializer(const TypeDecl& typeDecl);
    void deferDeinitCall(llvm::Value* valueToDeinit, const VariableDecl* decl);
    IRGenScope& globalScope() { return scopes.front(); }

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
    std::vector<IRGenScope> scopes;

    llvm::LLVMContext ctx;
    llvm::IRBuilder<> builder;
    llvm::Module* module = nullptr;
    std::vector<llvm::Module*> generatedModules;
    llvm::BasicBlock::iterator lastAlloca;

    std::map<std::string, FunctionInstantiation> functionInstantiations;
    llvm::StringMap<std::pair<llvm::StructType*, const TypeDecl*>> structs;
    const Decl* currentDecl;

    /// The basic blocks to branch to on a 'break'/'continue' statement.
    llvm::SmallVector<llvm::BasicBlock*, 4> breakTargets;
    llvm::SmallVector<llvm::BasicBlock*, 4> continueTargets;
};

} // namespace delta
