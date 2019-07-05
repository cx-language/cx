#pragma once

#include <memory>
#include <vector>
#pragma warning(push, 0)
#include <llvm/Support/Casting.h>
#pragma warning(pop)
#include "expr.h"

namespace delta {

class VarDecl;

enum class StmtKind {
    ReturnStmt,
    VarStmt,
    ExprStmt,
    DeferStmt,
    IfStmt,
    SwitchStmt,
    WhileStmt,
    ForStmt,
    BreakStmt,
    ContinueStmt,
    CompoundStmt
};

class Stmt {
public:
    virtual ~Stmt() = 0;

    bool isReturnStmt() const { return getKind() == StmtKind::ReturnStmt; }
    bool isVarStmt() const { return getKind() == StmtKind::VarStmt; }
    bool isExprStmt() const { return getKind() == StmtKind::ExprStmt; }
    bool isDeferStmt() const { return getKind() == StmtKind::DeferStmt; }
    bool isIfStmt() const { return getKind() == StmtKind::IfStmt; }
    bool isSwitchStmt() const { return getKind() == StmtKind::SwitchStmt; }
    bool isWhileStmt() const { return getKind() == StmtKind::WhileStmt; }
    bool isForStmt() const { return getKind() == StmtKind::ForStmt; }
    bool isBreakStmt() const { return getKind() == StmtKind::BreakStmt; }
    bool isContinueStmt() const { return getKind() == StmtKind::ContinueStmt; }
    bool isCompoundStmt() const { return getKind() == StmtKind::CompoundStmt; }

    StmtKind getKind() const { return kind; }
    bool isBreakable() const;
    bool isContinuable() const;
    std::unique_ptr<Stmt> instantiate(const llvm::StringMap<Type>& genericArgs) const;

protected:
    Stmt(StmtKind kind) : kind(kind) {}

private:
    const StmtKind kind;
};

inline Stmt::~Stmt() {}

class ReturnStmt : public Stmt {
public:
    ReturnStmt(std::unique_ptr<Expr>&& value, SourceLocation location)
    : Stmt(StmtKind::ReturnStmt), value(std::move(value)), location(location) {}
    Expr* getReturnValue() const { return value.get(); }
    SourceLocation getLocation() const { return location; }
    static bool classof(const Stmt* s) { return s->getKind() == StmtKind::ReturnStmt; }

private:
    std::unique_ptr<Expr> value;
    SourceLocation location;
};

class VarStmt : public Stmt {
public:
    VarStmt(std::unique_ptr<VarDecl> decl) : Stmt(StmtKind::VarStmt), decl(std::move(decl)) {}
    VarDecl& getDecl() const { return *decl; }
    static bool classof(const Stmt* s) { return s->getKind() == StmtKind::VarStmt; }

private:
    std::unique_ptr<VarDecl> decl;
};

/// A statement that consists of the evaluation of a single expression.
class ExprStmt : public Stmt {
public:
    ExprStmt(std::unique_ptr<Expr> expr) : Stmt(StmtKind::ExprStmt), expr(std::move(expr)) {}
    Expr& getExpr() const { return *expr; }
    static bool classof(const Stmt* s) { return s->getKind() == StmtKind::ExprStmt; }

private:
    std::unique_ptr<Expr> expr;
};

class DeferStmt : public Stmt {
public:
    DeferStmt(std::unique_ptr<Expr> expr) : Stmt(StmtKind::DeferStmt), expr(std::move(expr)) {}
    Expr& getExpr() const { return *expr; }
    static bool classof(const Stmt* s) { return s->getKind() == StmtKind::DeferStmt; }

private:
    std::unique_ptr<Expr> expr;
};

class IfStmt : public Stmt {
public:
    IfStmt(std::unique_ptr<Expr> condition, std::vector<std::unique_ptr<Stmt>>&& thenBody, std::vector<std::unique_ptr<Stmt>>&& elseBody)
    : Stmt(StmtKind::IfStmt), condition(std::move(condition)), thenBody(std::move(thenBody)), elseBody(std::move(elseBody)) {}
    Expr& getCondition() const { return *condition; }
    llvm::ArrayRef<std::unique_ptr<Stmt>> getThenBody() const { return thenBody; }
    llvm::ArrayRef<std::unique_ptr<Stmt>> getElseBody() const { return elseBody; }
    llvm::MutableArrayRef<std::unique_ptr<Stmt>> getThenBody() { return thenBody; }
    llvm::MutableArrayRef<std::unique_ptr<Stmt>> getElseBody() { return elseBody; }
    static bool classof(const Stmt* s) { return s->getKind() == StmtKind::IfStmt; }

private:
    std::unique_ptr<Expr> condition;
    std::vector<std::unique_ptr<Stmt>> thenBody;
    std::vector<std::unique_ptr<Stmt>> elseBody;
};

class SwitchCase {
public:
    SwitchCase(std::unique_ptr<Expr> value, std::vector<std::unique_ptr<Stmt>>&& stmts)
    : value(std::move(value)), stmts(std::move(stmts)) {}
    Expr* getValue() const { return value.get(); }
    llvm::ArrayRef<std::unique_ptr<Stmt>> getStmts() const { return stmts; }
    llvm::MutableArrayRef<std::unique_ptr<Stmt>> getStmts() { return stmts; }

private:
    std::unique_ptr<Expr> value;
    std::vector<std::unique_ptr<Stmt>> stmts;
};

class SwitchStmt : public Stmt {
public:
    SwitchStmt(std::unique_ptr<Expr> condition, std::vector<SwitchCase>&& cases, std::vector<std::unique_ptr<Stmt>>&& defaultStmts)
    : Stmt(StmtKind::SwitchStmt), condition(std::move(condition)), cases(std::move(cases)), defaultStmts(std::move(defaultStmts)) {}
    Expr& getCondition() const { return *condition; }
    llvm::ArrayRef<SwitchCase> getCases() const { return cases; }
    llvm::MutableArrayRef<SwitchCase> getCases() { return cases; }
    llvm::ArrayRef<std::unique_ptr<Stmt>> getDefaultStmts() const { return defaultStmts; }
    llvm::MutableArrayRef<std::unique_ptr<Stmt>> getDefaultStmts() { return defaultStmts; }
    static bool classof(const Stmt* s) { return s->getKind() == StmtKind::SwitchStmt; }

private:
    std::unique_ptr<Expr> condition;
    std::vector<SwitchCase> cases;
    std::vector<std::unique_ptr<Stmt>> defaultStmts;
};

class WhileStmt : public Stmt {
public:
    WhileStmt(std::unique_ptr<Expr> condition, std::vector<std::unique_ptr<Stmt>>&& body, std::unique_ptr<Expr> increment)
    : Stmt(StmtKind::WhileStmt), condition(std::move(condition)), body(std::move(body)), increment(std::move(increment)) {}
    Expr& getCondition() const { return *condition; }
    llvm::ArrayRef<std::unique_ptr<Stmt>> getBody() const { return body; }
    llvm::MutableArrayRef<std::unique_ptr<Stmt>> getBody() { return body; }
    Expr* getIncrement() const { return increment.get(); }
    static bool classof(const Stmt* s) { return s->getKind() == StmtKind::WhileStmt; }

private:
    std::unique_ptr<Expr> condition;
    std::vector<std::unique_ptr<Stmt>> body;
    std::unique_ptr<Expr> increment; // Used in 'for' loop lowering.
};

class ForStmt : public Stmt {
public:
    ForStmt(std::unique_ptr<VarDecl> variable, std::unique_ptr<Expr> range, std::vector<std::unique_ptr<Stmt>>&& body, SourceLocation location)
    : Stmt(StmtKind::ForStmt), variable(std::move(variable)), range(std::move(range)), body(std::move(body)), location(location) {}
    VarDecl* getVariable() const { return variable.get(); }
    Expr& getRangeExpr() const { return *range; }
    llvm::ArrayRef<std::unique_ptr<Stmt>> getBody() const { return body; }
    SourceLocation getLocation() const { return location; }
    std::unique_ptr<Stmt> lower(int nestLevel);
    static bool classof(const Stmt* s) { return s->getKind() == StmtKind::ForStmt; }

private:
    std::unique_ptr<VarDecl> variable;
    std::unique_ptr<Expr> range;
    std::vector<std::unique_ptr<Stmt>> body;
    SourceLocation location;
};

class BreakStmt : public Stmt {
public:
    BreakStmt(SourceLocation location) : Stmt(StmtKind::BreakStmt), location(location) {}
    SourceLocation getLocation() const { return location; }
    static bool classof(const Stmt* s) { return s->getKind() == StmtKind::BreakStmt; }

private:
    SourceLocation location;
};

class ContinueStmt : public Stmt {
public:
    ContinueStmt(SourceLocation location) : Stmt(StmtKind::ContinueStmt), location(location) {}
    SourceLocation getLocation() const { return location; }
    static bool classof(const Stmt* s) { return s->getKind() == StmtKind::ContinueStmt; }

private:
    SourceLocation location;
};

class CompoundStmt : public Stmt {
public:
    CompoundStmt(std::vector<std::unique_ptr<Stmt>>&& body) : Stmt(StmtKind::CompoundStmt), body(std::move(body)) {}
    llvm::ArrayRef<std::unique_ptr<Stmt>> getBody() const { return body; }
    llvm::MutableArrayRef<std::unique_ptr<Stmt>> getBody() { return body; }
    static bool classof(const Stmt* s) { return s->getKind() == StmtKind::CompoundStmt; }

private:
    std::vector<std::unique_ptr<Stmt>> body;
};

} // namespace delta
