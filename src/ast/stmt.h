#pragma once
#include "expr.h"
#include <vector>

namespace cx {

struct Decl;
struct VarDecl;

enum class StmtKind {
    ReturnStmt,
    VarStmt,
    ExprStmt,
    DeferStmt,
    IfStmt,
    SwitchStmt,
    WhileStmt,
    DoWhileStmt,
    ForStmt,
    ForEachStmt,
    BreakStmt,
    ContinueStmt,
    CompoundStmt,
};

struct Stmt {
    virtual ~Stmt() = 0;
    bool isReturnStmt() const { return kind == StmtKind::ReturnStmt; }
    bool isVarStmt() const { return kind == StmtKind::VarStmt; }
    bool isExprStmt() const { return kind == StmtKind::ExprStmt; }
    bool isDeferStmt() const { return kind == StmtKind::DeferStmt; }
    bool isIfStmt() const { return kind == StmtKind::IfStmt; }
    bool isSwitchStmt() const { return kind == StmtKind::SwitchStmt; }
    bool isWhileStmt() const { return kind == StmtKind::WhileStmt; }
    bool isForStmt() const { return kind == StmtKind::ForStmt; }
    bool isForEachStmt() const { return kind == StmtKind::ForEachStmt; }
    bool isBreakStmt() const { return kind == StmtKind::BreakStmt; }
    bool isContinueStmt() const { return kind == StmtKind::ContinueStmt; }
    bool isCompoundStmt() const { return kind == StmtKind::CompoundStmt; }
    bool isBreakable() const;
    bool isContinuable() const;
    Stmt* instantiate(const llvm::StringMap<GenericArg>& genericArgs) const;
    Stmt* instantiateImpl(const llvm::StringMap<GenericArg>& genericArgs) const;

    const StmtKind kind;
    // Safety checks disabled for this statement by `@unchecked`-family attributes.
    DisabledChecks disabledChecks = DisabledChecks::None;

protected:
    Stmt(StmtKind kind) : kind(kind) {}
};

inline Stmt::~Stmt() {}

struct ReturnStmt : Stmt {
    ReturnStmt(Expr* value, Location location) : Stmt(StmtKind::ReturnStmt), value(value), location(location) {}
    static bool classof(const Stmt* s) { return s->kind == StmtKind::ReturnStmt; }

    Expr* value;
    Location location;
    /// Decls moved-from on the path reaching this return. Branch merging may forget
    /// these moves for later code, but this path must still skip their destructors.
    AstVector<const Decl*> movedDecls;
};

struct VarStmt : Stmt {
    VarStmt(AstVector<VarDecl*>&& decls) : Stmt(StmtKind::VarStmt), decls(std::move(decls)) {}
    static bool classof(const Stmt* s) { return s->kind == StmtKind::VarStmt; }

    AstVector<VarDecl*> decls;
};

/// A statement that consists of the evaluation of a single expression.
struct ExprStmt : Stmt {
    ExprStmt(Expr* expr, bool discardsResult = false) : Stmt(StmtKind::ExprStmt), expr(expr), discardsResult(discardsResult) {}
    static bool classof(const Stmt* s) { return s->kind == StmtKind::ExprStmt; }

    Expr* expr;
    // True for the explicit '_ = expr' discard form.
    bool discardsResult;
};

struct DeferStmt : Stmt {
    DeferStmt(Expr* expr, Location location) : Stmt(StmtKind::DeferStmt), expr(expr), location(location) {}
    static bool classof(const Stmt* s) { return s->kind == StmtKind::DeferStmt; }

    Expr* expr;
    Location location;
};

struct IfStmt : Stmt {
    IfStmt(Expr* condition, AstVector<Stmt*>&& thenBody, AstVector<Stmt*>&& elseBody, Location elseLocation = Location())
    : Stmt(StmtKind::IfStmt), condition(condition), thenBody(std::move(thenBody)), elseBody(std::move(elseBody)), elseLocation(elseLocation) {}
    static bool classof(const Stmt* s) { return s->kind == StmtKind::IfStmt; }

    Expr* condition;
    // Set when the condition binds an enum payload (`if s is Case name`); visible in the then-branch only.
    VarDecl* isBinding = nullptr;
    AstVector<Stmt*> thenBody;
    AstVector<Stmt*> elseBody;
    Location elseLocation;
};

struct SwitchCase {
    Expr* value;
    VarDecl* associatedValue;
    AstVector<Stmt*> stmts;
};

struct SwitchStmt : Stmt {
    SwitchStmt(Expr* condition, AstVector<SwitchCase>&& cases, AstVector<Stmt*>&& defaultStmts)
    : Stmt(StmtKind::SwitchStmt), condition(condition), cases(std::move(cases)), defaultStmts(std::move(defaultStmts)) {}
    static bool classof(const Stmt* s) { return s->kind == StmtKind::SwitchStmt; }

    Expr* condition;
    AstVector<SwitchCase> cases;
    AstVector<Stmt*> defaultStmts;
    bool coversAllEnumCases = false;
};

struct WhileStmt : Stmt {
    WhileStmt(Expr* condition, AstVector<Stmt*>&& body, Location location)
    : Stmt(StmtKind::WhileStmt), condition(condition), body(std::move(body)), location(location) {}
    Stmt* lower();
    static bool classof(const Stmt* s) { return s->kind == StmtKind::WhileStmt; }

    Expr* condition;
    AstVector<Stmt*> body;
    Location location;
};

struct DoWhileStmt : Stmt {
    DoWhileStmt(Expr* condition, AstVector<Stmt*>&& body, Location location)
    : Stmt(StmtKind::DoWhileStmt), condition(condition), body(std::move(body)), location(location) {}
    static bool classof(const Stmt* s) { return s->kind == StmtKind::DoWhileStmt; }

    Expr* condition;
    AstVector<Stmt*> body;
    Location location;
};

struct ForStmt : Stmt {
    ForStmt(VarStmt* variable, Expr* condition, AstVector<Expr*>&& increments, AstVector<Stmt*>&& body, Location location)
    : Stmt(StmtKind::ForStmt), variable(variable), condition(condition), increments(std::move(increments)), body(std::move(body)), location(location) {}
    static bool classof(const Stmt* s) { return s->kind == StmtKind::ForStmt; }

    VarStmt* variable;
    Expr* condition;
    AstVector<Expr*> increments;
    AstVector<Stmt*> body;
    Location location;
};

struct ForEachStmt : Stmt {
    ForEachStmt(VarDecl* variable, VarDecl* indexVariable, Expr* range, AstVector<Stmt*>&& body, Location location)
    : Stmt(StmtKind::ForEachStmt), variable(variable), indexVariable(indexVariable), range(range), body(std::move(body)), location(location) {}
    Stmt* lower(int nestLevel, bool rangeIsConst);
    static bool classof(const Stmt* s) { return s->kind == StmtKind::ForEachStmt; }

    VarDecl* variable;
    VarDecl* indexVariable; // Null unless written as 'for elem, index in ...'.
    Expr* range;
    AstVector<Stmt*> body;
    Location location;
};

struct BreakStmt : Stmt {
    BreakStmt(Location location) : Stmt(StmtKind::BreakStmt), location(location) {}
    static bool classof(const Stmt* s) { return s->kind == StmtKind::BreakStmt; }

    Location location;
};

struct ContinueStmt : Stmt {
    ContinueStmt(Location location) : Stmt(StmtKind::ContinueStmt), location(location) {}
    static bool classof(const Stmt* s) { return s->kind == StmtKind::ContinueStmt; }

    Location location;
};

struct CompoundStmt : Stmt {
    CompoundStmt(AstVector<Stmt*>&& body) : Stmt(StmtKind::CompoundStmt), body(std::move(body)) {}
    static bool classof(const Stmt* s) { return s->kind == StmtKind::CompoundStmt; }

    AstVector<Stmt*> body;
};

} // namespace cx
