#pragma once

#include <string>
#include <memory>
#include <vector>
#include <boost/variant.hpp>
#include <llvm/ADT/SmallVector.h>
#include <llvm/Support/Casting.h>
#include "expr.h"
#include "stmt.h"
#include "type.h"
#include "srcloc.h"

namespace llvm { class StringRef; }

namespace delta {

class Module;

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
    Module* getModule() const { return module; }
    SrcLoc getSrcLoc() const;

protected:
    Decl(DeclKind kind, Module* module) : kind(kind), module(module) { }

private:
    const DeclKind kind;
    Module* module;
};

inline Decl::~Decl() { }

class ParamDecl : public Decl {
public:
    Type type;
    std::string name;
    SrcLoc srcLoc;

    ParamDecl(Type type, std::string&& name, SrcLoc srcLoc)
    : Decl(DeclKind::ParamDecl, nullptr), type(type), name(std::move(name)), srcLoc(srcLoc) { }
    static bool classof(const Decl* d) { return d->getKind() == DeclKind::ParamDecl; }
    bool operator==(const ParamDecl& other) const {
        return type == other.type && name == other.name;
    }
};

class GenericParamDecl : public Decl {
public:
    std::string name;
    SrcLoc srcLoc;
    llvm::SmallVector<std::string, 1> constraints;

    GenericParamDecl(std::string&& name, SrcLoc srcLoc)
    : Decl(DeclKind::GenericParamDecl, nullptr), name(std::move(name)), srcLoc(srcLoc) { }
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
             std::string&& receiverType, Module* module, SrcLoc srcLoc)
    : Decl(DeclKind::FuncDecl, module), name(std::move(name)), params(std::move(params)),
      returnType(returnType), receiverType(std::move(receiverType)), mutating(false),
      srcLoc(srcLoc) { }
    bool isExtern() const { return body == nullptr; };
    bool isMemberFunc() const { return !receiverType.empty(); }
    bool isMutating() const { return mutating; }
    void setMutating(bool m) { mutating = m; }
    const FuncType* getFuncType() const;
    bool signatureMatches(const FuncDecl& other, bool matchReceiver = true) const;
    static bool classof(const Decl* d) { return d->getKind() == DeclKind::FuncDecl; }
};

class GenericFuncDecl : public Decl {
public:
    std::shared_ptr<FuncDecl> func;
    std::vector<GenericParamDecl> genericParams;

    GenericFuncDecl(std::shared_ptr<FuncDecl>&& func,
                    std::vector<GenericParamDecl>&& genericParams)
    : Decl(DeclKind::GenericFuncDecl, func->getModule()), func(std::move(func)),
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
    : Decl(DeclKind::InitDecl, nullptr), type(std::move(type)), params(std::move(params)),
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
    : Decl(DeclKind::DeinitDecl, nullptr), type(std::move(type)), body(std::move(body)),
      srcLoc(srcLoc) { }
    TypeDecl& getTypeDecl() const { return *boost::get<TypeDecl*>(type); }
    llvm::StringRef getTypeName() const;
    static bool classof(const Decl* d) { return d->getKind() == DeclKind::DeinitDecl; }
};

enum class TypeTag { Struct, Class, Interface, Union };

class TypeDecl : public Decl {
public:
    TypeTag tag;
    std::string name;
    std::vector<FieldDecl> fields;
    std::vector<std::unique_ptr<FuncDecl>> memberFuncs; // Only used by interfaces.
    std::vector<GenericParamDecl> genericParams;
    SrcLoc srcLoc;

    TypeDecl(TypeTag tag, std::string&& name, std::vector<FieldDecl>&& fields,
             std::vector<std::unique_ptr<FuncDecl>>&& memberFuncs,
             std::vector<GenericParamDecl>&& genericParams, Module* module, SrcLoc srcLoc)
    : Decl(DeclKind::TypeDecl, module), tag(tag), name(std::move(name)),
      fields(std::move(fields)), memberFuncs(std::move(memberFuncs)),
      genericParams(std::move(genericParams)), srcLoc(srcLoc) { }

    Type getType(bool isMutable = false, std::vector<Type>&& genericArgs = {}) const;
    Type getTypeForPassing(bool isMutable = false) const; /// 'T&' if this is class, or plain 'T' otherwise.
    bool passByValue() const { return isStruct() || isUnion(); }
    bool isStruct() const { return tag == TypeTag::Struct; }
    bool isClass() const { return tag == TypeTag::Class; }
    bool isInterface() const { return tag == TypeTag::Interface; }
    bool isUnion() const { return tag == TypeTag::Union; }
    bool isGeneric() const { return !genericParams.empty(); }
    unsigned getFieldIndex(llvm::StringRef fieldName) const;
    static bool classof(const Decl* d) { return d->getKind() == DeclKind::TypeDecl; }
};

class VarDecl : public Decl {
public:
    Type type;
    std::string name;
    std::shared_ptr<Expr> initializer; /// Null if the initializer is 'uninitialized'.
    SrcLoc srcLoc;

    VarDecl(Type type, std::string&& name, std::shared_ptr<Expr>&& initializer,
            Module* module, SrcLoc srcLoc)
    : Decl(DeclKind::VarDecl, module), type(type), name(std::move(name)),
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
    : Decl(DeclKind::FieldDecl, nullptr), type(type), name(std::move(name)), srcLoc(srcLoc) { }
    static bool classof(const Decl* d) { return d->getKind() == DeclKind::FieldDecl; }
};

class ImportDecl : public Decl {
public:
    std::string target;
    SrcLoc srcLoc;

    ImportDecl(std::string&& target, SrcLoc srcLoc)
    : Decl(DeclKind::ImportDecl, nullptr), target(std::move(target)), srcLoc(srcLoc) { }
    static bool classof(const Decl* d) { return d->getKind() == DeclKind::ImportDecl; }
};

}
