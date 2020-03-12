#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#pragma warning(push, 0)
#include <llvm/ADT/Optional.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/Support/Casting.h>
#pragma warning(pop)
#include "expr.h"
#include "location.h"
#include "stmt.h"
#include "type.h"
#include "../support/utility.h"

namespace std {
template<>
struct hash<std::vector<delta::Type>> {
    size_t operator()(llvm::ArrayRef<delta::Type> types) const {
        ASSERT(!types.empty());
        size_t hashValue = reinterpret_cast<size_t>(types[0].getBase());

        for (auto type : types.drop_front()) {
            hashValue ^= reinterpret_cast<size_t>(type.getBase());
        }

        return hashValue;
    }
};
} // namespace std

namespace llvm {
class StringRef;
}

namespace delta {

class Module;
class TypeDecl;
class FieldDecl;
class FunctionDecl;
class EnumDecl;

enum class DeclKind {
    GenericParamDecl,
    FunctionDecl,
    MethodDecl,
    InitDecl,
    DeinitDecl,
    FunctionTemplate,
    TypeDecl,
    TypeTemplate,
    EnumDecl,
    EnumCase,
    VarDecl,
    FieldDecl,
    ParamDecl,
    ImportDecl,
};

enum class AccessLevel {
    None,
    Private,
    Default,
};

inline llvm::raw_ostream& operator<<(llvm::raw_ostream& stream, AccessLevel accessLevel) {
    switch (accessLevel) {
        case AccessLevel::None:
            llvm_unreachable("invalid access level");
        case AccessLevel::Private:
            return stream << "private";
        case AccessLevel::Default:
            // TODO: Rename to "internal" when public access level is added.
            return stream << "public";
    }
    llvm_unreachable("all cases handled");
}

class Decl {
public:
    virtual ~Decl() = 0;

    bool isVariableDecl() const { return getKind() >= DeclKind::VarDecl && getKind() <= DeclKind::ParamDecl; }
    bool isParamDecl() const { return getKind() == DeclKind::ParamDecl; }
    bool isFunctionDecl() const { return getKind() >= DeclKind::FunctionDecl && getKind() <= DeclKind::DeinitDecl; }
    bool isMethodDecl() const { return getKind() >= DeclKind::MethodDecl && getKind() <= DeclKind::DeinitDecl; }
    bool isGenericParamDecl() const { return getKind() == DeclKind::GenericParamDecl; }
    bool isInitDecl() const { return getKind() == DeclKind::InitDecl; }
    bool isDeinitDecl() const { return getKind() == DeclKind::DeinitDecl; }
    bool isFunctionTemplate() const { return getKind() == DeclKind::FunctionTemplate; }
    bool isTypeDecl() const { return getKind() == DeclKind::TypeDecl || getKind() == DeclKind::EnumDecl; }
    bool isTypeTemplate() const { return getKind() == DeclKind::TypeTemplate; }
    bool isEnumDecl() const { return getKind() == DeclKind::EnumDecl; }
    bool isEnumCaseDecl() const { return getKind() == DeclKind::EnumCase; }
    bool isVarDecl() const { return getKind() == DeclKind::VarDecl; }
    bool isFieldDecl() const { return getKind() == DeclKind::FieldDecl; }
    bool isImportDecl() const { return getKind() == DeclKind::ImportDecl; }

    DeclKind getKind() const { return kind; }
    virtual Module* getModule() const = 0;
    virtual SourceLocation getLocation() const = 0;
    virtual llvm::StringRef getName() const = 0;
    bool isMain() const { return getName() == "main"; }
    bool isLambda() const { return isFunctionDecl() && getName().startswith("__lambda"); }
    AccessLevel getAccessLevel() const { return accessLevel; }
    virtual bool isReferenced() const { return referenced; }
    void setReferenced(bool referenced) { this->referenced = referenced; }
    bool hasBeenMoved() const;
    Decl* instantiate(const llvm::StringMap<Type>& genericArgs, llvm::ArrayRef<Type> genericArgsArray) const;

protected:
    Decl(DeclKind kind, AccessLevel accessLevel) : kind(kind), accessLevel(accessLevel), referenced(false) {}

private:
    const DeclKind kind;
    AccessLevel accessLevel;
    bool referenced;
};

inline Decl::~Decl() {}

class Movable {
public:
    bool isMoved() const { return moved; }
    void setMoved(bool moved) { this->moved = moved; }

private:
    bool moved = false;
};

/// Represents any variable declaration, including local variables, global variables, member variables, and parameters.
class VariableDecl : public Decl {
public:
    Type getType() const { return type; }
    void setType(Type type) { this->type = NOTNULL(type); }
    Decl* getParent() const { return parent; }
    void setParent(Decl* p) { parent = p; }
    static bool classof(const Decl* d) { return d->isVariableDecl(); }

protected:
    VariableDecl(DeclKind kind, AccessLevel accessLevel, Decl* parent, Type type) : Decl(kind, accessLevel), parent(parent), type(type) {}

private:
    Decl* parent;
    Type type;
};

class ParamDecl : public VariableDecl, public Movable {
public:
    ParamDecl(Type type, std::string&& name, bool isNamedArgument, SourceLocation location)
    : VariableDecl(DeclKind::ParamDecl, AccessLevel::None, nullptr /* initialized by FunctionDecl constructor */, type),
      name(std::move(name)), location(location), namedArgument(isNamedArgument) {}
    llvm::StringRef getName() const override { return name; }
    bool isNamedArgument() const { return namedArgument; }
    Module* getModule() const override { return nullptr; }
    SourceLocation getLocation() const override { return location; }
    static bool classof(const Decl* d) { return d->getKind() == DeclKind::ParamDecl; }
    bool operator==(const ParamDecl& other) const { return getType() == other.getType() && getName() == other.getName(); }

private:
    std::string name;
    SourceLocation location;
    bool namedArgument;
};

std::vector<ParamDecl> instantiateParams(llvm::ArrayRef<ParamDecl> params, const llvm::StringMap<Type>& genericArgs);

class GenericParamDecl : public Decl {
public:
    GenericParamDecl(std::string&& name, SourceLocation location)
    : Decl(DeclKind::GenericParamDecl, AccessLevel::None), name(std::move(name)), location(location) {}
    llvm::StringRef getName() const override { return name; }
    llvm::ArrayRef<Type> getConstraints() const { return constraints; }
    void setConstraints(llvm::ArrayRef<Type> c) { constraints.assign(c.begin(), c.end()); }
    Module* getModule() const override { return nullptr; }
    SourceLocation getLocation() const override { return location; }
    static bool classof(const Decl* d) { return d->getKind() == DeclKind::GenericParamDecl; }

private:
    std::string name;
    llvm::SmallVector<Type, 1> constraints;
    SourceLocation location;
};

class FunctionProto {
public:
    FunctionProto(std::string&& name, std::vector<ParamDecl>&& params, Type returnType, bool isVarArg, bool isExtern)
    : name(std::move(name)), params(std::move(params)), returnType(returnType), varArg(isVarArg), external(isExtern) {}
    llvm::StringRef getName() const { return name; }
    llvm::ArrayRef<ParamDecl> getParams() const { return params; }
    llvm::MutableArrayRef<ParamDecl> getParams() { return params; }
    Type getReturnType() const { return NOTNULL(returnType); }
    void setReturnType(Type type) { returnType = type; }
    bool isVarArg() const { return varArg; }
    bool isExtern() const { return external; }
    FunctionProto instantiate(const llvm::StringMap<Type>& genericArgs) const;

private:
    std::string name;
    std::vector<ParamDecl> params;
    Type returnType;
    bool varArg;
    bool external;
};

std::string getQualifiedFunctionName(Type receiver, llvm::StringRef name, llvm::ArrayRef<Type> genericArgs);

class FunctionDecl : public Decl {
public:
    FunctionDecl(FunctionProto&& proto, std::vector<Type>&& genericArgs, AccessLevel accessLevel, Module& module, SourceLocation location)
    : FunctionDecl(DeclKind::FunctionDecl, std::move(proto), std::move(genericArgs), accessLevel, module, location) {
        for (auto& param : getParams()) {
            param.setParent(this);
        }
    }
    bool isExtern() const { return getProto().isExtern(); }
    bool isVariadic() const { return getProto().isVarArg(); }
    llvm::StringRef getName() const override { return getProto().getName(); }
    std::string getQualifiedName() const;
    llvm::ArrayRef<Type> getGenericArgs() const { return genericArgs; }
    Type getReturnType() const { return getProto().getReturnType(); }
    llvm::ArrayRef<ParamDecl> getParams() const { return getProto().getParams(); }
    llvm::MutableArrayRef<ParamDecl> getParams() { return getProto().getParams(); }
    const FunctionProto& getProto() const { return proto; }
    FunctionProto& getProto() { return proto; }
    virtual TypeDecl* getTypeDecl() const { return nullptr; }
    bool hasBody() const { return body.hasValue(); }
    llvm::ArrayRef<Stmt*> getBody() const { return *body; }
    llvm::MutableArrayRef<Stmt*> getBody() { return *body; }
    void setBody(std::vector<Stmt*>&& body) { this->body = std::move(body); }
    SourceLocation getLocation() const override { return location; }
    FunctionType* getFunctionType() const;
    bool signatureMatches(const FunctionDecl& other, bool matchReceiver = true) const;
    Module* getModule() const override { return &module; }
    FunctionDecl* instantiate(const llvm::StringMap<Type>& genericArgs, llvm::ArrayRef<Type> genericArgsArray);
    bool isTypechecked() const { return typechecked; }
    void setTypechecked(bool typechecked) { this->typechecked = typechecked; }
    static bool classof(const Decl* d) { return d->isFunctionDecl(); }

protected:
    FunctionDecl(DeclKind kind, FunctionProto&& proto, std::vector<Type>&& genericArgs, AccessLevel accessLevel, Module& module, SourceLocation location)
    : Decl(kind, accessLevel), proto(std::move(proto)), genericArgs(std::move(genericArgs)), location(location), module(module),
      typechecked(false) {}

private:
    FunctionProto proto;
    std::vector<Type> genericArgs;
    llvm::Optional<std::vector<Stmt*>> body;
    SourceLocation location;
    Module& module;
    bool typechecked;
};

class MethodDecl : public FunctionDecl {
public:
    MethodDecl(FunctionProto proto, TypeDecl& receiverTypeDecl, std::vector<Type>&& genericArgs, AccessLevel accessLevel, SourceLocation location)
    : MethodDecl(DeclKind::MethodDecl, std::move(proto), receiverTypeDecl, std::move(genericArgs), accessLevel, location) {}
    TypeDecl* getTypeDecl() const override { return typeDecl; }
    MethodDecl* instantiate(const llvm::StringMap<Type>& genericArgs, TypeDecl& typeDecl);
    static bool classof(const Decl* d) { return d->isMethodDecl(); }

protected:
    MethodDecl(DeclKind kind, FunctionProto proto, TypeDecl& typeDecl, std::vector<Type>&& genericArgs, AccessLevel accessLevel, SourceLocation location);

private:
    TypeDecl* typeDecl;
};

class InitDecl : public MethodDecl {
public:
    InitDecl(TypeDecl& receiverTypeDecl, std::vector<ParamDecl>&& params, AccessLevel accessLevel, SourceLocation location)
    : MethodDecl(DeclKind::InitDecl, FunctionProto("init", std::move(params), Type::getVoid(), false, false), receiverTypeDecl, {},
                 accessLevel, location) {}
    static bool classof(const Decl* d) { return d->getKind() == DeclKind::InitDecl; }
};

class DeinitDecl : public MethodDecl {
public:
    DeinitDecl(TypeDecl& receiverTypeDecl, SourceLocation location)
    : MethodDecl(DeclKind::DeinitDecl, FunctionProto("deinit", {}, Type::getVoid(), false, false), receiverTypeDecl, {}, AccessLevel::None, location) {}
    static bool classof(const Decl* d) { return d->getKind() == DeclKind::DeinitDecl; }
};

class FunctionTemplate : public Decl {
public:
    FunctionTemplate(std::vector<GenericParamDecl>&& genericParams, FunctionDecl* functionDecl, AccessLevel accessLevel)
    : Decl(DeclKind::FunctionTemplate, accessLevel), genericParams(std::move(genericParams)), functionDecl(functionDecl) {}
    llvm::StringRef getName() const override { return getFunctionDecl()->getName(); }
    std::string getQualifiedName() const { return getFunctionDecl()->getQualifiedName(); }
    bool isReferenced() const override;
    static bool classof(const Decl* d) { return d->isFunctionTemplate(); }
    llvm::ArrayRef<GenericParamDecl> getGenericParams() const { return genericParams; }
    FunctionDecl* getFunctionDecl() const { return functionDecl; }
    FunctionDecl* instantiate(const llvm::StringMap<Type>& genericArgs);
    Module* getModule() const override { return functionDecl->getModule(); }
    SourceLocation getLocation() const override { return functionDecl->getLocation(); }

private:
    std::vector<GenericParamDecl> genericParams;
    FunctionDecl* functionDecl;
    std::unordered_map<std::vector<Type>, FunctionDecl*> instantiations;
};

enum class TypeTag { Struct, Interface, Union, Enum };

/// A non-template function declaration or a function template instantiation.
class TypeDecl : public Decl {
public:
    TypeDecl(TypeTag tag, std::string&& name, std::vector<Type>&& genericArgs, std::vector<Type>&& interfaces, AccessLevel accessLevel,
             Module& module, SourceLocation location)
    : Decl(DeclKind::TypeDecl, accessLevel), tag(tag), name(std::move(name)), genericArgs(std::move(genericArgs)),
      interfaces(std::move(interfaces)), location(location), module(module) {}
    TypeTag getTag() const { return tag; }
    llvm::StringRef getName() const override { return name; }
    std::string getQualifiedName() const;
    llvm::ArrayRef<FieldDecl> getFields() const { return fields; }
    std::vector<FieldDecl>& getFields() { return fields; }
    llvm::ArrayRef<Decl*> getMethods() const { return methods; }
    llvm::ArrayRef<Type> getGenericArgs() const { return genericArgs; }
    llvm::ArrayRef<Type> getInterfaces() const { return interfaces; }
    bool hasInterface(const TypeDecl& interface) const;
    bool isCopyable() const;
    SourceLocation getLocation() const override { return location; }
    void addField(FieldDecl&& field);
    void addMethod(Decl* decl);
    llvm::ArrayRef<Decl*> getMemberDecls() const { return methods; }
    DeinitDecl* getDeinitializer() const;
    Type getType(Mutability mutability = Mutability::Mutable) const;
    Type getTypeForPassing() const;
    bool passByValue() const { return (isStruct() && isCopyable()) || isUnion(); }
    bool isStruct() const { return tag == TypeTag::Struct; }
    bool isInterface() const { return tag == TypeTag::Interface; }
    bool isUnion() const { return tag == TypeTag::Union; }
    unsigned getFieldIndex(llvm::StringRef fieldName) const;
    Module* getModule() const override { return &module; }
    static bool classof(const Decl* d) { return d->isTypeDecl(); }

protected:
    TypeDecl(DeclKind kind, TypeTag tag, std::string&& name, AccessLevel accessLevel, Module& module, SourceLocation location)
    : Decl(kind, accessLevel), tag(tag), name(std::move(name)), location(location), module(module) {}

private:
    TypeTag tag;
    std::string name;
    std::vector<Type> genericArgs;
    std::vector<Type> interfaces;
    std::vector<FieldDecl> fields;
    std::vector<Decl*> methods;
    SourceLocation location;
    Module& module;
};

class TypeTemplate : public Decl {
public:
    TypeTemplate(std::vector<GenericParamDecl>&& genericParams, TypeDecl* typeDecl, AccessLevel accessLevel)
    : Decl(DeclKind::TypeTemplate, accessLevel), genericParams(std::move(genericParams)), typeDecl(typeDecl) {}
    llvm::ArrayRef<GenericParamDecl> getGenericParams() const { return genericParams; }
    llvm::StringRef getName() const override { return getTypeDecl()->getName(); }
    TypeDecl* getTypeDecl() const { return typeDecl; }
    TypeDecl* instantiate(const llvm::StringMap<Type>& genericArgs);
    TypeDecl* instantiate(llvm::ArrayRef<Type> genericArgs);
    Module* getModule() const override { return typeDecl->getModule(); }
    SourceLocation getLocation() const override { return typeDecl->getLocation(); }
    static bool classof(const Decl* d) { return d->getKind() == DeclKind::TypeTemplate; }

private:
    std::vector<GenericParamDecl> genericParams;
    TypeDecl* typeDecl;
    std::unordered_map<std::vector<Type>, TypeDecl*> instantiations;
};

class EnumCase : public VariableDecl {
public:
    EnumCase(std::string&& name, Expr* value, Type associatedType, AccessLevel accessLevel, SourceLocation location);
    llvm::StringRef getName() const override { return name; }
    Expr* getValue() const { return value; }
    Expr*& getValue() { return value; }
    Type getAssociatedType() const { return associatedType; }
    EnumDecl* getEnumDecl() const { return llvm::cast<EnumDecl>(getParent()); }
    SourceLocation getLocation() const override { return location; }
    Module* getModule() const override { return getParent()->getModule(); }
    static bool classof(const Decl* d) { return d->getKind() == DeclKind::EnumCase; }

private:
    std::string name;
    Expr* value;
    Type associatedType;
    SourceLocation location;
};

class EnumDecl : public TypeDecl {
public:
    EnumDecl(std::string&& name, std::vector<EnumCase>&& cases, AccessLevel accessLevel, Module& module, SourceLocation location)
    : TypeDecl(DeclKind::EnumDecl, TypeTag::Enum, std::move(name), accessLevel, module, location), cases(std::move(cases)) {
        for (auto& enumCase : this->cases) {
            enumCase.setParent(this);
            enumCase.setType(getType());
        }
    }
    bool hasAssociatedValues() const;
    llvm::ArrayRef<EnumCase> getCases() const { return cases; }
    llvm::MutableArrayRef<EnumCase> getCases() { return cases; }
    EnumCase* getCaseByName(llvm::StringRef name);
    // TODO: Select tag type to be able to hold all enum values.
    Type getTagType() const { return Type::getInt(); }
    static bool classof(const Decl* d) { return d->getKind() == DeclKind::EnumDecl; }

private:
    std::vector<EnumCase> cases;
};

class VarDecl : public VariableDecl, public Movable {
public:
    VarDecl(Type type, std::string&& name, Expr* initializer, Decl* parent, AccessLevel accessLevel, Module& module, SourceLocation location)
    : VariableDecl(DeclKind::VarDecl, accessLevel, parent, type), name(std::move(name)), initializer(initializer), location(location),
      module(module) {}
    llvm::StringRef getName() const override { return name; }
    Expr* getInitializer() const { return initializer; }
    Expr*& getInitializer() { return initializer; }
    SourceLocation getLocation() const override { return location; }
    Module* getModule() const override { return &module; }
    static bool classof(const Decl* d) { return d->getKind() == DeclKind::VarDecl; }

private:
    std::string name;
    Expr* initializer;
    SourceLocation location;
    Module& module;
};

class FieldDecl : public VariableDecl {
public:
    FieldDecl(Type type, std::string&& name, TypeDecl& parent, AccessLevel accessLevel, SourceLocation location)
    : VariableDecl(DeclKind::FieldDecl, accessLevel, &parent, type), name(std::move(name)), location(location) {}
    llvm::StringRef getName() const override { return name; }
    std::string getQualifiedName() const;
    Module* getModule() const override { return getParent()->getModule(); }
    SourceLocation getLocation() const override { return location; }
    static bool classof(const Decl* d) { return d->getKind() == DeclKind::FieldDecl; }

private:
    std::string name;
    SourceLocation location;
};

class ImportDecl : public Decl {
public:
    ImportDecl(std::string&& target, Module& module, SourceLocation location)
    : Decl(DeclKind::ImportDecl, AccessLevel::None), target(std::move(target)), location(location), module(module) {}
    llvm::StringRef getName() const override { return ""; }
    llvm::StringRef getTarget() const { return target; }
    SourceLocation getLocation() const override { return location; }
    Module* getModule() const override { return &module; }
    static bool classof(const Decl* d) { return d->getKind() == DeclKind::ImportDecl; }

private:
    std::string target;
    SourceLocation location;
    Module& module;
};

} // namespace delta
