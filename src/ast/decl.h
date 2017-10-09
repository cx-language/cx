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
    bool hasBeenMoved() const;
    void markAsMoved();

protected:
    Decl(DeclKind kind) : kind(kind) {}

private:
    const DeclKind kind;
};

inline Decl::~Decl() {}

class Movable {
public:
    bool isMoved() const { return moved; }
    void setMoved(bool moved) { this->moved = moved; }

private:
    bool moved = false;
};

class ParamDecl : public Decl, public Movable {
public:
    ParamDecl(Type type, std::string&& name, SourceLocation location)
    : Decl(DeclKind::ParamDecl), type(type), name(std::move(name)), location(location),
      parent(nullptr) {}
    Type getType() const { return type; }
    llvm::StringRef getName() const { return name; }
    FunctionLikeDecl* getParent() const { ASSERT(parent); return parent; }
    void setParent(FunctionLikeDecl* parent) { this->parent = parent; }
    SourceLocation getLocation() const { return location; }
    static bool classof(const Decl* d) { return d->getKind() == DeclKind::ParamDecl; }
    bool operator==(const ParamDecl& other) const {
        return getType() == other.getType() && getName() == other.getName();
    }

private:
    Type type;
    std::string name;
    SourceLocation location;
    FunctionLikeDecl* parent;
};

class GenericParamDecl : public Decl {
public:
    GenericParamDecl(std::string&& name, SourceLocation location)
    : Decl(DeclKind::GenericParamDecl), name(std::move(name)), parent(nullptr), location(location) {}
    llvm::StringRef getName() const { return name; }
    llvm::ArrayRef<std::string> getConstraints() const { return constraints; }
    void addConstraint(std::string&& constraint) { constraints.emplace_back(std::move(constraint)); }
    Decl* getParent() const { ASSERT(parent); return parent; }
    void setParent(Decl* parent) { this->parent = parent; }
    SourceLocation getLocation() const { return location; }
    static bool classof(const Decl* d) { return d->getKind() == DeclKind::GenericParamDecl; }

private:
    std::string name;
    llvm::SmallVector<std::string, 1> constraints;
    Decl* parent;
    SourceLocation location;
};

class FunctionProto {
public:
    FunctionProto(std::string&& name, std::vector<ParamDecl>&& params, Type returnType,
                  std::vector<GenericParamDecl>&& genericParams, bool isVarArg)
    : name(std::move(name)), params(std::move(params)), returnType(returnType),
      genericParams(std::move(genericParams)), varArg(isVarArg) {}
    llvm::StringRef getName() const { return name; }
    llvm::ArrayRef<ParamDecl> getParams() const { return params; }
    llvm::MutableArrayRef<ParamDecl> getParams() { return params; }
    Type getReturnType() const { return returnType; }
    llvm::ArrayRef<GenericParamDecl> getGenericParams() const { return genericParams; }
    bool isVarArg() const { return varArg; }

private:
    std::string name;
    std::vector<ParamDecl> params;
    Type returnType;
    std::vector<GenericParamDecl> genericParams;
    bool varArg;
};

class FunctionLikeDecl : public Decl {
public:
    bool isExtern() const { return !getBody(); }
    bool isVariadic() const { return getProto().isVarArg(); }
    bool isGeneric() const { return !getProto().getGenericParams().empty(); }
    llvm::StringRef getName() const { return getProto().getName(); }
    Type getReturnType() const { return getProto().getReturnType(); }
    llvm::ArrayRef<ParamDecl> getParams() const { return getProto().getParams(); }
    llvm::MutableArrayRef<ParamDecl> getParams() { return getProto().getParams(); }
    llvm::ArrayRef<GenericParamDecl> getGenericParams() const { return getProto().getGenericParams(); }
    const FunctionProto& getProto() const { return proto; }
    FunctionProto& getProto() { return proto; }
    virtual TypeDecl* getTypeDecl() const { return nullptr; }
    virtual bool isMutating() const { return false; }
    std::vector<std::unique_ptr<Stmt>>* getBody() const { return body.get(); }
    void setBody(std::shared_ptr<std::vector<std::unique_ptr<Stmt>>>&& body) { this->body = body; }
    SourceLocation getLocation() const { return location; }
    const FunctionType* getFunctionType() const;
    static bool classof(const Decl* d) { return d->isFunctionLikeDecl(); }

protected:
    FunctionLikeDecl(DeclKind kind, FunctionProto&& proto, SourceLocation location,
                     std::shared_ptr<std::vector<std::unique_ptr<Stmt>>>&& body = nullptr)
    : Decl(kind), proto(std::move(proto)), body(std::move(body)), location(location) {}

private:
    FunctionProto proto;
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
    Type getThisType() const;
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
    TypeDecl(TypeTag tag, std::string&& name, std::vector<GenericParamDecl>&& genericParams,
             Module& module, SourceLocation location)
    : Decl(DeclKind::TypeDecl), tag(tag), name(std::move(name)),
      genericParams(std::move(genericParams)), location(location), module(module) {}
    TypeTag getTag() const { return tag; }
    llvm::StringRef getName() const { return name; }
    llvm::ArrayRef<FieldDecl> getFields() const { return fields; }
    std::vector<FieldDecl>& getFields() { return fields; }
    llvm::ArrayRef<std::unique_ptr<FunctionLikeDecl>> getMethods() const { return methods; }
    llvm::ArrayRef<GenericParamDecl> getGenericParams() const { return genericParams; }
    SourceLocation getLocation() const { return location; }
    void addField(FieldDecl&& field);
    void addMethod(std::unique_ptr<FunctionLikeDecl> decl);
    llvm::ArrayRef<std::unique_ptr<FunctionLikeDecl>> getMemberDecls() const { return methods; }
    DeinitDecl* getDeinitializer() const;
    Type getType(llvm::ArrayRef<Type> genericArgs, bool isMutable = false) const;
    Type getUnresolvedType(bool isMutable) const;
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
    TypeTag tag;
    std::string name;
    std::vector<FieldDecl> fields;
    std::vector<std::unique_ptr<FunctionLikeDecl>> methods; ///< MethodDecls, InitDecls, and DeinitDecls
    std::vector<GenericParamDecl> genericParams;
    SourceLocation location;
    Module& module;
};

class VarDecl : public Decl, public Movable {
public:
    VarDecl(Type type, std::string&& name, std::shared_ptr<Expr>&& initializer,
            Module& module, SourceLocation location)
    : Decl(DeclKind::VarDecl), type(type), name(std::move(name)),
      initializer(std::move(initializer)), location(location), module(module) {}
    Type getType() const { return type; }
    void setType(Type type) { this->type = type; }
    llvm::StringRef getName() const { return name; }
    Expr* getInitializer() const { return initializer.get(); }
    SourceLocation getLocation() const { return location; }
    Module* getModule() const { return &module; }
    static bool classof(const Decl* d) { return d->getKind() == DeclKind::VarDecl; }

private:
    Type type;
    std::string name;
    std::shared_ptr<Expr> initializer; /// Null if the initializer is 'uninitialized'.
    SourceLocation location;
    Module& module;
};

class FieldDecl : public Decl {
public:
    FieldDecl(Type type, std::string&& name, TypeDecl& parent, SourceLocation location)
    : Decl(DeclKind::FieldDecl), type(type), name(std::move(name)), location(location),
      parent(parent) {}
    Type getType() const { return type; }
    llvm::StringRef getName() const { return name; }
    SourceLocation getLocation() const { return location; }
    TypeDecl* getParent() const { return &parent; }
    static bool classof(const Decl* d) { return d->getKind() == DeclKind::FieldDecl; }

private:
    Type type;
    std::string name;
    SourceLocation location;
    TypeDecl& parent;
};

class ImportDecl : public Decl {
public:
    ImportDecl(std::string&& target, Module& module, SourceLocation location)
    : Decl(DeclKind::ImportDecl), target(std::move(target)), location(location), module(module) {}
    llvm::StringRef getTarget() const { return target; }
    SourceLocation getLocation() const { return location; }
    Module* getModule() const { return &module; }
    static bool classof(const Decl* d) { return d->getKind() == DeclKind::ImportDecl; }

private:
    std::string target;
    SourceLocation location;
    Module& module;
};

}
