#pragma once

#include <string>
#include <utility>
#include <vector>
#pragma warning(push, 0)
#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/APSInt.h>
#include <llvm/Support/Casting.h>
#pragma warning(pop)
#include "arena.h"
#include "checks.h"
#include "location.h"
#include "token.h"
#include "type.h"

namespace cx {

struct Decl;
struct FieldDecl;
struct FunctionDecl;
struct VarDecl;
struct Module;

// Maps an array swizzle component to its element index, or -1 for other
// characters. Shared by swizzle typechecking and constant folding.
inline int swizzleIndexFor(char c) {
    switch (c) {
    case 'x':
    case 'r':
    case 's':
        return 0;
    case 'y':
    case 'g':
    case 't':
        return 1;
    case 'z':
    case 'b':
    case 'p':
        return 2;
    case 'w':
    case 'a':
    case 'q':
        return 3;
    default:
        return -1;
    }
}

enum class ExprKind {
    VarExpr,
    StringLiteralExpr,
    CharacterLiteralExpr,
    IntLiteralExpr,
    FloatLiteralExpr,
    BoolLiteralExpr,
    NullLiteralExpr,
    UndefinedLiteralExpr,
    ArrayLiteralExpr,
    AnonymousStructExpr,
    UnaryExpr,
    BinaryExpr,
    CallExpr,
    SizeofExpr,
    MemberExpr,
    IndexExpr,
    IndexAssignmentExpr,
    UnwrapExpr,
    LambdaExpr,
    IfExpr,
    SwitchExpr,
    ImplicitCastExpr,
    VarDeclExpr,
};

struct Expr {
    virtual ~Expr() = 0;

    bool isVarExpr() const { return kind == ExprKind::VarExpr; }
    bool isStringLiteralExpr() const { return kind == ExprKind::StringLiteralExpr; }
    bool isCharacterLiteralExpr() const { return kind == ExprKind::CharacterLiteralExpr; }
    bool isIntLiteralExpr() const { return kind == ExprKind::IntLiteralExpr; }
    bool isFloatLiteralExpr() const { return kind == ExprKind::FloatLiteralExpr; }
    bool isBoolLiteralExpr() const { return kind == ExprKind::BoolLiteralExpr; }
    bool isNullLiteralExpr() const { return kind == ExprKind::NullLiteralExpr; }
    bool isUndefinedLiteralExpr() const { return kind == ExprKind::UndefinedLiteralExpr; }
    bool isArrayLiteralExpr() const { return kind == ExprKind::ArrayLiteralExpr; }
    bool isAnonymousStructExpr() const { return kind == ExprKind::AnonymousStructExpr; }
    bool isUnaryExpr() const { return kind == ExprKind::UnaryExpr; }
    bool isBinaryExpr() const { return kind == ExprKind::BinaryExpr; }
    bool isCallExpr() const { return kind == ExprKind::CallExpr; }
    bool isSizeofExpr() const { return kind == ExprKind::SizeofExpr; }
    bool isMemberExpr() const { return kind == ExprKind::MemberExpr; }
    bool isIndexExpr() const { return kind == ExprKind::IndexExpr; }
    bool isUnwrapExpr() const { return kind == ExprKind::UnwrapExpr; }
    bool isLambdaExpr() const { return kind == ExprKind::LambdaExpr; }
    bool isIfExpr() const { return kind == ExprKind::IfExpr; }
    bool isImplicitCastExpr() const { return kind == ExprKind::ImplicitCastExpr; }
    bool isVarDeclExpr() const { return kind == ExprKind::VarDeclExpr; }

    bool hasType() const { return !!type; }
    bool hasAssignableType() const { return !!assignableType; }
    void removeTypes() {
        type = Type();
        assignableType = Type();
    }
    bool isAssignment() const;
    bool isReferenceExpr() const;
    bool isConstant() const;
    // True when getConstantIntegerValue/getConstantBoolValue handle the expression. Stricter than
    // isConstant: ternary conditions and &&/|| operands must be boolean-foldable, comparisons must
    // be over integers, and implicit casts must be numeric widenings.
    bool isFoldableIntConstant() const;
    bool isFoldableBoolConstant() const;
    llvm::APSInt getConstantIntegerValue() const;
    bool getConstantBoolValue() const;
    bool isLvalue() const;
    Expr* instantiate(const llvm::StringMap<GenericArg>& genericArgs) const;
    Expr* instantiateImpl(const llvm::StringMap<GenericArg>& genericArgs) const;
    FieldDecl* getFieldDecl() const;
    const Expr* withoutImplicitCast() const;
    bool isThis() const;
    // True when this expression addresses storage inside a union: a member
    // access through a union-typed base (possibly under further member or
    // index steps). Such storage may not hold a live value of the
    // expression's type, so assignment never destroys it and deinit on it
    // never consumes the base.
    bool isInsideUnion() const;

    ExprKind kind;
    Type type;
    Type assignableType;
    Location location;
    // Safety checks disabled for this expression by `@unchecked`-family attributes.
    DisabledChecks disabledChecks = DisabledChecks::None;
    // True when this expression was explicitly parenthesized in source.
    bool parenthesized = false;
    // True when the value was moved into its consumer (set by Typechecker::setMoved).
    // IRGen skips temporary-destructor registration for flagged constructor calls.
    bool isMovedFrom = false;
    // One past the last source character of the expression. Invalid for
    // synthesized expressions, which render as a point at location.
    Location endLocation;

protected:
    Expr(ExprKind kind, Location location) : kind(kind), location(location) {}
};

inline Expr::~Expr() {}

struct VarExpr : Expr {
    VarExpr(llvm::StringRef identifier, Location location) : Expr(ExprKind::VarExpr, location), decl(nullptr), identifier(internString(identifier)) {}
    static bool classof(const Expr* e) { return e->kind == ExprKind::VarExpr; }

    Decl* decl;
    llvm::StringRef identifier;
    // True if generic instantiation rewrote a type-parameter reference (e.g. `T` in `T(x)`) to the argument's name.
    bool instantiatedFromTypeParam = false;
};

struct StringLiteralExpr : Expr {
    StringLiteralExpr(llvm::StringRef value, Location location) : Expr(ExprKind::StringLiteralExpr, location), value(internString(value)) {}
    static bool classof(const Expr* e) { return e->kind == ExprKind::StringLiteralExpr; }

    llvm::StringRef value;
};

struct CharacterLiteralExpr : Expr {
    CharacterLiteralExpr(char value, Location location) : Expr(ExprKind::CharacterLiteralExpr, location), value(value) {}
    static bool classof(const Expr* e) { return e->kind == ExprKind::CharacterLiteralExpr; }

    char value;
};

struct IntLiteralExpr : Expr {
    IntLiteralExpr(uint64_t value, bool isSigned, Location location, bool isWide = false)
    : Expr(ExprKind::IntLiteralExpr, location), value(value), isSigned(isSigned), isWide(isWide) {
        ASSERT(!isWide || isSigned); // The 65-bit shape only ever pairs with signed.
    }
    static bool classof(const Expr* e) { return e->kind == ExprKind::IntLiteralExpr; }
    // Rebuilds the exact shape Token::getIntegerValue used to build (65-bit signed when a non-negative value sets the high bit, so
    // -9223372036854775808 still negates to int64 min). Wide values transiently heap-allocate inside the returned APSInt; all other shapes stay inline.
    llvm::APSInt getValue() const {
        llvm::APSInt result(isWide ? 65 : 64, !isSigned);
        result = value;
        return result;
    }

    uint64_t value;
    bool isSigned;
    bool isWide;
};

struct FloatLiteralExpr : Expr {
    FloatLiteralExpr(llvm::APFloat value, Location location) : Expr(ExprKind::FloatLiteralExpr, location), value(std::move(value)) {
        // APFloat heap-allocates only past 64 significand bits. Doubles and x87 long doubles stay inline, so this member owns no malloc
        // memory on x86-64 and macOS ARM64; quad-precision long doubles (some Linux ARM64/PPC/s390x ABIs) would trip this assert.
        ASSERT(llvm::APFloat::semanticsPrecision(this->value.getSemantics()) <= 64);
    }
    static bool classof(const Expr* e) { return e->kind == ExprKind::FloatLiteralExpr; }

    llvm::APFloat value;
};

struct BoolLiteralExpr : Expr {
    BoolLiteralExpr(bool value, Location location) : Expr(ExprKind::BoolLiteralExpr, location), value(value) {}
    static bool classof(const Expr* e) { return e->kind == ExprKind::BoolLiteralExpr; }

    bool value;
};

struct NullLiteralExpr : Expr {
    NullLiteralExpr(Location location) : Expr(ExprKind::NullLiteralExpr, location) {}
    static bool classof(const Expr* e) { return e->kind == ExprKind::NullLiteralExpr; }
};

struct UndefinedLiteralExpr : Expr {
    UndefinedLiteralExpr(Location location) : Expr(ExprKind::UndefinedLiteralExpr, location) {}
    static bool classof(const Expr* e) { return e->kind == ExprKind::UndefinedLiteralExpr; }
};

struct ArrayLiteralExpr : Expr {
    ArrayLiteralExpr(AstVector<Expr*>&& elements, Location location) : Expr(ExprKind::ArrayLiteralExpr, location), elements(std::move(elements)) {}
    static bool classof(const Expr* e) { return e->kind == ExprKind::ArrayLiteralExpr; }

    AstVector<Expr*> elements;
};

struct NamedValue {
    NamedValue(Expr* value) : NamedValue("", NOTNULL(value)) {}
    NamedValue(llvm::StringRef name, Expr* value, Location location = Location())
    : name(internString(name)), value(value), location(location.isValid() ? location : this->value->location) {}

    llvm::StringRef name; // Empty if no name specified.
    Expr* value;
    Location location;
};

struct AnonymousStructExpr : Expr {
    AnonymousStructExpr(AstVector<NamedValue>&& elements, Location location) : Expr(ExprKind::AnonymousStructExpr, location), elements(std::move(elements)) {}
    const Expr* getElementByName(llvm::StringRef name) const;
    static bool classof(const Expr* e) { return e->kind == ExprKind::AnonymousStructExpr; }

    AstVector<NamedValue> elements;
};

struct CallExpr : Expr {
    CallExpr(Expr* callee, AstVector<NamedValue>&& args, AstVector<GenericArg>&& genericArgs, Location location)
    : Expr(ExprKind::CallExpr, location), callee(callee), args(std::move(args)), genericArgs(std::move(genericArgs)), calleeDecl(nullptr) {}
    bool callsNamedFunction() const { return callee->isVarExpr() || callee->isMemberExpr(); }
    llvm::StringRef getFunctionName() const;
    std::string getQualifiedFunctionName() const;
    bool isMethodCall() const { return callee->isMemberExpr(); }
    bool isBuiltinConversion() const { return builtinConversion || Type::isBuiltinScalar(getFunctionName()); }
    bool isBuiltinCast() const { return getFunctionName() == "cast"; }
    bool isMoveInit() const;
    // Whether an `init` call runs on another instance's storage: an explicit
    // receiver that is neither `this` nor a type name (qualified parent init).
    bool isForeignInit() const;
    const Expr* getReceiver() const;
    Expr* getReceiver();
    // The parameter index of an argument, or the argument's position when the
    // mapping is absent (unvalidated calls); -1 marks variadic extras.
    int paramIndexForArg(size_t i) const { return argParamIndices.size() == args.size() ? argParamIndices[i] : int(i); }
    static bool classof(const Expr* e) {
        switch (e->kind) {
        case ExprKind::CallExpr:
        case ExprKind::UnaryExpr:
        case ExprKind::BinaryExpr:
        case ExprKind::IndexExpr:
        case ExprKind::IndexAssignmentExpr:
        case ExprKind::UnwrapExpr:
            return true;
        default:
            return false;
        }
    }

    Expr* callee;
    AstVector<NamedValue> args;
    AstVector<GenericArg> genericArgs;
    Type receiverType;
    Decl* calleeDecl;
    bool builtinConversion = false;
    // True for the `range.iterator()` call synthesized by for-in lowering: not
    // user code, so const-receiver checks (which would reject the aliasing
    // iterator) do not apply. Element constness still flows from the range.
    bool isForInLowering = false;
    // Maps each arg to its parameter index, or -1 for variadic extras. Filled by typechecking.
    // Args stay in written order so they evaluate in argument order; backends reorder via this mapping.
    AstVector<int> argParamIndices;

protected:
    CallExpr(ExprKind kind, Expr* callee, AstVector<NamedValue>&& args, Location location)
    : Expr(kind, location), callee(callee), args(std::move(args)), calleeDecl(nullptr) {}
};

struct UnaryExpr : CallExpr {
    UnaryExpr(UnaryOperator op, Expr* operand, Location location)
    : CallExpr(ExprKind::UnaryExpr, makeAST<VarExpr>(toString(op.kind), location), {NamedValue(operand)}, location), op(op) {}
    Expr& getOperand() { return *args[0].value; }
    const Expr& getOperand() const { return *args[0].value; }
    llvm::APSInt getConstantIntegerValue() const;
    static bool classof(const Expr* e) { return e->kind == ExprKind::UnaryExpr; }

    UnaryOperator op;
};

struct BinaryExpr : CallExpr {
    BinaryExpr(BinaryOperator op, Expr* left, Expr* right, Location location)
    : CallExpr(ExprKind::BinaryExpr, makeAST<VarExpr>(cx::getFunctionName(op), location), {NamedValue(left), NamedValue(right)}, location), op(op) {}
    const Expr& getLHS() const { return *args[0].value; }
    const Expr& getRHS() const { return *args[1].value; }
    Expr& getLHS() { return *args[0].value; }
    Expr& getRHS() { return *args[1].value; }
    void setLHS(Expr* expr) { args[0].value = NOTNULL(expr); }
    void setRHS(Expr* expr) { args[1].value = NOTNULL(expr); }
    llvm::APSInt getConstantIntegerValue() const;
    static bool classof(const Expr* e) { return e->kind == ExprKind::BinaryExpr; }

    BinaryOperator op;
    // True when the LHS was already consumed (moved or deinited) at this assignment; its destructor must not run.
    bool lhsIsMoved = false;
    // True when the LHS holds a live field default (or delegation-built value) in a constructor; overwriting it destroys the old value.
    bool lhsIsLive = false;
    // True when the operator is derived from its counterpart (e.g. != from ==) and the result must be negated.
    bool negateResult = false;
    // True when typechecking already warned that a narrowed optional compared against null cannot be null.
    bool redundantNullCheckWarned = false;
    // For lowered `==`/`!=` (anonymous struct elementwise comparison, optional-vs-wrapped
    // comparison): replacement AST over compiler-generated temporaries. Codegen binds
    // the temporaries to the operand values, so operands with side effects evaluate
    // once. Null when the comparison wasn't lowered this way (e.g. in global initializers).
    VarDecl* comparisonTempLHS = nullptr;
    VarDecl* comparisonTempRHS = nullptr;
    Expr* comparisonLowering = nullptr;
    // NOTE: Array programming (`float[3] + float[3]`, etc.) does NOT use lowering
    // AST fields; typechecking validates and returns the type directly, and IRGen
    // emits element-wise directly (see emitBinaryExpr). No temporaries needed
    // since IRGen emits operands once and reuses values for elements.
};

bool isBuiltinOp(Token::Kind op, Type lhs, Type rhs);

/// A compile-time expression returning the size of a given type in bytes, e.g. 'sizeof(int)'.
struct SizeofExpr : Expr {
    SizeofExpr(Type operandType, Location location) : Expr(ExprKind::SizeofExpr, location), operandType(operandType) {}
    static bool classof(const Expr* e) { return e->kind == ExprKind::SizeofExpr; }

    Type operandType;
};

/// A member access expression using the dot syntax, such as 'a.b'.
struct MemberExpr : Expr {
    MemberExpr(Expr* base, llvm::StringRef member, Location location) : Expr(ExprKind::MemberExpr, location), base(base), member(internString(member)) {}
    static bool classof(const Expr* e) { return e->kind == ExprKind::MemberExpr; }

    Expr* base;
    llvm::StringRef member;
    Decl* decl = nullptr;
    // For array swizzles (`vec.xy`, `vec.rgba`, etc.): element indices, empty when not a swizzle.
    // Set by typechecking; IRGen emits element extracts + array build from these.
    AstVector<int> swizzleIndices;
};

/// An element access expression using the element's index in brackets: 'base[index]'.
struct IndexExpr : CallExpr {
    IndexExpr(Expr* base, Expr* index, Location location, bool fromEnd = false)
    : CallExpr(ExprKind::IndexExpr, makeAST<MemberExpr>(base, fromEnd ? "[-]" : "[]", location), {NamedValue("", index)}, location), fromEnd(fromEnd) {}
    const Expr* getBase() const { return getReceiver(); }
    const Expr* getIndex() const { return args[0].value; }
    Expr* getBase() { return getReceiver(); }
    Expr* getIndex() { return args[0].value; }
    void setIndex(Expr* expr) { args[0].value = NOTNULL(expr); }
    static bool classof(const Expr* e) { return e->kind == ExprKind::IndexExpr; }

    // True for 'base[-index]', which indexes from the end without a runtime sign check.
    bool fromEnd;

protected:
    IndexExpr(Expr* base, Expr* index, Expr* value, Location location, bool fromEnd = false)
    : CallExpr(ExprKind::IndexAssignmentExpr, makeAST<MemberExpr>(base, fromEnd ? "[-]=" : "[]=", location), {NamedValue("", index), NamedValue("", value)},
               location),
      fromEnd(fromEnd) {}
};

/// An assignment to an indexed access: 'base[index] = value'.
struct IndexAssignmentExpr : IndexExpr {
    IndexAssignmentExpr(Expr* base, Expr* index, Expr* value, Location location, bool fromEnd = false) : IndexExpr(base, index, value, location, fromEnd) {}
    const Expr* getValue() const { return args[1].value; }
    Expr* getValue() { return args[1].value; }
    void setValue(Expr* expr) { args[1].value = NOTNULL(expr); }
    static bool classof(const Expr* e) { return e->kind == ExprKind::IndexAssignmentExpr; }
};

/// A postfix '!' expression, desugared into an 'unwrap' method call on the operand. Optionals
/// keep dedicated handling: unwrapping one yields the wrapped value, or triggers an assertion
/// error if it is null (by default), or causes undefined behavior (in unchecked mode).
struct UnwrapExpr : CallExpr {
    UnwrapExpr(Expr* operand, Location location) : CallExpr(ExprKind::UnwrapExpr, makeAST<MemberExpr>(operand, "unwrap", location), {}, location) {}
    static bool classof(const Expr* e) { return e->kind == ExprKind::UnwrapExpr; }
};

struct LambdaExpr : Expr {
    LambdaExpr(AstVector<ParamDecl>&& params, Module* module, Location location);
    static bool classof(const Expr* e) { return e->kind == ExprKind::LambdaExpr; }

    FunctionDecl* functionDecl;
};

struct IfExpr : Expr {
    IfExpr(Expr* condition, Expr* thenExpr, Expr* elseExpr, Location location)
    : Expr(ExprKind::IfExpr, location), condition(condition), thenExpr(thenExpr), elseExpr(elseExpr) {}
    static bool classof(const Expr* e) { return e->kind == ExprKind::IfExpr; }

    Expr* condition;
    Expr* thenExpr;
    Expr* elseExpr;
};

struct SwitchExprArm {
    Expr* value;
    VarDecl* associatedValue;
    Expr* expr;
};

struct SwitchExpr : Expr {
    SwitchExpr(Expr* condition, AstVector<SwitchExprArm>&& arms, Expr* defaultExpr, Location location)
    : Expr(ExprKind::SwitchExpr, location), condition(condition), arms(std::move(arms)), defaultExpr(defaultExpr) {}
    static bool classof(const Expr* e) { return e->kind == ExprKind::SwitchExpr; }

    Expr* condition;
    AstVector<SwitchExprArm> arms;
    Expr* defaultExpr;
};

struct ImplicitCastExpr : Expr {
    enum Kind {
        OptionalWrap,
        OptionalUnwrap,
        OptionalUnwrapPointer,
        AutoReference,
        AutoDereference,
        NumericWiden,
        UserConversion,
    };

    ImplicitCastExpr(Expr* operand, Type targetType, Kind kind, const FunctionDecl* conversionDecl = nullptr)
    : Expr(ExprKind::ImplicitCastExpr, operand->location), operand(operand), castKind(kind), conversionDecl(conversionDecl) {
        type = NOTNULL(targetType);
        assignableType = NOTNULL(targetType);
        endLocation = operand->endLocation;
    }
    static bool classof(const Expr* e) { return e->kind == ExprKind::ImplicitCastExpr; }

    Expr* operand;
    Kind castKind;
    // The implicit constructor or conversion member to call. Only set for UserConversion.
    const FunctionDecl* conversionDecl;
};

struct VarDeclExpr : Expr {
    VarDeclExpr(VarDecl* varDecl);
    static bool classof(const Expr* e) { return e->kind == ExprKind::VarDeclExpr; }

    VarDecl* varDecl;
};

/// Returns the source start of the whole expression. Most expressions start at
/// their location, but BinaryExpr (operator token), IndexExpr and
/// IndexAssignmentExpr ('[' token), MemberExpr ('.' token), CallExpr (via a
/// MemberExpr callee), and ternary IfExpr ('?' token) store a mid-expression
/// location, and postfix '++'/'--'/'!' sit after their operand, so those
/// descend to the LHS/base/callee/condition.
Location getExprRangeStart(const Expr& expr);

// Reports division by zero and out-of-range shifts in a (possibly partly)
// folded array size. Shared by the parser and sema's deferred-size folding.
void checkArraySizeDivisors(const Expr& expr);

/// Resets synthesized lambda names so a repeated compilation in the same
/// process names them identically.
void resetLambdaNameCounter();

} // namespace cx
