#pragma once

#include <vector>
#include <memory>
#include <llvm/Support/Casting.h>
#include "expr.h"

namespace delta {

class VarDecl;

enum class StmtKind {
    ReturnStmt,
    VarStmt,
    IncrementStmt,
    DecrementStmt,
    ExprStmt,
    DeferStmt,
    IfStmt,
    SwitchStmt,
    WhileStmt,
    ForStmt,
    BreakStmt,
    AssignStmt,
    CompoundStmt
};

class Stmt {
public:
    virtual ~Stmt() = 0;

    bool isReturnStmt() const { return getKind() == StmtKind::ReturnStmt; }
    bool isVarStmt() const { return getKind() == StmtKind::VarStmt; }
    bool isIncrementStmt() const { return getKind() == StmtKind::IncrementStmt; }
    bool isDecrementStmt() const { return getKind() == StmtKind::DecrementStmt; }
    bool isExprStmt() const { return getKind() == StmtKind::ExprStmt; }
    bool isDeferStmt() const { return getKind() == StmtKind::DeferStmt; }
    bool isIfStmt() const { return getKind() == StmtKind::IfStmt; }
    bool isSwitchStmt() const { return getKind() == StmtKind::SwitchStmt; }
    bool isWhileStmt() const { return getKind() == StmtKind::WhileStmt; }
    bool isForStmt() const { return getKind() == StmtKind::ForStmt; }
    bool isBreakStmt() const { return getKind() == StmtKind::BreakStmt; }
    bool isAssignStmt() const { return getKind() == StmtKind::AssignStmt; }
    bool isCompoundStmt() const { return getKind() == StmtKind::CompoundStmt; }

    StmtKind getKind() const { return kind; }

    std::unique_ptr<Stmt> instantiate(const llvm::StringMap<Type>& genericArgs) const;

protected:
    Stmt(StmtKind kind) : kind(kind) {}

private:
    const StmtKind kind;
};

inline Stmt::~Stmt() {}

class ReturnStmt : public Stmt {
public:
    ReturnStmt(std::unique_ptr<Expr> value, SourceLocation location)
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
    VarStmt(std::unique_ptr<VarDecl> decl)
    : Stmt(StmtKind::VarStmt), decl(std::move(decl)) {}
    VarDecl& getDecl() const { return *decl; }
    static bool classof(const Stmt* s) { return s->getKind() == StmtKind::VarStmt; }

private:
    std::unique_ptr<VarDecl> decl;
};

class IncrementStmt : public Stmt {
public:
    IncrementStmt(std::unique_ptr<Expr> operand, SourceLocation location)
    : Stmt(StmtKind::IncrementStmt), operand(std::move(operand)), location(location) {}
    Expr& getOperand() const { return *operand; }
    SourceLocation getLocation() const { return location; }
    static bool classof(const Stmt* s) { return s->getKind() == StmtKind::IncrementStmt; }

private:
    std::unique_ptr<Expr> operand;
    SourceLocation location; // Location of '++'.
};

class DecrementStmt : public Stmt {
public:
    DecrementStmt(std::unique_ptr<Expr> operand, SourceLocation location)
    : Stmt(StmtKind::DecrementStmt), operand(std::move(operand)), location(location) {}
    Expr& getOperand() const { return *operand; }
    SourceLocation getLocation() const { return location; }
    static bool classof(const Stmt* s) { return s->getKind() == StmtKind::DecrementStmt; }

private:
    std::unique_ptr<Expr> operand;
    SourceLocation location; // Location of '--'.
};

/// A statement that consists of the evaluation of a single expression.
class ExprStmt : public Stmt {
public:
    ExprStmt(std::unique_ptr<Expr> expr)
    : Stmt(StmtKind::ExprStmt), expr(std::move(expr)) {}
    Expr& getExpr() const { return *expr; }
    static bool classof(const Stmt* s) { return s->getKind() == StmtKind::ExprStmt; }

private:
    std::unique_ptr<Expr> expr;
};

class DeferStmt : public Stmt {
public:
    DeferStmt(std::unique_ptr<Expr> expr)
    : Stmt(StmtKind::DeferStmt), expr(std::move(expr)) {}
    Expr& getExpr() const { return *expr; }
    static bool classof(const Stmt* s) { return s->getKind() == StmtKind::DeferStmt; }

private:
    std::unique_ptr<Expr> expr;
};

class IfStmt : public Stmt {
public:
    IfStmt(std::unique_ptr<Expr> condition, std::vector<std::unique_ptr<Stmt>>&& thenBody,
           std::vector<std::unique_ptr<Stmt>>&& elseBody)
    : Stmt(StmtKind::IfStmt), condition(std::move(condition)),
      thenBody(std::move(thenBody)), elseBody(std::move(elseBody)) {}
    Expr& getCondition() const { return *condition; }
    llvm::ArrayRef<std::unique_ptr<Stmt>> getThenBody() const { return thenBody; }
    llvm::ArrayRef<std::unique_ptr<Stmt>> getElseBody() const { return elseBody; }
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

private:
    std::unique_ptr<Expr> value;
    std::vector<std::unique_ptr<Stmt>> stmts;
};

class SwitchStmt : public Stmt {
public:
    SwitchStmt(std::unique_ptr<Expr> condition, std::vector<SwitchCase>&& cases,
               std::vector<std::unique_ptr<Stmt>>&& defaultStmts)
    : Stmt(StmtKind::SwitchStmt), condition(std::move(condition)),
      cases(std::move(cases)), defaultStmts(std::move(defaultStmts)) {}
    Expr& getCondition() const { return *condition; }
    llvm::ArrayRef<SwitchCase> getCases() const { return cases; }
    llvm::ArrayRef<std::unique_ptr<Stmt>> getDefaultStmts() const { return defaultStmts; }
    static bool classof(const Stmt* s) { return s->getKind() == StmtKind::SwitchStmt; }

private:
    std::unique_ptr<Expr> condition;
    std::vector<SwitchCase> cases;
    std::vector<std::unique_ptr<Stmt>> defaultStmts;
};

class WhileStmt : public Stmt {
public:
    WhileStmt(std::unique_ptr<Expr> condition, std::vector<std::unique_ptr<Stmt>>&& body)
    : Stmt(StmtKind::WhileStmt), condition(std::move(condition)), body(std::move(body)) {}
    Expr& getCondition() const { return *condition; }
    llvm::ArrayRef<std::unique_ptr<Stmt>> getBody() const { return body; }
    static bool classof(const Stmt* s) { return s->getKind() == StmtKind::WhileStmt; }

private:
    std::unique_ptr<Expr> condition;
    std::vector<std::unique_ptr<Stmt>> body;
};

class ForStmt : public Stmt {
public:
    ForStmt(std::unique_ptr<VarDecl> variable, std::unique_ptr<Expr> range,
            std::vector<std::unique_ptr<Stmt>>&& body, SourceLocation location)
    : Stmt(StmtKind::ForStmt), variable(std::move(variable)), range(std::move(range)),
      body(std::move(body)), location(location) {}
    VarDecl* getVariable() const { return variable.get(); }
    Expr& getRangeExpr() const { return *range; }
    llvm::ArrayRef<std::unique_ptr<Stmt>> getBody() const { return body; }
    SourceLocation getLocation() const { return location; }
    std::unique_ptr<Stmt> lower();
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

/// An assignment statement, e.g. `a = b`.
/// Also used to represent compound assignments, e.g. `a += b`, desugared as `a = a + b`.
class AssignStmt : public Stmt {
public:
    AssignStmt(std::shared_ptr<Expr>&& lhs, std::unique_ptr<Expr> rhs, bool isCompoundAssignment,
               SourceLocation location)
    : Stmt(StmtKind::AssignStmt), lhs(std::move(lhs)), rhs(std::move(rhs)),
      isCompound(isCompoundAssignment), location(location) {}
    const Expr* getLHS() const { return lhs.get(); }
    const Expr* getRHS() const { return rhs.get(); }
    Expr* getLHS() { return lhs.get(); }
    Expr* getRHS() { return rhs.get(); }
    bool isCompoundAssignment() const { return isCompound; }
    SourceLocation getLocation() const { return location; }
    static bool classof(const Stmt* s) { return s->getKind() == StmtKind::AssignStmt; }

private:
    std::shared_ptr<Expr> lhs; // shared_ptr to support compound assignments.
    std::unique_ptr<Expr> rhs; // Null if the right-hand side is 'undefined'.
    bool isCompound;
    SourceLocation location; // Location of operator symbol.
};

class CompoundStmt : public Stmt {
public:
    CompoundStmt(std::vector<std::unique_ptr<Stmt>>&& body)
    : Stmt(StmtKind::CompoundStmt), body(std::move(body)) {}
    llvm::ArrayRef<std::unique_ptr<Stmt>> getBody() const { return body; }
    static bool classof(const Stmt* s) { return s->getKind() == StmtKind::CompoundStmt; }

private:
    std::vector<std::unique_ptr<Stmt>> body;
};

}
