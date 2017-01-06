#pragma once

#include <string>
#include <memory>
#include <cassert>
#include <vector>
#include <boost/variant.hpp>
#include "expr.h"
#include "stmt.h"
#include "type.h"

enum class DeclType {
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
#define DEFINE_DECL_TYPE_GETTER_AND_CONSTRUCTOR(type) \
    Decl(type&& value) : data(std::move(value)) { } \
    \
    type& get##type() { \
        assert(getType() == DeclType::type); \
        return boost::get<type>(data); \
    } \
    const type& get##type() const { \
        assert(getType() == DeclType::type); \
        return boost::get<type>(data); \
    }
    DEFINE_DECL_TYPE_GETTER_AND_CONSTRUCTOR(ParamDecl)
    DEFINE_DECL_TYPE_GETTER_AND_CONSTRUCTOR(FuncDecl)
    DEFINE_DECL_TYPE_GETTER_AND_CONSTRUCTOR(VarDecl)
#undef DEFINE_DECL_TYPE_GETTER_AND_CONSTRUCTOR

    Decl(Decl&& decl) : data(std::move(decl.data)) { }
    DeclType getType() const { return static_cast<DeclType>(data.which()); }

private:
    boost::variant<ParamDecl, FuncDecl, VarDecl> data;
};
