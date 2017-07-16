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
    AugAssignStmt,
};

class Stmt {
public:
    virtual ~Stmt() = 0;

#define DEFINE_STMT_IS_AND_GET(KIND) \
    bool is##KIND() const { return getKind() == StmtKind::KIND; } \
    class KIND& get##KIND() { return llvm::cast<class KIND>(*this); } \
    const class KIND& get##KIND() const { return llvm::cast<class KIND>(*this); }
    DEFINE_STMT_IS_AND_GET(ReturnStmt)
    DEFINE_STMT_IS_AND_GET(VarStmt)
    DEFINE_STMT_IS_AND_GET(IncrementStmt)
    DEFINE_STMT_IS_AND_GET(DecrementStmt)
    DEFINE_STMT_IS_AND_GET(ExprStmt)
    DEFINE_STMT_IS_AND_GET(DeferStmt)
    DEFINE_STMT_IS_AND_GET(IfStmt)
    DEFINE_STMT_IS_AND_GET(SwitchStmt)
    DEFINE_STMT_IS_AND_GET(WhileStmt)
    DEFINE_STMT_IS_AND_GET(ForStmt)
    DEFINE_STMT_IS_AND_GET(BreakStmt)
    DEFINE_STMT_IS_AND_GET(AssignStmt)
    DEFINE_STMT_IS_AND_GET(AugAssignStmt)
#undef DEFINE_STMT_IS_AND_GET

    StmtKind getKind() const { return kind; }

protected:
    Stmt(StmtKind kind) : kind(kind) { }

private:
    const StmtKind kind;
};

inline Stmt::~Stmt() { }

class ReturnStmt : public Stmt {
public:
    std::vector<std::unique_ptr<Expr>> values;

    ReturnStmt(std::vector<std::unique_ptr<Expr>>&& values, SourceLocation location)
    : Stmt(StmtKind::ReturnStmt), values(std::move(values)), location(location) { }
    SourceLocation getLocation() const { return location; }
    static bool classof(const Stmt* s) { return s->getKind() == StmtKind::ReturnStmt; }

private:
    SourceLocation location;
};

class VarStmt : public Stmt {
public:
    std::unique_ptr<VarDecl> decl;

    VarStmt(std::unique_ptr<VarDecl> decl)
    : Stmt(StmtKind::VarStmt), decl(std::move(decl)) { }
    static bool classof(const Stmt* s) { return s->getKind() == StmtKind::VarStmt; }
};

class IncrementStmt : public Stmt {
public:
    std::unique_ptr<Expr> operand;

    IncrementStmt(std::unique_ptr<Expr> operand, SourceLocation location)
    : Stmt(StmtKind::IncrementStmt), operand(std::move(operand)), location(location) { }
    SourceLocation getLocation() const { return location; }
    static bool classof(const Stmt* s) { return s->getKind() == StmtKind::IncrementStmt; }

private:
    SourceLocation location; // Location of '++'.
};

class DecrementStmt : public Stmt {
public:
    std::unique_ptr<Expr> operand;

    DecrementStmt(std::unique_ptr<Expr> operand, SourceLocation location)
    : Stmt(StmtKind::DecrementStmt), operand(std::move(operand)), location(location) { }
    SourceLocation getLocation() const { return location; }
    static bool classof(const Stmt* s) { return s->getKind() == StmtKind::DecrementStmt; }

private:
    SourceLocation location; // Location of '--'.
};

/// A statement that consists of the evaluation of a single expression.
class ExprStmt : public Stmt {
public:
    std::unique_ptr<Expr> expr;

    ExprStmt(std::unique_ptr<Expr> expr)
    : Stmt(StmtKind::ExprStmt), expr(std::move(expr)) { }
    static bool classof(const Stmt* s) { return s->getKind() == StmtKind::ExprStmt; }
};

class DeferStmt : public Stmt {
public:
    std::unique_ptr<Expr> expr;

    DeferStmt(std::unique_ptr<Expr> expr)
    : Stmt(StmtKind::DeferStmt), expr(std::move(expr)) { }
    static bool classof(const Stmt* s) { return s->getKind() == StmtKind::DeferStmt; }
};

class IfStmt : public Stmt {
public:
    std::unique_ptr<Expr> condition;
    std::vector<std::unique_ptr<Stmt>> thenBody;
    std::vector<std::unique_ptr<Stmt>> elseBody;

    IfStmt(std::unique_ptr<Expr> condition, std::vector<std::unique_ptr<Stmt>>&& thenBody,
           std::vector<std::unique_ptr<Stmt>>&& elseBody)
    : Stmt(StmtKind::IfStmt), condition(std::move(condition)),
      thenBody(std::move(thenBody)), elseBody(std::move(elseBody)) { }
    static bool classof(const Stmt* s) { return s->getKind() == StmtKind::IfStmt; }
};

class SwitchCase {
public:
    std::unique_ptr<Expr> value;
    std::vector<std::unique_ptr<Stmt>> stmts;
};

class SwitchStmt : public Stmt {
public:
    std::unique_ptr<Expr> condition;
    std::vector<SwitchCase> cases;
    std::vector<std::unique_ptr<Stmt>> defaultStmts;

    SwitchStmt(std::unique_ptr<Expr> condition, std::vector<SwitchCase>&& cases,
               std::vector<std::unique_ptr<Stmt>>&& defaultStmts)
    : Stmt(StmtKind::SwitchStmt), condition(std::move(condition)),
      cases(std::move(cases)), defaultStmts(std::move(defaultStmts)) { }
    static bool classof(const Stmt* s) { return s->getKind() == StmtKind::SwitchStmt; }
};

class WhileStmt : public Stmt {
public:
    std::unique_ptr<Expr> condition;
    std::vector<std::unique_ptr<Stmt>> body;

    WhileStmt(std::unique_ptr<Expr> condition, std::vector<std::unique_ptr<Stmt>>&& body)
    : Stmt(StmtKind::WhileStmt), condition(std::move(condition)), body(std::move(body)) { }
    static bool classof(const Stmt* s) { return s->getKind() == StmtKind::WhileStmt; }
};

class ForStmt : public Stmt {
public:
    std::string id;
    std::unique_ptr<Expr> range;
    std::vector<std::unique_ptr<Stmt>> body;

    ForStmt(std::string&& id, std::unique_ptr<Expr> range,
            std::vector<std::unique_ptr<Stmt>>&& body, SourceLocation location)
    : Stmt(StmtKind::ForStmt), id(std::move(id)), range(std::move(range)),
      body(std::move(body)), location(location) { }
    SourceLocation getLocation() const { return location; }
    static bool classof(const Stmt* s) { return s->getKind() == StmtKind::ForStmt; }

private:
    SourceLocation location; // Location of 'id'.
};

class BreakStmt : public Stmt {
public:
    BreakStmt(SourceLocation location) : Stmt(StmtKind::BreakStmt), location(location) { }
    SourceLocation getLocation() const { return location; }
    static bool classof(const Stmt* s) { return s->getKind() == StmtKind::BreakStmt; }

private:
    SourceLocation location;
};

class AssignStmt : public Stmt {
public:
    std::unique_ptr<Expr> lhs;
    std::unique_ptr<Expr> rhs;

    AssignStmt(std::unique_ptr<Expr> lhs, std::unique_ptr<Expr> rhs, SourceLocation location)
    : Stmt(StmtKind::AssignStmt), lhs(std::move(lhs)), rhs(std::move(rhs)), location(location) { }
    SourceLocation getLocation() const { return location; }
    static bool classof(const Stmt* s) { return s->getKind() == StmtKind::AssignStmt; }

private:
    SourceLocation location; // Location of '='.
};

/// An augmented assignment (a.k.a. compound assignment) statement.
class AugAssignStmt : public Stmt {
public:
    std::unique_ptr<Expr> lhs;
    std::unique_ptr<Expr> rhs;
    BinaryOperator op;

    AugAssignStmt(std::unique_ptr<Expr> lhs, std::unique_ptr<Expr> rhs,
                  BinaryOperator op, SourceLocation location)
    : Stmt(StmtKind::AugAssignStmt), lhs(std::move(lhs)), rhs(std::move(rhs)),
      op(op), location(location) { }
    SourceLocation getLocation() const { return location; }
    static bool classof(const Stmt* s) { return s->getKind() == StmtKind::AugAssignStmt; }

private:
    SourceLocation location; // Location of operator symbol.
};

}
