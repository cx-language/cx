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
#include "location.h"
#include "token.h"
#include "type.h"

namespace cx {

struct Decl;
struct FieldDecl;
struct FunctionDecl;
struct VarDecl;
struct Module;

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
    FieldDecl* getFieldDecl() const;
    const Expr* withoutImplicitCast() const;
    bool isThis() const;

    ExprKind kind;
    Type type;
    Type assignableType;
    Location location;
    // True when this expression was explicitly parenthesized in source.
    bool parenthesized = false;
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
};

struct StringLiteralExpr : Expr {
    StringLiteralExpr(std::string&& value, Location location) : Expr(ExprKind::StringLiteralExpr, location), value(std::move(value)) {}
    static bool classof(const Expr* e) { return e->kind == ExprKind::StringLiteralExpr; }

    std::string value;
};

struct CharacterLiteralExpr : Expr {
    CharacterLiteralExpr(char value, Location location) : Expr(ExprKind::CharacterLiteralExpr, location), value(value) {}
    static bool classof(const Expr* e) { return e->kind == ExprKind::CharacterLiteralExpr; }

    char value;
};

struct IntLiteralExpr : Expr {
    IntLiteralExpr(llvm::APSInt value, Location location) : Expr(ExprKind::IntLiteralExpr, location), value(std::move(value)) {}
    static bool classof(const Expr* e) { return e->kind == ExprKind::IntLiteralExpr; }

    llvm::APSInt value;
};

struct FloatLiteralExpr : Expr {
    FloatLiteralExpr(llvm::APFloat value, Location location) : Expr(ExprKind::FloatLiteralExpr, location), value(std::move(value)) {}
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
    ArrayLiteralExpr(std::vector<Expr*>&& elements, Location location) : Expr(ExprKind::ArrayLiteralExpr, location), elements(std::move(elements)) {}
    static bool classof(const Expr* e) { return e->kind == ExprKind::ArrayLiteralExpr; }

    std::vector<Expr*> elements;
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
    AnonymousStructExpr(std::vector<NamedValue>&& elements, Location location) : Expr(ExprKind::AnonymousStructExpr, location), elements(std::move(elements)) {}
    const Expr* getElementByName(llvm::StringRef name) const;
    static bool classof(const Expr* e) { return e->kind == ExprKind::AnonymousStructExpr; }

    std::vector<NamedValue> elements;
};

struct CallExpr : Expr {
    CallExpr(Expr* callee, std::vector<NamedValue>&& args, std::vector<GenericArg>&& genericArgs, Location location)
    : Expr(ExprKind::CallExpr, location), callee(callee), args(std::move(args)), genericArgs(std::move(genericArgs)), calleeDecl(nullptr) {}
    bool callsNamedFunction() const { return callee->isVarExpr() || callee->isMemberExpr(); }
    llvm::StringRef getFunctionName() const;
    std::string getQualifiedFunctionName() const;
    bool isMethodCall() const { return callee->isMemberExpr(); }
    bool isBuiltinConversion() const { return Type::isBuiltinScalar(getFunctionName()); }
    bool isBuiltinCast() const { return getFunctionName() == "cast"; }
    bool isMoveInit() const;
    const Expr* getReceiver() const;
    Expr* getReceiver();
    static bool classof(const Expr* e) {
        switch (e->kind) {
        case ExprKind::CallExpr:
        case ExprKind::UnaryExpr:
        case ExprKind::BinaryExpr:
        case ExprKind::IndexExpr:
        case ExprKind::IndexAssignmentExpr:
            return true;
        default:
            return false;
        }
    }

    Expr* callee;
    std::vector<NamedValue> args;
    std::vector<GenericArg> genericArgs;
    Type receiverType;
    Decl* calleeDecl;
    // Maps each arg to its parameter index, or -1 for variadic extras. Filled by typechecking.
    // Args stay in written order so they evaluate in argument order; backends reorder via this mapping.
    std::vector<int> argParamIndices;

protected:
    CallExpr(ExprKind kind, Expr* callee, std::vector<NamedValue>&& args, Location location)
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
    // True when the operator is derived from its counterpart (e.g. != from ==) and the result must be negated.
    bool negateResult = false;
    // For anonymous struct `==`/`!=`: elementwise lowering over compiler-generated temporaries.
    // Codegen binds the temporaries to the operand values, so operands with side
    // effects evaluate once no matter how many elements are compared. Null when
    // the comparison wasn't lowered this way (e.g. in global initializers).
    VarDecl* anonymousStructTempLHS = nullptr;
    VarDecl* anonymousStructTempRHS = nullptr;
    Expr* anonymousStructComparisonLowering = nullptr;
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

/// A postfix expression that unwraps an optional (nullable) value, yielding the value wrapped by
/// the optional, for example 'foo!'. If the optional is null, the operation triggers an assertion
/// error (by default), or causes undefined behavior (in unchecked mode).
struct UnwrapExpr : Expr {
    UnwrapExpr(Expr* operand, Location location) : Expr(ExprKind::UnwrapExpr, location), operand(operand) {}
    static bool classof(const Expr* e) { return e->kind == ExprKind::UnwrapExpr; }

    Expr* operand;
};

struct LambdaExpr : Expr {
    LambdaExpr(std::vector<ParamDecl>&& params, Module* module, Location location);
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
    SwitchExpr(Expr* condition, std::vector<SwitchExprArm>&& arms, Expr* defaultExpr, Location location)
    : Expr(ExprKind::SwitchExpr, location), condition(condition), arms(std::move(arms)), defaultExpr(defaultExpr) {}
    static bool classof(const Expr* e) { return e->kind == ExprKind::SwitchExpr; }

    Expr* condition;
    std::vector<SwitchExprArm> arms;
    Expr* defaultExpr;
};

struct ImplicitCastExpr : Expr {
    enum Kind {
        OptionalWrap,
        OptionalUnwrap,
        AutoReference,
        AutoDereference,
        Reborrow,
        NumericWiden,
    };

    ImplicitCastExpr(Expr* operand, Type targetType, Kind kind) : Expr(ExprKind::ImplicitCastExpr, operand->location), operand(operand), castKind(kind) {
        type = NOTNULL(targetType);
        assignableType = NOTNULL(targetType);
        endLocation = operand->endLocation;
    }
    static bool classof(const Expr* e) { return e->kind == ExprKind::ImplicitCastExpr; }

    Expr* operand;
    Kind castKind;
};

struct VarDeclExpr : Expr {
    VarDeclExpr(VarDecl* varDecl);
    static bool classof(const Expr* e) { return e->kind == ExprKind::VarDeclExpr; }

    VarDecl* varDecl;
};

} // namespace cx
