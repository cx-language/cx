#pragma once

#include <string>
#include <memory>
#include <vector>
#include <llvm/ADT/SmallVector.h>
#include <llvm/Support/Casting.h>
#include "expr.h"
#include "stmt.h"
#include "type.h"
#include "location.h"
#include "../support/utility.h"

namespace llvm { class StringRef; }

namespace delta {

class Module;
class TypeDecl;
class FieldDecl;
class FunctionLikeDecl;

enum class DeclKind {
    ParamDecl,
    GenericParamDecl,
    FunctionDecl,
    MethodDecl,
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

    bool isParamDecl() const { return getKind() == DeclKind::ParamDecl; }
    bool isFunctionLikeDecl() const { return getKind() >= DeclKind::FunctionDecl && getKind() <= DeclKind::DeinitDecl; }
    bool isFunctionDecl() const { return getKind() >= DeclKind::FunctionDecl && getKind() <= DeclKind::MethodDecl; }
    bool isMethodDecl() const { return getKind() == DeclKind::MethodDecl; }
    bool isGenericParamDecl() const { return getKind() == DeclKind::GenericParamDecl; }
    bool isInitDecl() const { return getKind() == DeclKind::InitDecl; }
    bool isDeinitDecl() const { return getKind() == DeclKind::DeinitDecl; }
    bool isTypeDecl() const { return getKind() == DeclKind::TypeDecl; }
    bool isVarDecl() const { return getKind() == DeclKind::VarDecl; }
    bool isFieldDecl() const { return getKind() == DeclKind::FieldDecl; }
    bool isImportDecl() const { return getKind() == DeclKind::ImportDecl; }

    DeclKind getKind() const { return kind; }
    Module* getModule() const;
    SourceLocation getLocation() const;

protected:
    Decl(DeclKind kind) : kind(kind) {}

private:
    const DeclKind kind;
};

inline Decl::~Decl() {}

class ParamDecl : public Decl {
public:
    Type type;
    std::string name;
    SourceLocation location;

    ParamDecl(Type type, std::string&& name, SourceLocation location)
    : Decl(DeclKind::ParamDecl), type(type), name(std::move(name)), location(location),
      parent(nullptr) {}
    Type getType() const { return type; }
    FunctionLikeDecl* getParent() const { ASSERT(parent); return parent; }
    void setParent(FunctionLikeDecl* parent) { this->parent = parent; }
    static bool classof(const Decl* d) { return d->getKind() == DeclKind::ParamDecl; }
    bool operator==(const ParamDecl& other) const {
        return type == other.type && name == other.name;
    }

private:
    FunctionLikeDecl* parent;
};

class GenericParamDecl : public Decl {
public:
    std::string name;
    SourceLocation location;
    llvm::SmallVector<std::string, 1> constraints;

    GenericParamDecl(std::string&& name, SourceLocation location)
    : Decl(DeclKind::GenericParamDecl), name(std::move(name)), location(location),
      parent(nullptr) {}
    Decl* getParent() const { ASSERT(parent); return parent; }
    void setParent(Decl* parent) { this->parent = parent; }
    static bool classof(const Decl* d) { return d->getKind() == DeclKind::GenericParamDecl; }

private:
    Decl* parent;
};

class FunctionProto {
public:
    FunctionProto(std::string&& name, std::vector<ParamDecl>&& params, Type returnType,
                  std::vector<GenericParamDecl>&& genericParams, bool isVarArg)
    : name(std::move(name)), params(std::move(params)), returnType(returnType),
      genericParams(std::move(genericParams)), isVarArg(isVarArg) {}

    std::string name;
    std::vector<ParamDecl> params;
    Type returnType;
    std::vector<GenericParamDecl> genericParams;
    bool isVarArg;
};

class FunctionLikeDecl : public Decl {
public:
    bool isExtern() const { return !getBody(); }
    bool isVariadic() const { return getProto().isVarArg; }
    bool isGeneric() const { return !getProto().genericParams.empty(); }
    llvm::StringRef getName() const { return getProto().name; }
    Type getReturnType() const { return getProto().returnType; }
    llvm::ArrayRef<ParamDecl> getParams() const { return getProto().params; }
    llvm::MutableArrayRef<ParamDecl> getParams() { return getProto().params; }
    llvm::ArrayRef<GenericParamDecl> getGenericParams() const { return getProto().genericParams; }
    const FunctionProto& getProto() const { return proto; }
    FunctionProto& getProto() { return proto; }
    virtual TypeDecl* getTypeDecl() const { return nullptr; }
    virtual bool isMutating() const { return false; }
    std::vector<std::unique_ptr<Stmt>>* getBody() const { return body.get(); }
    const FunctionType* getFunctionType() const;
    static bool classof(const Decl* d) { return d->isFunctionLikeDecl(); }

protected:
    FunctionLikeDecl(DeclKind kind, FunctionProto&& proto, SourceLocation location,
                     std::shared_ptr<std::vector<std::unique_ptr<Stmt>>>&& body = nullptr)
    : Decl(kind), proto(std::move(proto)), body(std::move(body)), location(location) {}

private:
    FunctionProto proto;

public:
    std::shared_ptr<std::vector<std::unique_ptr<Stmt>>> body;
    SourceLocation location;
};

class FunctionDecl : public FunctionLikeDecl {
public:
    FunctionDecl(FunctionProto&& proto, Module& module, SourceLocation location)
    : FunctionDecl(DeclKind::FunctionDecl, std::move(proto), module, location) {}
    bool signatureMatches(const FunctionDecl& other, bool matchReceiver = true) const;
    Module* getModule() const { return &module; }
    static bool classof(const Decl* d) { return d->isFunctionDecl(); }

protected:
    FunctionDecl(DeclKind kind, FunctionProto&& proto, Module& module, SourceLocation location)
    : FunctionLikeDecl(kind, std::move(proto), location), module(module) {}

private:
    Module& module;
};

class MethodDecl : public FunctionDecl {
public:
    MethodDecl(FunctionProto proto, TypeDecl& receiverTypeDecl, SourceLocation location)
    : MethodDecl(DeclKind::MethodDecl, std::move(proto), receiverTypeDecl, location) {}

    bool isMutating() const override { return mutating; }
    void setMutating(bool mutating) { this->mutating = mutating; }
    TypeDecl* getTypeDecl() const override { return typeDecl; }
    static bool classof(const Decl* d) { return d->isMethodDecl(); }

protected:
    MethodDecl(DeclKind kind, FunctionProto proto, TypeDecl& typeDecl, SourceLocation location);

private:
    TypeDecl* typeDecl;
    bool mutating;
};

class InitDecl : public FunctionLikeDecl {
public:
    InitDecl(TypeDecl& receiverTypeDecl, std::vector<ParamDecl>&& params,
             std::shared_ptr<std::vector<std::unique_ptr<Stmt>>>&& body, SourceLocation location)
    : FunctionLikeDecl(DeclKind::InitDecl, FunctionProto("init", std::move(params), Type::getVoid(), {}, false),
                       location, std::move(body)), typeDecl(&receiverTypeDecl) {}
    TypeDecl* getTypeDecl() const override { return typeDecl; }
    static bool classof(const Decl* d) { return d->getKind() == DeclKind::InitDecl; }

private:
    TypeDecl* typeDecl;
};

class DeinitDecl : public FunctionLikeDecl {
public:
    DeinitDecl(TypeDecl& receiverTypeDecl, std::shared_ptr<std::vector<std::unique_ptr<Stmt>>>&& body,
               SourceLocation location)
    : FunctionLikeDecl(DeclKind::DeinitDecl, FunctionProto("deinit", {}, Type::getVoid(), {}, false),
                       location, std::move(body)), typeDecl(&receiverTypeDecl) {}
    TypeDecl* getTypeDecl() const override { return typeDecl; }
    static bool classof(const Decl* d) { return d->getKind() == DeclKind::DeinitDecl; }

private:
    TypeDecl* typeDecl;
};

enum class TypeTag { Struct, Class, Interface, Union };

class TypeDecl : public Decl {
public:
    TypeTag tag;
    std::string name;
    std::vector<FieldDecl> fields;
    std::vector<std::unique_ptr<FunctionLikeDecl>> methods; ///< MethodDecls, InitDecls, and DeinitDecls
    std::vector<GenericParamDecl> genericParams;
    SourceLocation location;

    TypeDecl(TypeTag tag, std::string&& name, std::vector<GenericParamDecl>&& genericParams,
             Module& module, SourceLocation location)
    : Decl(DeclKind::TypeDecl), tag(tag), name(std::move(name)),
      genericParams(std::move(genericParams)), location(location), module(module) {}

    void addField(FieldDecl&& field);
    void addMethod(std::unique_ptr<FunctionLikeDecl> decl);
    llvm::ArrayRef<std::unique_ptr<FunctionLikeDecl>> getMemberDecls() const { return methods; }
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
    Module* getModule() const { return &module; }
    static bool classof(const Decl* d) { return d->getKind() == DeclKind::TypeDecl; }

private:
    Module& module;
};

class VarDecl : public Decl {
public:
    Type type;
    std::string name;
    std::shared_ptr<Expr> initializer; /// Null if the initializer is 'uninitialized'.
    SourceLocation location;

    VarDecl(Type type, std::string&& name, std::shared_ptr<Expr>&& initializer,
            Module& module, SourceLocation location)
    : Decl(DeclKind::VarDecl), type(type), name(std::move(name)),
      initializer(std::move(initializer)), location(location), module(module) {}
    Type getType() const { return type; }
    Module* getModule() const { return &module; }
    static bool classof(const Decl* d) { return d->getKind() == DeclKind::VarDecl; }

private:
    Module& module;
};

class FieldDecl : public Decl {
public:
    Type type;
    std::string name;
    SourceLocation location;

    FieldDecl(Type type, std::string&& name, TypeDecl& parent, SourceLocation location)
    : Decl(DeclKind::FieldDecl), type(type), name(std::move(name)), location(location),
      parent(parent) {}
    TypeDecl* getParent() const { return &parent; }
    static bool classof(const Decl* d) { return d->getKind() == DeclKind::FieldDecl; }

private:
    TypeDecl& parent;
};

class ImportDecl : public Decl {
public:
    std::string target;
    SourceLocation location;

    ImportDecl(std::string&& target, Module& module, SourceLocation location)
    : Decl(DeclKind::ImportDecl), target(std::move(target)), location(location), module(module) {}
    Module* getModule() const { return &module; }
    static bool classof(const Decl* d) { return d->getKind() == DeclKind::ImportDecl; }

private:
    Module& module;
};

}
