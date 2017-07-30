#pragma once

#include <cassert>
#include <string>
#include <memory>
#include <vector>
#include <llvm/ADT/SmallVector.h>
#include <llvm/Support/Casting.h>
#include "expr.h"
#include "stmt.h"
#include "type.h"
#include "location.h"

namespace llvm { class StringRef; }

namespace delta {

class Module;

enum class DeclKind {
    ParamDecl,
    FunctionDecl,
    GenericParamDecl,
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
    DEFINE_DECL_IS_AND_GET(FunctionDecl)
    DEFINE_DECL_IS_AND_GET(GenericParamDecl)
    DEFINE_DECL_IS_AND_GET(InitDecl)
    DEFINE_DECL_IS_AND_GET(DeinitDecl)
    DEFINE_DECL_IS_AND_GET(TypeDecl)
    DEFINE_DECL_IS_AND_GET(VarDecl)
    DEFINE_DECL_IS_AND_GET(FieldDecl)
    DEFINE_DECL_IS_AND_GET(ImportDecl)
#undef DEFINE_DECL_IS_AND_GET

    DeclKind getKind() const { return kind; }
    Module* getModule() const { return module; }
    SourceLocation getLocation() const;

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
    SourceLocation location;

    ParamDecl(Type type, std::string&& name, SourceLocation location)
    : Decl(DeclKind::ParamDecl, nullptr), type(type), name(std::move(name)), location(location) { }

    Type getType() const { return type; }

    static bool classof(const Decl* d) { return d->getKind() == DeclKind::ParamDecl; }
    bool operator==(const ParamDecl& other) const {
        return type == other.type && name == other.name;
    }
};

class GenericParamDecl : public Decl {
public:
    std::string name;
    SourceLocation location;
    llvm::SmallVector<std::string, 1> constraints;

    GenericParamDecl(std::string&& name, SourceLocation location)
    : Decl(DeclKind::GenericParamDecl, nullptr), name(std::move(name)), location(location) { }
    static bool classof(const Decl* d) { return d->getKind() == DeclKind::GenericParamDecl; }
};

class FunctionDecl : public Decl {
public:
    std::string name;
    std::vector<ParamDecl> params;
    Type returnType;
    bool mutating;
    std::vector<GenericParamDecl> genericParams;
    std::shared_ptr<std::vector<std::unique_ptr<Stmt>>> body;
    SourceLocation location;

    FunctionDecl(std::string&& name, std::vector<ParamDecl>&& params, Type returnType,
                 TypeDecl* receiverTypeDecl, std::vector<GenericParamDecl>&& genericParams,
                 bool isVarArg, Module* module, SourceLocation location)
    : Decl(DeclKind::FunctionDecl, module), name(std::move(name)), params(std::move(params)),
      returnType(returnType), mutating(false), genericParams(std::move(genericParams)),
      location(location), receiverTypeDecl(receiverTypeDecl), isVarArg(isVarArg) { }

    bool isExtern() const { return body == nullptr; }
    bool isVariadic() const { return isVarArg; }
    bool isGeneric() const { return !genericParams.empty(); }
    bool isMethod() const { return receiverTypeDecl != nullptr; }
    bool isMutating() const { return mutating; }
    void setMutating(bool m) { mutating = m; }
    llvm::ArrayRef<ParamDecl> getParams() const { return params; }
    llvm::ArrayRef<GenericParamDecl> getGenericParams() const { return genericParams; }
    TypeDecl* getReceiverTypeDecl() const { return receiverTypeDecl; }
    const FunctionType* getFunctionType() const;
    bool signatureMatches(const FunctionDecl& other, bool matchReceiver = true) const;
    static bool classof(const Decl* d) { return d->getKind() == DeclKind::FunctionDecl; }

private:
    TypeDecl* receiverTypeDecl;
    bool isVarArg;
};

class InitDecl : public Decl {
public:
    std::string typeName;
    TypeDecl* typeDecl;
    std::vector<ParamDecl> params;
    std::shared_ptr<std::vector<std::unique_ptr<Stmt>>> body;
    SourceLocation location;

    InitDecl(std::string&& typeName, std::vector<ParamDecl>&& params,
             std::shared_ptr<std::vector<std::unique_ptr<Stmt>>>&& body, SourceLocation location)
    : Decl(DeclKind::InitDecl, nullptr), typeName(std::move(typeName)),
      params(std::move(params)), body(std::move(body)), location(location) { }

    TypeDecl& getTypeDecl() const { assert(typeDecl); return *typeDecl; }
    llvm::ArrayRef<ParamDecl> getParams() const { return params; }
    llvm::StringRef getTypeName() const { return typeName; }

    static bool classof(const Decl* d) { return d->getKind() == DeclKind::InitDecl; }
};

class DeinitDecl : public Decl {
public:
    std::string typeName;
    TypeDecl* typeDecl;
    std::shared_ptr<std::vector<std::unique_ptr<Stmt>>> body;
    SourceLocation location;

    DeinitDecl(std::string&& typeName,
               std::shared_ptr<std::vector<std::unique_ptr<Stmt>>>&& body, SourceLocation location)
    : Decl(DeclKind::DeinitDecl, nullptr), typeName(std::move(typeName)),
      body(std::move(body)), location(location) { }

    TypeDecl& getTypeDecl() const { assert(typeDecl); return *typeDecl; }
    llvm::StringRef getTypeName() const { return typeName; }

    static bool classof(const Decl* d) { return d->getKind() == DeclKind::DeinitDecl; }
};

enum class TypeTag { Struct, Class, Interface, Union };

class TypeDecl : public Decl {
public:
    TypeTag tag;
    std::string name;
    std::vector<FieldDecl> fields;
    std::vector<std::unique_ptr<Decl>> methods; ///< FunctionDecls, InitDecls, and DeinitDecls
    std::vector<GenericParamDecl> genericParams;
    SourceLocation location;

    TypeDecl(TypeTag tag, std::string&& name, std::vector<GenericParamDecl>&& genericParams,
             Module* module, SourceLocation location)
    : Decl(DeclKind::TypeDecl, module), tag(tag), name(std::move(name)),
      genericParams(std::move(genericParams)), location(location) { }

    void addField(FieldDecl&& field);
    void addMethod(std::unique_ptr<Decl> decl);
    llvm::ArrayRef<std::unique_ptr<Decl>> getMemberDecls() const { return methods; }
    DeinitDecl* getDeinitializer() const;
    Type getType(llvm::ArrayRef<Type> genericArgs, bool isMutable = false) const;
    /// 'T&' if this is class, or plain 'T' otherwise.
    Type getTypeForPassing(llvm::ArrayRef<Type> genericArgs, bool isMutable = false) const;
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
    SourceLocation location;

    VarDecl(Type type, std::string&& name, std::shared_ptr<Expr>&& initializer,
            Module* module, SourceLocation location)
    : Decl(DeclKind::VarDecl, module), type(type), name(std::move(name)),
      initializer(std::move(initializer)), location(location) { }
    Type getType() const { return type; }
    static bool classof(const Decl* d) { return d->getKind() == DeclKind::VarDecl; }
};

class FieldDecl : public Decl {
public:
    Type type;
    std::string name;
    SourceLocation location;

    FieldDecl(Type type, std::string&& name, SourceLocation location)
    : Decl(DeclKind::FieldDecl, nullptr), type(type), name(std::move(name)), location(location) { }
    static bool classof(const Decl* d) { return d->getKind() == DeclKind::FieldDecl; }
};

class ImportDecl : public Decl {
public:
    std::string target;
    SourceLocation location;

    ImportDecl(std::string&& target, SourceLocation location)
    : Decl(DeclKind::ImportDecl, nullptr), target(std::move(target)), location(location) { }
    static bool classof(const Decl* d) { return d->getKind() == DeclKind::ImportDecl; }
};

}
