#pragma once
#include "expr.h"
#include <vector>
#pragma warning(push, 0)
#include <llvm/ADT/SmallPtrSet.h>
#include <llvm/ADT/SmallVector.h>
#pragma warning(pop)

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

    const StmtKind kind;

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
    llvm::SmallPtrSet<const Decl*, 8> movedDecls;
};

struct VarStmt : Stmt {
    VarStmt(llvm::SmallVector<VarDecl*, 1>&& decls) : Stmt(StmtKind::VarStmt), decls(std::move(decls)) {}
    static bool classof(const Stmt* s) { return s->kind == StmtKind::VarStmt; }

    llvm::SmallVector<VarDecl*, 1> decls;
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
    DeferStmt(Expr* expr) : Stmt(StmtKind::DeferStmt), expr(expr) {}
    static bool classof(const Stmt* s) { return s->kind == StmtKind::DeferStmt; }

    Expr* expr;
};

struct IfStmt : Stmt {
    IfStmt(Expr* condition, std::vector<Stmt*>&& thenBody, std::vector<Stmt*>&& elseBody, Location elseLocation = Location())
    : Stmt(StmtKind::IfStmt), condition(condition), thenBody(std::move(thenBody)), elseBody(std::move(elseBody)), elseLocation(elseLocation) {}
    static bool classof(const Stmt* s) { return s->kind == StmtKind::IfStmt; }

    Expr* condition;
    std::vector<Stmt*> thenBody;
    std::vector<Stmt*> elseBody;
    Location elseLocation;
};

struct SwitchCase {
    Expr* value;
    VarDecl* associatedValue;
    std::vector<Stmt*> stmts;
};

struct SwitchStmt : Stmt {
    SwitchStmt(Expr* condition, std::vector<SwitchCase>&& cases, std::vector<Stmt*>&& defaultStmts)
    : Stmt(StmtKind::SwitchStmt), condition(condition), cases(std::move(cases)), defaultStmts(std::move(defaultStmts)) {}
    static bool classof(const Stmt* s) { return s->kind == StmtKind::SwitchStmt; }

    Expr* condition;
    std::vector<SwitchCase> cases;
    std::vector<Stmt*> defaultStmts;
    bool coversAllEnumCases = false;
};

struct WhileStmt : Stmt {
    WhileStmt(Expr* condition, std::vector<Stmt*>&& body, Location location)
    : Stmt(StmtKind::WhileStmt), condition(condition), body(std::move(body)), location(location) {}
    Stmt* lower();
    static bool classof(const Stmt* s) { return s->kind == StmtKind::WhileStmt; }

    Expr* condition;
    std::vector<Stmt*> body;
    Location location;
};

struct DoWhileStmt : Stmt {
    DoWhileStmt(Expr* condition, std::vector<Stmt*>&& body, Location location)
    : Stmt(StmtKind::DoWhileStmt), condition(condition), body(std::move(body)), location(location) {}
    static bool classof(const Stmt* s) { return s->kind == StmtKind::DoWhileStmt; }

    Expr* condition;
    std::vector<Stmt*> body;
    Location location;
};

struct ForStmt : Stmt {
    ForStmt(VarStmt* variable, Expr* condition, Expr* increment, std::vector<Stmt*>&& body, Location location)
    : Stmt(StmtKind::ForStmt), variable(variable), condition(condition), increment(increment), body(std::move(body)), location(location) {}
    static bool classof(const Stmt* s) { return s->kind == StmtKind::ForStmt; }

    VarStmt* variable;
    Expr* condition;
    Expr* increment;
    std::vector<Stmt*> body;
    Location location;
};

struct ForEachStmt : Stmt {
    ForEachStmt(VarDecl* variable, Expr* range, std::vector<Stmt*>&& body, Location location)
    : Stmt(StmtKind::ForEachStmt), variable(variable), range(range), body(std::move(body)), location(location) {}
    Stmt* lower(int nestLevel);
    static bool classof(const Stmt* s) { return s->kind == StmtKind::ForEachStmt; }

    VarDecl* variable;
    Expr* range;
    std::vector<Stmt*> body;
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
    CompoundStmt(std::vector<Stmt*>&& body) : Stmt(StmtKind::CompoundStmt), body(std::move(body)) {}
    static bool classof(const Stmt* s) { return s->kind == StmtKind::CompoundStmt; }

    std::vector<Stmt*> body;
};

} // namespace cx
