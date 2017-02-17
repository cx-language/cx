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
#include "srcloc.h"

namespace llvm { class StringRef; }

namespace delta {

enum class DeclKind {
    ParamDecl,
    FuncDecl,
    InitDecl, /// A struct or class initializer declaration.
    DeinitDecl, /// A struct or class deinitializer declaration.
    TypeDecl,
    VarDecl,
    FieldDecl, /// A struct or class field declaration.
    ImportDecl,
};

struct ParamDecl {
    std::string label; // Empty if no label.
    Type type;
    std::string name;
    SrcLoc srcLoc;
};

struct FuncDecl {
    std::string name;
    std::vector<ParamDecl> params;
    Type returnType;
    std::string receiverType; /// Empty if non-member function.
    std::shared_ptr<std::vector<Stmt>> body;
    SrcLoc srcLoc;

    bool isExtern() const { return body == nullptr; };
    bool isMemberFunc() const { return !receiverType.empty(); }
    FuncType getFuncType() const;
};

struct InitDecl {
    /// The name of the struct or class this initializer initializes, or (after
    /// type checking) a pointer to the corresponding type declaration.
    boost::variant<std::string, struct TypeDecl*> type;
    std::vector<ParamDecl> params;
    std::shared_ptr<std::vector<Stmt>> body;
    SrcLoc srcLoc;

    TypeDecl& getTypeDecl() const { return *boost::get<TypeDecl*>(type); }
    const std::string& getTypeName() const { return boost::get<std::string>(type); }
};

struct DeinitDecl {
    /// The name of the struct or class this deinitializer deinitializes, or (after
    /// type checking) a pointer to the corresponding type declaration.
    boost::variant<std::string, struct TypeDecl*> type;
    std::shared_ptr<std::vector<Stmt>> body;
    SrcLoc srcLoc;

    TypeDecl& getTypeDecl() const { return *boost::get<TypeDecl*>(type); }
    const std::string& getTypeName() const { return boost::get<std::string>(type); }
};

enum class TypeTag { Struct, Class };

struct TypeDecl {
    TypeTag tag;
    std::string name;
    std::vector<struct FieldDecl> fields;
    SrcLoc srcLoc;

    Type getType() const;
    Type getTypeForPassing() const; /// 'T&' if this is class, or plain 'T' otherwise.
    bool passByValue() const { return isStruct(); }
    bool isStruct() const { return tag == TypeTag::Struct; }
    bool isClass() const { return tag == TypeTag::Class; }
    unsigned getFieldIndex(llvm::StringRef fieldName) const;
};

struct VarDecl {
    /// The explicitly declared type, or a bool indicating whether the inferred
    /// type should be mutable.
    boost::variant<Type, bool> type;
    std::string name;
    std::shared_ptr<Expr> initializer; /// Null if the initializer is 'uninitialized'.
    SrcLoc srcLoc;

    boost::optional<const Type&> getDeclaredType() const {
        if (type.which() == 0) return boost::get<Type>(type);
        else return boost::none;
    }
    const Type& getType() const {
        return boost::get<Type>(type);
    }
    bool isMutable() const {
        if (type.which() == 1) return boost::get<bool>(type);
        else return getType().isMutable();
    }
};

struct FieldDecl {
    Type type;
    std::string name;
    SrcLoc srcLoc;
};

struct ImportDecl {
    std::string target;
    SrcLoc srcLoc;
};

class Decl {
public:
#define DEFINE_DECLKIND_GETTER_AND_CONSTRUCTOR(KIND) \
    Decl(KIND&& value) : data(std::move(value)) { } \
    \
    bool is##KIND() const { return getKind() == DeclKind::KIND; } \
    \
    KIND& get##KIND() { \
        assert(is##KIND()); \
        return boost::get<KIND>(data); \
    } \
    const KIND& get##KIND() const { \
        assert(is##KIND()); \
        return boost::get<KIND>(data); \
    }
    DEFINE_DECLKIND_GETTER_AND_CONSTRUCTOR(ParamDecl)
    DEFINE_DECLKIND_GETTER_AND_CONSTRUCTOR(FuncDecl)
    DEFINE_DECLKIND_GETTER_AND_CONSTRUCTOR(InitDecl)
    DEFINE_DECLKIND_GETTER_AND_CONSTRUCTOR(DeinitDecl)
    DEFINE_DECLKIND_GETTER_AND_CONSTRUCTOR(TypeDecl)
    DEFINE_DECLKIND_GETTER_AND_CONSTRUCTOR(VarDecl)
    DEFINE_DECLKIND_GETTER_AND_CONSTRUCTOR(FieldDecl)
    DEFINE_DECLKIND_GETTER_AND_CONSTRUCTOR(ImportDecl)
#undef DEFINE_DECLKIND_GETTER_AND_CONSTRUCTOR

    Decl(Decl&& decl) : data(std::move(decl.data)) { }
    DeclKind getKind() const { return static_cast<DeclKind>(data.which()); }
    SrcLoc getSrcLoc() const;

private:
    boost::variant<ParamDecl, FuncDecl, InitDecl, DeinitDecl, TypeDecl, VarDecl,
        FieldDecl, ImportDecl> data;
};

}
