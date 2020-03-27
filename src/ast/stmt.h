#pragma once

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
    ForEachStmt,
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
    bool isForEachStmt() const { return getKind() == StmtKind::ForEachStmt; }
    bool isBreakStmt() const { return getKind() == StmtKind::BreakStmt; }
    bool isContinueStmt() const { return getKind() == StmtKind::ContinueStmt; }
    bool isCompoundStmt() const { return getKind() == StmtKind::CompoundStmt; }

    StmtKind getKind() const { return kind; }
    bool isBreakable() const;
    bool isContinuable() const;
    Stmt* instantiate(const llvm::StringMap<Type>& genericArgs) const;

protected:
    Stmt(StmtKind kind) : kind(kind) {}

private:
    const StmtKind kind;
};

inline Stmt::~Stmt() {}

class ReturnStmt : public Stmt {
public:
    ReturnStmt(Expr* value, SourceLocation location) : Stmt(StmtKind::ReturnStmt), value(value), location(location) {}
    Expr* getReturnValue() const { return value; }
    SourceLocation getLocation() const { return location; }
    static bool classof(const Stmt* s) { return s->getKind() == StmtKind::ReturnStmt; }

private:
    Expr* value;
    SourceLocation location;
};

class VarStmt : public Stmt {
public:
    VarStmt(VarDecl* decl) : Stmt(StmtKind::VarStmt), decl(decl) {}
    VarDecl& getDecl() const { return *decl; }
    static bool classof(const Stmt* s) { return s->getKind() == StmtKind::VarStmt; }

private:
    VarDecl* decl;
};

/// A statement that consists of the evaluation of a single expression.
class ExprStmt : public Stmt {
public:
    ExprStmt(Expr* expr) : Stmt(StmtKind::ExprStmt), expr(expr) {}
    Expr& getExpr() const { return *expr; }
    static bool classof(const Stmt* s) { return s->getKind() == StmtKind::ExprStmt; }

private:
    Expr* expr;
};

class DeferStmt : public Stmt {
public:
    DeferStmt(Expr* expr) : Stmt(StmtKind::DeferStmt), expr(expr) {}
    Expr& getExpr() const { return *expr; }
    static bool classof(const Stmt* s) { return s->getKind() == StmtKind::DeferStmt; }

private:
    Expr* expr;
};

class IfStmt : public Stmt {
public:
    IfStmt(Expr* condition, std::vector<Stmt*>&& thenBody, std::vector<Stmt*>&& elseBody)
    : Stmt(StmtKind::IfStmt), condition(condition), thenBody(std::move(thenBody)), elseBody(std::move(elseBody)) {}
    Expr& getCondition() const { return *condition; }
    llvm::ArrayRef<Stmt*> getThenBody() const { return thenBody; }
    llvm::ArrayRef<Stmt*> getElseBody() const { return elseBody; }
    llvm::MutableArrayRef<Stmt*> getThenBody() { return thenBody; }
    llvm::MutableArrayRef<Stmt*> getElseBody() { return elseBody; }
    static bool classof(const Stmt* s) { return s->getKind() == StmtKind::IfStmt; }

private:
    Expr* condition;
    std::vector<Stmt*> thenBody;
    std::vector<Stmt*> elseBody;
};

class SwitchCase {
public:
    SwitchCase(Expr* value, VarDecl* associatedValue, std::vector<Stmt*>&& stmts)
    : value(value), associatedValue(associatedValue), stmts(std::move(stmts)) {}
    Expr* getValue() const { return value; }
    VarDecl* getAssociatedValue() const { return associatedValue; }
    llvm::ArrayRef<Stmt*> getStmts() const { return stmts; }
    llvm::MutableArrayRef<Stmt*> getStmts() { return stmts; }

private:
    Expr* value;
    VarDecl* associatedValue;
    std::vector<Stmt*> stmts;
};

class SwitchStmt : public Stmt {
public:
    SwitchStmt(Expr* condition, std::vector<SwitchCase>&& cases, std::vector<Stmt*>&& defaultStmts)
    : Stmt(StmtKind::SwitchStmt), condition(condition), cases(std::move(cases)), defaultStmts(std::move(defaultStmts)) {}
    Expr& getCondition() const { return *condition; }
    llvm::ArrayRef<SwitchCase> getCases() const { return cases; }
    llvm::MutableArrayRef<SwitchCase> getCases() { return cases; }
    llvm::ArrayRef<Stmt*> getDefaultStmts() const { return defaultStmts; }
    llvm::MutableArrayRef<Stmt*> getDefaultStmts() { return defaultStmts; }
    static bool classof(const Stmt* s) { return s->getKind() == StmtKind::SwitchStmt; }

private:
    Expr* condition;
    std::vector<SwitchCase> cases;
    std::vector<Stmt*> defaultStmts;
};

class WhileStmt : public Stmt {
public:
    WhileStmt(Expr* condition, std::vector<Stmt*>&& body, SourceLocation location)
    : Stmt(StmtKind::WhileStmt), condition(condition), body(std::move(body)), location(location) {}
    Expr& getCondition() const { return *condition; }
    llvm::ArrayRef<Stmt*> getBody() const { return body; }
    llvm::MutableArrayRef<Stmt*> getBody() { return body; }
    SourceLocation getLocation() const { return location; }
    Stmt* lower();
    static bool classof(const Stmt* s) { return s->getKind() == StmtKind::WhileStmt; }

private:
    Expr* condition;
    std::vector<Stmt*> body;
    SourceLocation location;
};

class ForStmt : public Stmt {
public:
    ForStmt(VarStmt* variable, Expr* condition, Expr* increment, std::vector<Stmt*>&& body, SourceLocation location)
    : Stmt(StmtKind::ForStmt), variable(variable), condition(condition), increment(increment), body(std::move(body)), location(location) {}
    VarStmt* getVariable() const { return variable; }
    Expr* getCondition() const { return condition; }
    Expr* getIncrement() const { return increment; }
    llvm::ArrayRef<Stmt*> getBody() const { return body; }
    llvm::MutableArrayRef<Stmt*> getBody() { return body; }
    SourceLocation getLocation() const { return location; }
    static bool classof(const Stmt* s) { return s->getKind() == StmtKind::ForStmt; }

private:
    VarStmt* variable;
    Expr* condition;
    Expr* increment;
    std::vector<Stmt*> body;
    SourceLocation location;
};

class ForEachStmt : public Stmt {
public:
    ForEachStmt(VarDecl* variable, Expr* range, std::vector<Stmt*>&& body, SourceLocation location)
    : Stmt(StmtKind::ForEachStmt), variable(variable), range(range), body(std::move(body)), location(location) {}
    VarDecl* getVariable() const { return variable; }
    Expr& getRangeExpr() const { return *range; }
    llvm::ArrayRef<Stmt*> getBody() const { return body; }
    SourceLocation getLocation() const { return location; }
    Stmt* lower(int nestLevel);
    static bool classof(const Stmt* s) { return s->getKind() == StmtKind::ForEachStmt; }

private:
    VarDecl* variable;
    Expr* range;
    std::vector<Stmt*> body;
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
    CompoundStmt(std::vector<Stmt*>&& body) : Stmt(StmtKind::CompoundStmt), body(std::move(body)) {}
    llvm::ArrayRef<Stmt*> getBody() const { return body; }
    llvm::MutableArrayRef<Stmt*> getBody() { return body; }
    static bool classof(const Stmt* s) { return s->getKind() == StmtKind::CompoundStmt; }

private:
    std::vector<Stmt*> body;
};

} // namespace delta
