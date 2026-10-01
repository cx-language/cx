#pragma once

#include <string>
#include <vector>
#pragma warning(push, 0)
#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/APSInt.h>
#pragma warning(pop)
#include "../ast/token.h"
#include "../ast/type.h"

namespace llvm {
class StringRef;
}

namespace cx {

struct Expr;
struct CallExpr;
struct BinaryExpr;
struct UnaryExpr;
struct MemberExpr;
struct Function;
struct BasicBlock;
struct Instruction;
struct Parameter;
struct IRField;

enum class IRTypeKind {
    IRBasicType,
    IRPointerType,
    IRFunctionType,
    IRArrayType,
    IRStructType,
    IRUnionType,
};

struct IRType {
    IRTypeKind kind;

    bool isBasicType() { return kind == IRTypeKind::IRBasicType; }
    bool isPointerType() { return kind == IRTypeKind::IRPointerType; }
    bool isFunctionType() { return kind == IRTypeKind::IRFunctionType; }
    bool isArrayType() { return kind == IRTypeKind::IRArrayType; }
    bool isStruct() { return kind == IRTypeKind::IRStructType; }
    bool isUnion() { return kind == IRTypeKind::IRUnionType; }
    bool isInteger();
    bool isSignedInteger();
    bool isUnsignedInteger();
    bool isFloatingPoint();
    bool isChar();
    bool isBool();
    bool isVoid();
    bool isNever();
    IRType* getPointee();
    llvm::ArrayRef<IRField> getFields();
    llvm::StringRef getName();
    IRType* getReturnType();
    llvm::ArrayRef<IRType*> getParamTypes();
    IRType* getElementType();
    int getArraySize();
    IRType* getPointerTo();
    bool equals(IRType* other);
    // Same calling-convention shape. Integer signedness, type names, pointee
    // types, and constness don't affect calls (opaque pointers, width-only
    // integers), so same-named externs differing only in those share one
    // symbol instead of conflicting.
    bool abiEquals(IRType* other);
};

struct IRBasicType : IRType {
    std::string name;

    static bool classof(const IRType* t) { return t->kind == IRTypeKind::IRBasicType; }
};

struct IRPointerType : IRType {
    IRType* pointee;
    bool mutablePointee;

    static bool classof(const IRType* t) { return t->kind == IRTypeKind::IRPointerType; }
};

struct IRFunctionType : IRType {
    IRType* returnType;
    std::vector<IRType*> paramTypes;
    bool isVariadic;

    static bool classof(const IRType* t) { return t->kind == IRTypeKind::IRFunctionType; }
};

struct IRArrayType : IRType {
    IRType* elementType;
    int size;
    // Set for sizeof-computed sizes, which backends fold with target layout.
    // Empty otherwise; size is -1 while symbolic.
    Type sizeofOperand;

    bool hasSymbolicSize() const { return (bool)sizeofOperand; }
    static bool classof(const IRType* t) { return t->kind == IRTypeKind::IRArrayType; }
};

struct IRField {
    IRType* type;
    std::string name;
    // True for generated members standing in for anonymous C structs/unions;
    // the name exists only in cx, C promotes the members (see FieldDecl).
    bool isAnonymous = false;
};

struct IRStructType : IRType {
    std::vector<IRField> fields;
    std::string name;
    std::string mangledName;
    bool packed;
    bool isImportedFromC;

    static bool classof(const IRType* t) { return t->kind == IRTypeKind::IRStructType; }
};

// TODO: combine IRUnionType to IRStructType?
struct IRUnionType : IRType {
    std::vector<IRField> fields;
    std::string name;
    std::string mangledName;
    bool isImportedFromC = false;

    static bool classof(const IRType* t) { return t->kind == IRTypeKind::IRUnionType; }
};

IRType* getIRType(Type astType);
// True when pointer constness differs at any level. Non-pointers do not.
bool pointeeConstDiffers(IRType* a, IRType* b);
// Bit width of an integer type, 0 for anything else.
int getIntegerBitWidth(IRType* type);
// Unsigned cx integer type of the given width (8, 16, 32, 64, or 128).
Type getUnsignedIntegerType(int width);
llvm::raw_ostream& operator<<(llvm::raw_ostream& stream, IRType* type);

enum class ValueKind {
    AllocaInst,
    ReturnInst,
    BranchInst,
    CondBranchInst,
    SwitchInst,
    LoadInst,
    StoreInst,
    InsertInst,
    ExtractInst,
    CallInst,
    BinaryInst,
    UnaryInst,
    GEPInst,
    ConstGEPInst,
    CastInst,
    UnreachableInst,
    ArrayOpInst,
    SizeofInst,
    CheckedArithInst,
    ArithOverflowInst,
    SaturatingArithInst,
    BasicBlock,
    Function,
    Parameter,
    GlobalVariable,
    ConstantString,
    ConstantInt,
    ConstantFP,
    ConstantBool,
    ConstantNull,
    Undefined,
};

struct Value {
    ValueKind kind;
    BasicBlock* parent = nullptr;

    Value(ValueKind kind) : kind(kind) {}
    IRType* getType() const;
    std::string getName() const;
    const Expr* getExpr() const;
    bool isTerminator() const { return kind == ValueKind::ReturnInst || kind == ValueKind::BranchInst || kind == ValueKind::CondBranchInst; }
    bool isGlobal() const { return kind == ValueKind::GlobalVariable || kind == ValueKind::Function; }
    void print(llvm::raw_ostream& stream) const;
    Value* getBranchArgument() const;
    bool loads(Value* pointer, int gepIndex = -1);
};

struct Instruction : Value {
    static bool classof(const Value* v) { return v->kind >= ValueKind::AllocaInst && v->kind <= ValueKind::SaturatingArithInst; }
};

struct AllocaInst : Instruction {
    IRType* allocatedType;
    std::string name;

    static bool classof(const Value* v) { return v->kind == ValueKind::AllocaInst; }
};

struct ReturnInst : Instruction {
    Value* value;

    static bool classof(const Value* v) { return v->kind == ValueKind::ReturnInst; }
};

struct BranchInst : Instruction {
    BasicBlock* destination;
    Value* argument;

    static bool classof(const Value* v) { return v->kind == ValueKind::BranchInst; }
};

struct CondBranchInst : Instruction {
    Value* condition;
    BasicBlock* trueBlock;
    BasicBlock* falseBlock;
    Value* argument;

    static bool classof(const Value* v) { return v->kind == ValueKind::CondBranchInst; }
};

struct SwitchInst : Instruction {
    Value* condition;
    BasicBlock* defaultBlock;
    std::vector<std::pair<Value*, BasicBlock*>> cases;

    static bool classof(const Value* v) { return v->kind == ValueKind::SwitchInst; }
};

struct LoadInst : Instruction {
    Value* value;
    const Expr* expr;
    std::string name;

    static bool classof(const Value* v) { return v->kind == ValueKind::LoadInst; }
};

struct StoreInst : Instruction {
    Value* value;
    Value* pointer;

    static bool classof(const Value* v) { return v->kind == ValueKind::StoreInst; }
};

struct InsertInst : Instruction {
    Value* aggregate;
    Value* value;
    int index;
    std::string name;

    static bool classof(const Value* v) { return v->kind == ValueKind::InsertInst; }
};

struct ExtractInst : Instruction {
    Value* aggregate;
    int index;
    std::string name;

    static bool classof(const Value* v) { return v->kind == ValueKind::ExtractInst; }
};

struct CallInst : Instruction {
    Value* function;
    std::vector<Value*> args;
    const Expr* expr;
    std::string name;

    static bool classof(const Value* v) { return v->kind == ValueKind::CallInst; }
};

struct BinaryInst : Instruction {
    BinaryOperator op;
    Value* left;
    Value* right;
    const Expr* expr;
    std::string name;

    static bool classof(const Value* v) { return v->kind == ValueKind::BinaryInst; }
};

struct UnaryInst : Instruction {
    UnaryOperator op;
    Value* operand;
    const UnaryExpr* expr;
    std::string name;

    static bool classof(const Value* v) { return v->kind == ValueKind::UnaryInst; }
};

struct GEPInst : Instruction {
    Value* pointer;
    std::vector<Value*> indexes;
    std::string name;
    const Expr* expr = nullptr;

    static bool classof(const Value* v) { return v->kind == ValueKind::GEPInst; }
};

struct ConstGEPInst : Instruction {
    Value* pointer;
    int index;
    const MemberExpr* expr;
    std::string name;

    static bool classof(const Value* v) { return v->kind == ValueKind::ConstGEPInst; }
};

struct CastInst : Instruction {
    Value* value;
    IRType* type;
    std::string name;

    static bool classof(const Value* v) { return v->kind == ValueKind::CastInst; }
};

struct UnreachableInst : Instruction {
    static bool classof(const Value* v) { return v->kind == ValueKind::UnreachableInst; }
};

// Element-wise array operation (`float[3] + float[3]`, `int[4] * 2`, ...).
// Each side is an array pointer or a broadcast scalar; at least one side is
// an array. Arithmetic evaluates to a pointer to a fresh result array,
// comparisons (==/!=, all/any semantics) to a bool. Kept whole so the LLVM
// backend can emit SIMD vector ops directly instead of relying on the
// optimizer to rediscover them from a scalar loop.
struct ArrayOpInst : Instruction {
    BinaryOperator op;
    Value* left;
    Value* right;
    IRType* arrayType;
    const Expr* expr;
    std::string name;

    static bool classof(const Value* v) { return v->kind == ValueKind::ArrayOpInst; }
};

struct SizeofInst : Instruction {
    IRType* type; ///< The type whose size is taken.
    IRType* resultType; ///< Type of the size value. Internal uses stay uint64.
    std::string name;

    static bool classof(const Value* v) { return v->kind == ValueKind::SizeofInst; }
};

// Integer arithmetic (+, -, *) whose overflow is detectable (see
// emitWrappingArithmetic). The result is the wrapped value; a paired
// ArithOverflowInst reads whether it overflowed. Kept whole so the LLVM
// backend can emit with.overflow intrinsics (hardware flags) instead of
// relying on the optimizer to rediscover them from manual checks; the C
// backend expands the check manually.
struct CheckedArithInst : Instruction {
    BinaryOperator op;
    Value* left;
    Value* right;
    const Expr* expr;
    std::string name;

    static bool classof(const Value* v) { return v->kind == ValueKind::CheckedArithInst; }
};

struct ArithOverflowInst : Instruction {
    Value* checked;
    std::string name;

    static bool classof(const Value* v) { return v->kind == ValueKind::ArithOverflowInst; }
};

// Saturating integer arithmetic (+, -) clamping to the type's minimum or
// maximum instead of wrapping (see emitSaturatingArithmetic). Kept whole so
// the LLVM backend can emit sadd.sat/ssub.sat intrinsics; the C backend
// expands the clamp manually. Multiply and shifts have no counterpart here:
// LLVM offers no saturating multiply, and its saturating shifts saturate
// 0 << big to the maximum while cx yields 0, so those keep the manual
// expansion.
struct SaturatingArithInst : Instruction {
    BinaryOperator op;
    Value* left;
    Value* right;
    const Expr* expr;
    std::string name;

    static bool classof(const Value* v) { return v->kind == ValueKind::SaturatingArithInst; }
};

struct BasicBlock : Value {
    std::string name;
    Function* parent;
    Parameter* parameter = nullptr;
    std::vector<Instruction*> body;
    std::vector<BasicBlock*> predecessors;

    BasicBlock(std::string name, Function* parent = nullptr);
    template<typename T> T* add(T* inst) {
        inst->parent = this;
        body.push_back(inst);
        return inst;
    }
    static bool classof(const Value* v) { return v->kind == ValueKind::BasicBlock; }
};

struct Parameter : Value {
    IRType* type;
    std::string name;

    static bool classof(const Value* v) { return v->kind == ValueKind::Parameter; }
};

struct Function : Value {
    std::string mangledName;
    std::string name;
    IRType* returnType;
    std::vector<Parameter> params;
    std::vector<BasicBlock*> body;
    bool isExtern;
    bool declaredExternC;
    bool isVariadic;
    Location location;

    static bool classof(const Value* v) { return v->kind == ValueKind::Function; }
};

struct GlobalVariable : Value {
    IRType* type;
    Value* value;
    std::string name;

    static bool classof(const Value* v) { return v->kind == ValueKind::GlobalVariable; }
};

struct ConstantString : Value {
    std::string value;

    static bool classof(const Value* v) { return v->kind == ValueKind::ConstantString; }
};

struct ConstantInt : Value {
    IRType* type;
    llvm::APSInt value;

    static bool classof(const Value* v) { return v->kind == ValueKind::ConstantInt; }
};

struct ConstantFP : Value {
    IRType* type;
    llvm::APFloat value;

    static bool classof(const Value* v) { return v->kind == ValueKind::ConstantFP; }
};

struct ConstantBool : Value {
    bool value;

    static bool classof(const Value* v) { return v->kind == ValueKind::ConstantBool; }
};

struct ConstantNull : Value {
    IRType* type;

    static bool classof(const Value* v) { return v->kind == ValueKind::ConstantNull; }
};

struct Undefined : Value {
    IRType* type;

    static bool classof(const Value* v) { return v->kind == ValueKind::Undefined; }
};

struct IRModule {
    std::string name;
    std::vector<Function*> functions;
    std::vector<GlobalVariable*> globalVariables;
    std::vector<std::string> includedHeaders;

    void print(llvm::raw_ostream& stream) const;
};

} // namespace cx
