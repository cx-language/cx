#pragma once

#include <string>
#include <memory>
#include <vector>
#include <boost/variant.hpp>
#include "expr.h"
#include "stmt.h"
#include "type.h"
#include "srcloc.h"

namespace llvm { class StringRef; }

namespace delta {

enum class DeclKind {
    ParamDecl,
    FuncDecl,
    GenericParamDecl,
    GenericFuncDecl,
    InitDecl, /// A struct or class initializer declaration.
    DeinitDecl, /// A struct or class deinitializer declaration.
    TypeDecl,
    VarDecl,
    FieldDecl, /// A struct or class field declaration.
    ImportDecl,
};

class Decl {
public:
    virtual ~Decl() = 0;

#define DEFINE_DECL_IS_AND_GET(KIND) \
    bool is##KIND() const { return getKind() == DeclKind::KIND; } \
    class KIND& get##KIND() { return llvm::cast<class KIND>(*this); } \
    const class KIND& get##KIND() const { return llvm::cast<class KIND>(*this); }
    DEFINE_DECL_IS_AND_GET(ParamDecl)
    DEFINE_DECL_IS_AND_GET(FuncDecl)
    DEFINE_DECL_IS_AND_GET(GenericParamDecl)
    DEFINE_DECL_IS_AND_GET(GenericFuncDecl)
    DEFINE_DECL_IS_AND_GET(InitDecl)
    DEFINE_DECL_IS_AND_GET(DeinitDecl)
    DEFINE_DECL_IS_AND_GET(TypeDecl)
    DEFINE_DECL_IS_AND_GET(VarDecl)
    DEFINE_DECL_IS_AND_GET(FieldDecl)
    DEFINE_DECL_IS_AND_GET(ImportDecl)
#undef DEFINE_DECL_IS_AND_GET

    DeclKind getKind() const { return kind; }
    SrcLoc getSrcLoc() const;

protected:
    Decl(DeclKind kind) : kind(kind) { }

private:
    const DeclKind kind;
};

inline Decl::~Decl() { }

class ParamDecl : public Decl {
public:
    std::string label; // Empty if no label.
    Type type;
    std::string name;
    SrcLoc srcLoc;

    ParamDecl(std::string&& label, Type type, std::string&& name, SrcLoc srcLoc)
    : Decl(DeclKind::ParamDecl), label(std::move(label)), type(type),
      name(std::move(name)), srcLoc(srcLoc) { }
    static bool classof(const Decl* d) { return d->getKind() == DeclKind::ParamDecl; }
};

class GenericParamDecl : public Decl {
public:
    std::string name;
    SrcLoc srcLoc;

    GenericParamDecl(std::string&& name, SrcLoc srcLoc)
    : Decl(DeclKind::GenericParamDecl), name(std::move(name)), srcLoc(srcLoc) { }
    static bool classof(const Decl* d) { return d->getKind() == DeclKind::GenericParamDecl; }
};

class FuncDecl : public Decl {
public:
    std::string name;
    std::vector<ParamDecl> params;
    Type returnType;
    std::string receiverType; /// Empty if non-member function.
    bool mutating;
    std::shared_ptr<std::vector<std::unique_ptr<Stmt>>> body;
    SrcLoc srcLoc;

    FuncDecl(std::string&& name, std::vector<ParamDecl>&& params, Type returnType,
             std::string&& receiverType, SrcLoc srcLoc)
    : Decl(DeclKind::FuncDecl), name(std::move(name)), params(std::move(params)),
      returnType(returnType), receiverType(std::move(receiverType)), mutating(false),
      srcLoc(srcLoc) { }
    bool isExtern() const { return body == nullptr; };
    bool isMemberFunc() const { return !receiverType.empty(); }
    bool isMutating() const { return mutating; }
    void setMutating(bool m) { mutating = m; }
    const FuncType* getFuncType() const;
    static bool classof(const Decl* d) { return d->getKind() == DeclKind::FuncDecl; }
};

class GenericFuncDecl : public Decl {
public:
    std::shared_ptr<FuncDecl> func;
    std::vector<GenericParamDecl> genericParams;

    GenericFuncDecl(std::shared_ptr<FuncDecl>&& func,
                    std::vector<GenericParamDecl>&& genericParams)
    : Decl(DeclKind::GenericFuncDecl), func(std::move(func)),
      genericParams(std::move(genericParams)) { }
    static bool classof(const Decl* d) { return d->getKind() == DeclKind::GenericFuncDecl; }
};

class InitDecl : public Decl {
public:
    /// The name of the struct or class this initializer initializes, or (after
    /// type checking) a pointer to the corresponding type declaration.
    boost::variant<std::string, TypeDecl*> type;
    std::vector<ParamDecl> params;
    std::shared_ptr<std::vector<std::unique_ptr<Stmt>>> body;
    SrcLoc srcLoc;

    InitDecl(std::string&& type, std::vector<ParamDecl>&& params,
             std::shared_ptr<std::vector<std::unique_ptr<Stmt>>>&& body, SrcLoc srcLoc)
    : Decl(DeclKind::InitDecl), type(std::move(type)), params(std::move(params)),
      body(std::move(body)), srcLoc(srcLoc) { }
    TypeDecl& getTypeDecl() const { return *boost::get<TypeDecl*>(type); }
    llvm::StringRef getTypeName() const;
    static bool classof(const Decl* d) { return d->getKind() == DeclKind::InitDecl; }
};

class DeinitDecl : public Decl {
public:
    /// The name of the struct or class this deinitializer deinitializes, or (after
    /// type checking) a pointer to the corresponding type declaration.
    boost::variant<std::string, TypeDecl*> type;
    std::shared_ptr<std::vector<std::unique_ptr<Stmt>>> body;
    SrcLoc srcLoc;

    DeinitDecl(std::string&& type,
               std::shared_ptr<std::vector<std::unique_ptr<Stmt>>>&& body, SrcLoc srcLoc)
    : Decl(DeclKind::DeinitDecl), type(std::move(type)), body(std::move(body)),
      srcLoc(srcLoc) { }
    TypeDecl& getTypeDecl() const { return *boost::get<TypeDecl*>(type); }
    llvm::StringRef getTypeName() const;
    static bool classof(const Decl* d) { return d->getKind() == DeclKind::DeinitDecl; }
};

enum class TypeTag { Struct, Class };

class TypeDecl : public Decl {
public:
    TypeTag tag;
    std::string name;
    std::vector<FieldDecl> fields;
    SrcLoc srcLoc;

    TypeDecl(TypeTag tag, std::string&& name, std::vector<FieldDecl>&& fields, SrcLoc srcLoc)
    : Decl(DeclKind::TypeDecl), tag(tag), name(std::move(name)), fields(std::move(fields)),
      srcLoc(srcLoc) { }
    Type getType(bool isMutable = false) const;
    Type getTypeForPassing(bool isMutable = false) const; /// 'T&' if this is class, or plain 'T' otherwise.
    bool passByValue() const { return isStruct(); }
    bool isStruct() const { return tag == TypeTag::Struct; }
    bool isClass() const { return tag == TypeTag::Class; }
    unsigned getFieldIndex(llvm::StringRef fieldName) const;
    static bool classof(const Decl* d) { return d->getKind() == DeclKind::TypeDecl; }
};

class VarDecl : public Decl {
public:
    Type type;
    std::string name;
    std::shared_ptr<Expr> initializer; /// Null if the initializer is 'uninitialized'.
    SrcLoc srcLoc;

    VarDecl(Type type, std::string&& name, std::shared_ptr<Expr>&& initializer, SrcLoc srcLoc)
    : Decl(DeclKind::VarDecl), type(type), name(std::move(name)),
      initializer(std::move(initializer)), srcLoc(srcLoc) { }
    Type getType() const { return type; }
    static bool classof(const Decl* d) { return d->getKind() == DeclKind::VarDecl; }
};

class FieldDecl : public Decl {
public:
    Type type;
    std::string name;
    SrcLoc srcLoc;

    FieldDecl(Type type, std::string&& name, SrcLoc srcLoc)
    : Decl(DeclKind::FieldDecl), type(type), name(std::move(name)), srcLoc(srcLoc) { }
    static bool classof(const Decl* d) { return d->getKind() == DeclKind::FieldDecl; }
};

class ImportDecl : public Decl {
public:
    std::string target;
    SrcLoc srcLoc;

    ImportDecl(std::string&& target, SrcLoc srcLoc)
    : Decl(DeclKind::ImportDecl), target(std::move(target)), srcLoc(srcLoc) { }
    static bool classof(const Decl* d) { return d->getKind() == DeclKind::ImportDecl; }
};

}
