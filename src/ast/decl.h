#pragma once

#include <string>
#include <memory>
#include <cassert>
#include <vector>
#include <boost/variant.hpp>
#include "expr.h"
#include "stmt.h"
#include "type.h"

enum class DeclKind {
    ParamDecl,
    FuncDecl,
    VarDecl,
};

struct ParamDecl {
    std::string type;
    std::string name;
};

struct FuncDecl {
    std::string name;
    std::vector<ParamDecl> params;
    Type returnType;
    std::vector<Stmt> body;
};

struct VarDecl {
    std::string name;
    Expr initializer;
};

class Decl {
public:
#define DEFINE_DECLKIND_GETTER_AND_CONSTRUCTOR(KIND) \
    Decl(KIND&& value) : data(std::move(value)) { } \
    \
    KIND& get##KIND() { \
        assert(getKind() == DeclKind::KIND); \
        return boost::get<KIND>(data); \
    } \
    const KIND& get##KIND() const { \
        assert(getKind() == DeclKind::KIND); \
        return boost::get<KIND>(data); \
    }
    DEFINE_DECLKIND_GETTER_AND_CONSTRUCTOR(ParamDecl)
    DEFINE_DECLKIND_GETTER_AND_CONSTRUCTOR(FuncDecl)
    DEFINE_DECLKIND_GETTER_AND_CONSTRUCTOR(VarDecl)
#undef DEFINE_DECLKIND_GETTER_AND_CONSTRUCTOR

    Decl(Decl&& decl) : data(std::move(decl.data)) { }
    DeclKind getKind() const { return static_cast<DeclKind>(data.which()); }

private:
    boost::variant<ParamDecl, FuncDecl, VarDecl> data;
};
