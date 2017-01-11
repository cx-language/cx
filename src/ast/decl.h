#pragma once

#include <string>
#include <memory>
#include <cassert>
#include <vector>
#include <boost/variant.hpp>
#include <boost/optional.hpp>
#include "expr.h"
#include "stmt.h"
#include "type.h"

enum class DeclKind {
    ParamDecl,
    FuncDecl,
    TypeDecl,
    VarDecl,
};

struct ParamDecl {
    Type type;
    std::string name;
};

struct FuncDecl {
    std::string name;
    std::vector<ParamDecl> params;
    Type returnType;
    std::shared_ptr<std::vector<Stmt>> body;
    bool isExtern() const { return body == nullptr; };
    FuncType getFuncType() const;
};

enum class TypeTag { Struct, Class };

struct TypeDecl {
    TypeTag tag;
    std::string name;
    Type getType() const;
};

struct VarDecl {
    /// The explicitly declared type, or a bool indicating whether the inferred
    /// type should be mutable.
    boost::variant<Type, bool> type;
    std::string name;
    std::shared_ptr<Expr> initializer;

    boost::optional<const Type&> getDeclaredType() const {
        if (type.which() == 0) return boost::get<Type>(type);
        else return boost::none;
    }
    const Type& getType() const {
        return boost::get<Type>(type);
    }
    bool isMutable() const { return boost::get<bool>(type); }
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
    DEFINE_DECLKIND_GETTER_AND_CONSTRUCTOR(TypeDecl)
    DEFINE_DECLKIND_GETTER_AND_CONSTRUCTOR(VarDecl)
#undef DEFINE_DECLKIND_GETTER_AND_CONSTRUCTOR

    Decl(Decl&& decl) : data(std::move(decl.data)) { }
    DeclKind getKind() const { return static_cast<DeclKind>(data.which()); }

private:
    boost::variant<ParamDecl, FuncDecl, TypeDecl, VarDecl> data;
};
