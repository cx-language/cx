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
    InitDecl, /// A struct or class initializer declaration.
    TypeDecl,
    VarDecl,
    FieldDecl, /// A struct or class field declaration.
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

struct InitDecl {
    /// The name of the struct or class this initializer initializes, or (after
    /// type checking) a pointer to the corresponding type declaration.
    boost::variant<std::string, struct TypeDecl*> type;
    std::vector<ParamDecl> params;
    std::shared_ptr<std::vector<Stmt>> body;
    TypeDecl& getTypeDecl() const { return *boost::get<TypeDecl*>(type); }
    const std::string& getTypeName() const { return boost::get<std::string>(type); }
};

enum class TypeTag { Struct, Class };

struct TypeDecl {
    TypeTag tag;
    std::string name;
    std::vector<struct FieldDecl> fields;
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

struct FieldDecl {
    Type type;
    std::string name;
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
    DEFINE_DECLKIND_GETTER_AND_CONSTRUCTOR(InitDecl)
    DEFINE_DECLKIND_GETTER_AND_CONSTRUCTOR(TypeDecl)
    DEFINE_DECLKIND_GETTER_AND_CONSTRUCTOR(VarDecl)
    DEFINE_DECLKIND_GETTER_AND_CONSTRUCTOR(FieldDecl)
#undef DEFINE_DECLKIND_GETTER_AND_CONSTRUCTOR

    Decl(Decl&& decl) : data(std::move(decl.data)) { }
    DeclKind getKind() const { return static_cast<DeclKind>(data.which()); }

private:
    boost::variant<ParamDecl, FuncDecl, InitDecl, TypeDecl, VarDecl, FieldDecl> data;
};
