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
class TypeDecl;
class FieldDecl;

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
    Module* getModule() const { return module; }
    SourceLocation getLocation() const;

protected:
    Decl(DeclKind kind, Module* module) : kind(kind), module(module) {}

private:
    const DeclKind kind;
    Module* module;
};

inline Decl::~Decl() {}

class ParamDecl : public Decl {
public:
    Type type;
    std::string name;
    SourceLocation location;

    ParamDecl(Type type, std::string&& name, SourceLocation location)
    : Decl(DeclKind::ParamDecl, nullptr), type(type), name(std::move(name)), location(location) {}

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
    : Decl(DeclKind::GenericParamDecl, nullptr), name(std::move(name)), location(location) {}
    static bool classof(const Decl* d) { return d->getKind() == DeclKind::GenericParamDecl; }
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
    FunctionLikeDecl(DeclKind kind, FunctionProto&& proto, Module* module, SourceLocation location,
                     std::shared_ptr<std::vector<std::unique_ptr<Stmt>>>&& body = nullptr)
    : Decl(kind, module), proto(std::move(proto)), body(std::move(body)), location(location) {}

private:
    FunctionProto proto;

public:
    std::shared_ptr<std::vector<std::unique_ptr<Stmt>>> body;
    SourceLocation location;
};

class FunctionDecl : public FunctionLikeDecl {
public:
    FunctionDecl(FunctionProto&& proto, Module* module, SourceLocation location)
    : FunctionDecl(DeclKind::FunctionDecl, std::move(proto), module, location) {}
    bool signatureMatches(const FunctionDecl& other, bool matchReceiver = true) const;
    static bool classof(const Decl* d) { return d->isFunctionDecl(); }

protected:
    FunctionDecl(DeclKind kind, FunctionProto&& proto, Module* module, SourceLocation location)
    : FunctionLikeDecl(kind, std::move(proto), module, location) {}
};

class MethodDecl : public FunctionDecl {
public:
    MethodDecl(FunctionProto proto, TypeDecl& receiverTypeDecl, Module* module, SourceLocation location)
    : MethodDecl(DeclKind::MethodDecl, std::move(proto), receiverTypeDecl, module, location) {}

    bool isMutating() const override { return mutating; }
    void setMutating(bool mutating) { this->mutating = mutating; }
    TypeDecl* getTypeDecl() const override { return typeDecl; }
    static bool classof(const Decl* d) { return d->isMethodDecl(); }

protected:
    MethodDecl(DeclKind kind, FunctionProto proto, TypeDecl& typeDecl, Module* module, SourceLocation location)
    : FunctionDecl(kind, std::move(proto), module, location), typeDecl(&typeDecl), mutating(false) {}

private:
    TypeDecl* typeDecl;
    bool mutating;
};

class InitDecl : public FunctionLikeDecl {
public:
    InitDecl(TypeDecl& receiverTypeDecl, std::vector<ParamDecl>&& params,
             std::shared_ptr<std::vector<std::unique_ptr<Stmt>>>&& body, SourceLocation location)
    : FunctionLikeDecl(DeclKind::InitDecl, FunctionProto("init", std::move(params), Type::getVoid(), {}, false),
                       nullptr, location, std::move(body)), typeDecl(&receiverTypeDecl) {}
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
                       nullptr, location, std::move(body)), typeDecl(&receiverTypeDecl) {}
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
             Module* module, SourceLocation location)
    : Decl(DeclKind::TypeDecl, module), tag(tag), name(std::move(name)),
      genericParams(std::move(genericParams)), location(location) {}

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
      initializer(std::move(initializer)), location(location) {}
    Type getType() const { return type; }
    static bool classof(const Decl* d) { return d->getKind() == DeclKind::VarDecl; }
};

class FieldDecl : public Decl {
public:
    Type type;
    std::string name;
    SourceLocation location;

    FieldDecl(Type type, std::string&& name, SourceLocation location)
    : Decl(DeclKind::FieldDecl, nullptr), type(type), name(std::move(name)), location(location) {}
    static bool classof(const Decl* d) { return d->getKind() == DeclKind::FieldDecl; }
};

class ImportDecl : public Decl {
public:
    std::string target;
    SourceLocation location;

    ImportDecl(std::string&& target, SourceLocation location)
    : Decl(DeclKind::ImportDecl, nullptr), target(std::move(target)), location(location) {}
    static bool classof(const Decl* d) { return d->getKind() == DeclKind::ImportDecl; }
};

}
