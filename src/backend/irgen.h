#pragma once

#include <vector>
#pragma warning(push, 0)
#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/StringSet.h>
#pragma warning(pop)
#include "../ast/decl.h"
#include "../ast/expr.h"
#include "../ast/stmt.h"
#include "../backend/ir.h"
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

    struct DeferredDestructor {
        Function* function;
        Value* value;
        const Decl* decl;
    };

    llvm::SmallVector<const Expr*, 8> deferredExprs;
    llvm::SmallVector<DeferredDestructor, 8> destructorsToCall;
    llvm::DenseMap<const Decl*, Value*> valuesByDecl;
    IRGenerator* irGenerator;
};

class IRGenerator {
public:
    IRGenerator();
    IRModule& emitModule(const Module& sourceModule);
    void emitFunctionBody(const FunctionDecl& decl, Function& function);
    void createDestructorCall(Function* destructor, Value* receiver);
    /// 'decl' is null if this is the 'this' value.
    void setLocalValue(Value* value, const VariableDecl* decl);
    Value* getValueOrNull(const Decl* decl);
    Value* getValue(const Decl* decl);
    Value* getThis(IRType* targetType = nullptr);
    /// Emits and loads value.
    Value* emitExpr(const Expr& expr);
    /// Emits value without loading.
    Value* emitLvalueExpr(const Expr& expr);
    /// Emits value as a pointer, storing it in a temporary alloca if needed.
    Value* emitExprAsPointer(const Expr& expr);
    Value* emitExprOrEnumTag(const Expr& expr, Value** enumValue);
    Value* emitExprWithoutAutoCast(const Expr& expr);
    Value* emitAutoCast(Value* value, const Expr& expr);
    Value* emitVarExpr(const VarExpr& expr);
    Value* emitStringLiteralExpr(const StringLiteralExpr& expr);
    Value* emitCharacterLiteralExpr(const CharacterLiteralExpr& expr);
    Value* emitIntLiteralExpr(const IntLiteralExpr& expr);
    Value* emitFloatLiteralExpr(const FloatLiteralExpr& expr);
    Value* emitBoolLiteralExpr(const BoolLiteralExpr& expr);
    Value* emitNullLiteralExpr(const NullLiteralExpr& expr);
    Value* emitUndefinedLiteralExpr(const UndefinedLiteralExpr& expr);
    Value* emitArrayLiteralExpr(const ArrayLiteralExpr& expr);
    Value* emitTupleExpr(const TupleExpr& expr);
    Value* emitImplicitNullComparison(Value* operand, BinaryOperator op = Token::NotEqual);
    Value* emitNot(const UnaryExpr& expr);
    Value* emitUnaryExpr(const UnaryExpr& expr);
    Value* emitConstantIncrement(const UnaryExpr& expr, int value);
    Value* emitLogicalAnd(const Expr& left, const Expr& right);
    Value* emitLogicalOr(const Expr& left, const Expr& right);
    Value* emitBinaryExpr(const BinaryExpr& expr);
    void emitAssignment(const BinaryExpr& expr);
    Value* emitExprForPassing(const Expr& expr, IRType* targetType);
    Value* emitOptionalConstruction(Type wrappedType, Value* arg);
    void emitAssert(Value* condition, SourceLocation location, llvm::StringRef message = "Assertion failed");
    Value* emitEnumCase(const EnumCase& enumCase, llvm::ArrayRef<NamedValue> associatedValueElements);
    Value* emitCallExpr(const CallExpr& expr, AllocaInst* thisAllocaForInit = nullptr);
    Value* emitBuiltinCast(const CallExpr& expr);
    Value* emitSizeofExpr(const SizeofExpr& expr);
    Value* emitAddressofExpr(const AddressofExpr& expr);
    Value* emitMemberAccess(Value* baseValue, const FieldDecl* field);
    Value* emitMemberExpr(const MemberExpr& expr);
    Value* emitTupleElementAccess(const MemberExpr& expr);
    Value* emitIndexExpr(const IndexExpr& expr);
    Value* emitUnwrapExpr(const UnwrapExpr& expr);
    Value* emitLambdaExpr(const LambdaExpr& expr);
    Value* emitIfExpr(const IfExpr& expr);
    Value* emitImplicitCastExpr(const ImplicitCastExpr& expr);
    void emitDeferredExprsAndDestructorCallsForReturn();
    void emitBlock(llvm::ArrayRef<Stmt*> stmts, BasicBlock* continuation);
    void emitReturnStmt(const ReturnStmt& stmt);
    void emitVarStmt(const VarStmt& stmt);
    void emitIfStmt(const IfStmt& ifStmt);
    void emitSwitchStmt(const SwitchStmt& switchStmt);
    void emitForStmt(const ForStmt& forStmt);
    void emitBreakStmt(const BreakStmt&);
    void emitContinueStmt(const ContinueStmt&);
    Value* emitAssignmentLHS(const Expr& lhs);
    void emitCompoundStmt(const CompoundStmt& stmt);
    void emitStmt(const Stmt& stmt);
    void emitDecl(const Decl& decl);
    void emitFunctionDecl(const FunctionDecl& decl);
    Value* emitVarDecl(const VarDecl& decl);
    Value* getFunctionForCall(const CallExpr& call);
    Function* getFunction(const FunctionDecl& decl);
    AllocaInst* createEntryBlockAlloca(IRType* type, const llvm::Twine& name = "");
    AllocaInst* createEntryBlockAlloca(Type type, const llvm::Twine& name = "") { return createEntryBlockAlloca(getIRType(type), name); }
    AllocaInst* createTempAlloca(Value* value);
    Value* createLoad(Value* value);
    void createStore(Value* value, Value* pointer);
    Value* createCall(Value* function, llvm::ArrayRef<Value*> args);
    void createCondBr(Value* condition, BasicBlock* trueBlock, BasicBlock* falseBlock) {
        insertBlock->body.push_back(new CondBranchInst { ValueKind::CondBranchInst, condition, trueBlock, falseBlock });
    }
    void createBr(BasicBlock* destination) { insertBlock->body.push_back(new BranchInst { ValueKind::BranchInst, destination }); }
    Value* createPhi(std::vector<std::pair<Value*, BasicBlock*>> valuesAndPredecessors, const llvm::Twine& name = "") {
        auto phi = new PhiInst { ValueKind::PhiInst, std::move(valuesAndPredecessors), name.str() };
        insertBlock->body.push_back(phi);
        return phi;
    }
    Value* createInsertValue(Value* aggregate, Value* value, int index) {
        auto inst = new InsertInst { ValueKind::InsertInst, aggregate, value, index, "" };
        insertBlock->body.push_back(inst);
        return inst;
    }
    Value* createExtractValue(Value* aggregate, int index, const llvm::Twine& name = "") {
        auto inst = new ExtractInst { ValueKind::ExtractInst, aggregate, index, name.str() };
        insertBlock->body.push_back(inst);
        return inst;
    }
    Value* createConstantInt(IRType* type, llvm::APSInt value) { return new ConstantInt { ValueKind::ConstantInt, type, std::move(value) }; }
    Value* createConstantInt(IRType* type, int64_t value) { return createConstantInt(type, llvm::APSInt::get(value)); }
    Value* createConstantInt(Type type, llvm::APSInt value) { return createConstantInt(getIRType(type), std::move(value)); }
    Value* createConstantInt(Type type, int64_t value) { return createConstantInt(getIRType(type), llvm::APSInt::get(value)); }
    Value* createConstantFP(IRType* type, llvm::APFloat value) { return new ConstantFP { ValueKind::ConstantFP, type, std::move(value) }; }
    Value* createConstantFP(IRType* type, double value) { return createConstantFP(type, llvm::APFloat(value)); }
    Value* createConstantFP(Type type, llvm::APFloat value) { return createConstantFP(getIRType(type), std::move(value)); }
    Value* createConstantFP(Type type, double value) { return createConstantFP(getIRType(type), llvm::APFloat(value)); }
    Value* createConstantBool(bool value) { return new ConstantBool { ValueKind::ConstantBool, value }; }
    Value* createConstantNull(IRType* type) {
        ASSERT(type->isPointerType());
        return new ConstantNull { ValueKind::ConstantNull, type };
    }
    Value* createConstantNull(Type type) { return createConstantNull(getIRType(type)); }
    Value* createUndefined(IRType* type) { return new Undefined { ValueKind::Undefined, type }; }
    Value* createUndefined(Type type) { return createUndefined(getIRType(type)); }
    Value* createBinaryOp(BinaryOperator op, Value* left, Value* right) {
        ASSERT(left->getType()->equals(right->getType()));
        auto binary = new BinaryInst { ValueKind::BinaryInst, op, left, right, "" };
        insertBlock->body.push_back(binary);
        return binary;
    }
    Value* createIsNull(Value* value, const llvm::Twine& name = "") {
        Value* nullValue;
        auto type = value->getType();

        if (type->isBool()) {
            nullValue = createConstantBool(false);
        } else if (type->isPointerType()) {
            nullValue = createConstantNull(type);
        } else if (type->isInteger()) {
            nullValue = createConstantInt(type, 0);
        } else if (type->isFloatingPoint()) {
            nullValue = createConstantFP(type, 0);
        } else {
            llvm_unreachable("invalid type passed to createIsNull");
        }

        ASSERT(value->getType()->equals(nullValue->getType()));
        auto op = new BinaryInst { ValueKind::BinaryInst, Token::Equal, value, nullValue, name.str() };
        insertBlock->body.push_back(op);
        return op;
    }
    Value* createNeg(Value* value) {
        auto op = new UnaryInst { ValueKind::UnaryInst, Token::Minus, value, "" };
        insertBlock->body.push_back(op);
        return op;
    }
    Value* createNot(Value* value) {
        auto op = new UnaryInst { ValueKind::UnaryInst, Token::Not, value, "" };
        insertBlock->body.push_back(op);
        return op;
    }
    Value* createGEP(Value* pointer, std::vector<Value*> indexes, const llvm::Twine& name = "") {
        auto gep = new GEPInst { ValueKind::GEPInst, pointer, std::move(indexes), name.str() };
        insertBlock->body.push_back(gep);
        return gep;
    }
    Value* createGEP(Value* pointer, int index0, int index1, const llvm::Twine& name = "") {
        if (pointer->getType()->getPointee()->isArrayType()) {
            ASSERT(index1 < pointer->getType()->getPointee()->getArraySize());
        } else {
            ASSERT(index1 < (int) pointer->getType()->getPointee()->getElements().size());
        }
        auto gep = new ConstGEPInst { ValueKind::ConstGEPInst, pointer, index0, index1, name.str() };
        insertBlock->body.push_back(gep);
        return gep;
    }
    Value* createCast(Value* value, IRType* type, const llvm::Twine& name = "") {
        auto cast = new CastInst { ValueKind::CastInst, value, type, name.str() };
        insertBlock->body.push_back(cast);
        return cast;
    }
    Value* createCast(Value* value, Type type, const llvm::Twine& name = "") { return createCast(value, getIRType(type), name); }
    Value* createGlobalVariable(Value* value, const llvm::Twine& name = "") {
        auto globalVariable = new GlobalVariable { ValueKind::GlobalVariable, value, name.str() };
        module->globalVariables.push_back(globalVariable);
        return globalVariable;
    }
    Value* createGlobalStringPtr(llvm::StringRef value) { return new ConstantString { ValueKind::ConstantString, value }; }
    Value* createSizeof(Type type) { return new SizeofInst { ValueKind::SizeofInst, getIRType(type), "" }; }
    SwitchInst* createSwitch(Value* condition, BasicBlock* defaultBlock) {
        auto switchInst = new SwitchInst { ValueKind::SwitchInst, condition, defaultBlock, {} };
        insertBlock->body.push_back(switchInst);
        return switchInst;
    }
    void createUnreachable() { insertBlock->body.push_back(new UnreachableInst { ValueKind::UnreachableInst }); }
    void createReturn(Value* value) { insertBlock->body.push_back(new ReturnInst { ValueKind::ReturnInst, value }); }
    Value* getArrayLength(const Expr& object, Type objectType);
    Value* getArrayIterator(const Expr& object, Type objectType);
    void beginScope();
    void endScope();
    void deferEvaluationOf(const Expr& expr);
    DestructorDecl* getDefaultDestructor(TypeDecl& typeDecl);
    void deferDestructorCall(Value* receiver, const VariableDecl* decl);
    IRGenScope& globalScope() { return scopes.front(); }
    void setInsertPoint(BasicBlock* block);

    struct FunctionInstantiation {
        const FunctionDecl* decl;
        Function* function;
    };

    std::vector<IRGenScope> scopes;
    IRModule* module = nullptr;
    std::vector<IRModule*> generatedModules;
    std::vector<FunctionInstantiation> functionInstantiations;
    const Decl* currentDecl;
    /// The basic blocks to branch to on a 'break'/'continue' statement.
    llvm::SmallVector<BasicBlock*, 4> breakTargets;
    llvm::SmallVector<BasicBlock*, 4> continueTargets;
    BasicBlock* insertBlock;
    Function* currentFunction = nullptr;
    static const int optionalHasValueFieldIndex = 0;
    static const int optionalValueFieldIndex = 1;
};

} // namespace delta
