#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#pragma warning(push, 0)
#include <llvm/ADT/SmallVector.h>
#include <llvm/Support/Casting.h>
#pragma warning(pop)
#include "../support/utility.h"
#include "expr.h"
#include "location.h"
#include "stmt.h"
#include "type.h"

namespace std {
template<> struct hash<std::vector<cx::Type>> {
    size_t operator()(llvm::ArrayRef<cx::Type> types) const {
        if (types.empty()) return 0; // Variadic instantiation with an empty pack.
        size_t hashValue = reinterpret_cast<size_t>(types[0].typeBase) ^ static_cast<size_t>(types[0].mutability);

        for (auto type : types.drop_front()) {
            hashValue ^= reinterpret_cast<size_t>(type.typeBase) ^ static_cast<size_t>(type.mutability);
        }

        return hashValue;
    }
};
} // namespace std

namespace llvm {
class StringRef;
}

namespace cx {

struct Module;
struct TypeDecl;
struct FieldDecl;
struct FunctionDecl;
struct EnumDecl;

enum class DeclKind {
    GenericParamDecl,
    FunctionDecl,
    MethodDecl,
    ConstructorDecl,
    DestructorDecl,
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

struct Decl {
    virtual ~Decl() = default;
    bool isVariableDecl() const { return kind >= DeclKind::VarDecl && kind <= DeclKind::ParamDecl; }
    bool isParamDecl() const { return kind == DeclKind::ParamDecl; }
    bool isFunctionDecl() const { return kind >= DeclKind::FunctionDecl && kind <= DeclKind::DestructorDecl; }
    bool isMethodDecl() const { return kind >= DeclKind::MethodDecl && kind <= DeclKind::DestructorDecl; }
    bool isGenericParamDecl() const { return kind == DeclKind::GenericParamDecl; }
    bool isConstructorDecl() const { return kind == DeclKind::ConstructorDecl; }
    bool isDestructorDecl() const { return kind == DeclKind::DestructorDecl; }
    bool isFunctionTemplate() const { return kind == DeclKind::FunctionTemplate; }
    bool isTypeDecl() const { return kind == DeclKind::TypeDecl || kind == DeclKind::EnumDecl; }
    bool isTypeTemplate() const { return kind == DeclKind::TypeTemplate; }
    bool isEnumDecl() const { return kind == DeclKind::EnumDecl; }
    bool isEnumCaseDecl() const { return kind == DeclKind::EnumCase; }
    bool isVarDecl() const { return kind == DeclKind::VarDecl; }
    bool isFieldDecl() const { return kind == DeclKind::FieldDecl; }
    bool isImportDecl() const { return kind == DeclKind::ImportDecl; }
    virtual Module* getModule() const = 0;
    virtual Location getLocation() const = 0;
    virtual llvm::StringRef getName() const = 0;
    bool isMain() const { return getName() == "main"; }
    bool isLambda() const { return isFunctionDecl() && getName().starts_with("__lambda"); }
    virtual bool isGlobal() const;
    virtual bool isReferenced() const { return referenced; }
    bool hasBeenMoved() const;
    Decl* instantiate(const llvm::StringMap<Type>& genericArgs, llvm::ArrayRef<Type> genericArgsArray) const;

    DeclKind kind;
    AccessLevel accessLevel;
    bool referenced;

protected:
    Decl(DeclKind kind, AccessLevel accessLevel) : kind(kind), accessLevel(accessLevel), referenced(false) {}
};

struct Movable {
    bool moved = false;
};

/// Represents any variable declaration, including local variables, global variables, member variables, and parameters.
struct VariableDecl : Decl {
    static bool classof(const Decl* d) { return d->isVariableDecl(); }

    Decl* parent;
    Type type;

protected:
    VariableDecl(DeclKind kind, AccessLevel accessLevel, Decl* parent, Type type) : Decl(kind, accessLevel), parent(parent), type(type) {}
};

struct ParamDecl : VariableDecl, Movable {
    ParamDecl(Type type, std::string&& name, bool isPublic, Location location)
    : VariableDecl(DeclKind::ParamDecl, AccessLevel::None, nullptr /* initialized by FunctionDecl constructor */, type), name(std::move(name)),
      location(location), isPublic(isPublic) {}
    llvm::StringRef getName() const override { return name; }
    Module* getModule() const override { return nullptr; }
    Location getLocation() const override { return location; }
    static bool classof(const Decl* d) { return d->kind == DeclKind::ParamDecl; }
    bool operator==(const ParamDecl& other) const { return type == other.type && getName() == other.getName() && isPack == other.isPack; }

    std::string name;
    Location location;
    bool isPublic;
    bool isPack = false;
    Expr* defaultValue = nullptr;
};

std::vector<ParamDecl> instantiateParams(llvm::ArrayRef<ParamDecl> params, const llvm::StringMap<Type>& genericArgs);

struct GenericParamDecl : Decl {
    GenericParamDecl(std::string&& name, Location location) : Decl(DeclKind::GenericParamDecl, AccessLevel::None), name(std::move(name)), location(location) {}
    llvm::StringRef getName() const override { return name; }
    Module* getModule() const override { return nullptr; }
    Location getLocation() const override { return location; }
    static bool classof(const Decl* d) { return d->kind == DeclKind::GenericParamDecl; }

    std::string name;
    llvm::SmallVector<Type, 1> constraints;
    Location location;
};

struct FunctionProto {
    FunctionProto instantiate(const llvm::StringMap<Type>& genericArgs) const;

    std::string name;
    std::vector<ParamDecl> params;
    Type returnType;
    bool varArg;
    bool external;
    std::string asmLabel;
};

std::string getQualifiedFunctionName(Type receiver, llvm::StringRef name, llvm::ArrayRef<Type> genericArgs);

struct FunctionDecl : Decl {
    FunctionDecl(FunctionProto&& proto, std::vector<Type>&& genericArgs, AccessLevel accessLevel, Module& module, Location location)
    : FunctionDecl(DeclKind::FunctionDecl, std::move(proto), std::move(genericArgs), accessLevel, module, location) {
        for (auto& param : getParams()) {
            param.parent = this;
        }
    }
    bool isExtern() const { return proto.external; }
    bool isVariadic() const { return proto.varArg; }
    bool hasPack() const { return !proto.params.empty() && proto.params.back().isPack; }
    const ParamDecl* getPackParam() const { return hasPack() ? &proto.params.back() : nullptr; }
    llvm::StringRef getName() const override { return proto.name; }
    std::string getQualifiedName() const;
    Type getReturnType() const { return proto.returnType; }
    llvm::ArrayRef<ParamDecl> getParams() const { return proto.params; }
    llvm::MutableArrayRef<ParamDecl> getParams() { return proto.params; }
    virtual TypeDecl* getTypeDecl() const { return nullptr; }
    Location getLocation() const override { return location; }
    FunctionType* getFunctionType() const;
    bool signatureMatches(const FunctionDecl& other, bool matchReceiver = true) const;
    Module* getModule() const override { return &module; }
    FunctionDecl* instantiate(const llvm::StringMap<Type>& genericArgs, llvm::ArrayRef<Type> genericArgsArray);
    static bool classof(const Decl* d) { return d->isFunctionDecl(); }

    FunctionProto proto;
    std::vector<Type> genericArgs;
    std::optional<std::vector<Stmt*>> body;
    Location location;
    Module& module;
    bool typechecked;
    bool isPackInstantiation = false;

protected:
    FunctionDecl(DeclKind kind, FunctionProto&& proto, std::vector<Type>&& genericArgs, AccessLevel accessLevel, Module& module, Location location)
    : Decl(kind, accessLevel), proto(std::move(proto)), genericArgs(std::move(genericArgs)), location(location), module(module), typechecked(false) {}
};

struct MethodDecl : FunctionDecl {
    MethodDecl(FunctionProto proto, TypeDecl& receiverTypeDecl, std::vector<Type>&& genericArgs, AccessLevel accessLevel, Location location)
    : MethodDecl(DeclKind::MethodDecl, std::move(proto), receiverTypeDecl, std::move(genericArgs), accessLevel, location) {}
    TypeDecl* getTypeDecl() const override { return typeDecl; }
    MethodDecl* instantiate(const llvm::StringMap<Type>& genericArgs, llvm::ArrayRef<Type> genericArgsArray, TypeDecl& typeDecl);
    static bool classof(const Decl* d) { return d->isMethodDecl(); }
    TypeDecl* typeDecl;

protected:
    MethodDecl(DeclKind kind, FunctionProto proto, TypeDecl& typeDecl, std::vector<Type>&& genericArgs, AccessLevel accessLevel, Location location);
};

struct ConstructorDecl : MethodDecl {
    ConstructorDecl(TypeDecl& receiverTypeDecl, std::vector<ParamDecl>&& params, AccessLevel accessLevel, Location location);
    static bool classof(const Decl* d) { return d->kind == DeclKind::ConstructorDecl; }
};

struct DestructorDecl : MethodDecl {
    DestructorDecl(TypeDecl& receiverTypeDecl, Location location);
    static bool classof(const Decl* d) { return d->kind == DeclKind::DestructorDecl; }
};

struct FunctionTemplate : Decl {
    FunctionTemplate(std::vector<GenericParamDecl>&& genericParams, FunctionDecl* functionDecl, AccessLevel accessLevel)
    : Decl(DeclKind::FunctionTemplate, accessLevel), genericParams(std::move(genericParams)), functionDecl(functionDecl) {}
    llvm::StringRef getName() const override { return functionDecl->getName(); }
    std::string getQualifiedName() const { return functionDecl->getQualifiedName(); }
    bool isReferenced() const override;
    static bool classof(const Decl* d) { return d->isFunctionTemplate(); }
    FunctionDecl* instantiate(const llvm::StringMap<Type>& genericArgs);
    FunctionDecl* instantiateVariadic(const llvm::StringMap<Type>& fixedArgs, const std::vector<llvm::StringMap<Type>>& packArgs, std::vector<Type>&& cacheKey);
    Module* getModule() const override { return functionDecl->getModule(); }
    Location getLocation() const override { return functionDecl->getLocation(); }

    std::vector<GenericParamDecl> genericParams;
    FunctionDecl* functionDecl;
    std::unordered_map<std::vector<Type>, FunctionDecl*> instantiations;
};

struct FieldDecl : VariableDecl {
    FieldDecl(Type type, std::string&& name, Expr* defaultValue, TypeDecl& parent, AccessLevel accessLevel, Location location);
    llvm::StringRef getName() const override { return name; }
    std::string getQualifiedName() const;
    TypeDecl* getParentDecl() const { return llvm::cast<TypeDecl>(VariableDecl::parent); }
    Module* getModule() const override;
    Location getLocation() const override { return location; }
    FieldDecl instantiate(const llvm::StringMap<Type>& genericArgs, TypeDecl& typeDecl) const;
    static bool classof(const Decl* d) { return d->kind == DeclKind::FieldDecl; }

    std::string name;
    Expr* defaultValue;
    Location location;
};

enum class TypeTag { Struct, Interface, Union, Enum };

/// A non-template function declaration or a function template instantiation.
struct TypeDecl : Decl {
    TypeDecl(TypeTag tag, std::string&& name, std::vector<Type>&& genericArgs, std::vector<Type>&& interfaces, AccessLevel accessLevel, Module& module,
             const TypeDecl* instantiatedFrom, Location location)
    : Decl(DeclKind::TypeDecl, accessLevel), tag(tag), name(std::move(name)), genericArgs(std::move(genericArgs)), interfaces(std::move(interfaces)),
      location(location), module(module), instantiatedFrom(instantiatedFrom) {}
    llvm::StringRef getName() const override { return name; }
    std::string getQualifiedName() const;
    bool hasInterface(const TypeDecl& interface) const;
    bool isCopyable() const;
    Location getLocation() const override { return location; }
    void addField(FieldDecl&& field);
    void addMethod(Decl* decl);
    ConstructorDecl* addAutogeneratedConstructor();
    std::vector<ConstructorDecl*> getConstructors() const;
    DestructorDecl* getDestructor() const;
    Type getType(Mutability mutability = Mutability::Mutable) const;
    Type getTypeForPassing() const;
    bool passByValue() const { return (isStruct() && isCopyable()) || isUnion(); }
    bool isStruct() const { return tag == TypeTag::Struct; }
    bool isInterface() const { return tag == TypeTag::Interface; }
    bool isUnion() const { return tag == TypeTag::Union; }
    unsigned getFieldIndex(const FieldDecl* field) const;
    Module* getModule() const override { return &module; }
    static bool classof(const Decl* d) { return d->isTypeDecl(); }
    TypeDecl(DeclKind kind, TypeTag tag, std::string&& name, AccessLevel accessLevel, Module& module, const TypeDecl* instantiatedFrom, Location location)
    : Decl(kind, accessLevel), tag(tag), name(std::move(name)), location(location), module(module), instantiatedFrom(instantiatedFrom) {}

    TypeTag tag;
    std::string name;
    std::vector<Type> genericArgs;
    std::vector<Type> interfaces;
    std::vector<FieldDecl> fields;
    std::vector<Decl*> methods;
    Location location;
    Module& module;
    const TypeDecl* instantiatedFrom;
    bool packed = false;
};

struct TypeTemplate : Decl {
    TypeTemplate(std::vector<GenericParamDecl>&& genericParams, TypeDecl* typeDecl, AccessLevel accessLevel)
    : Decl(DeclKind::TypeTemplate, accessLevel), genericParams(std::move(genericParams)), typeDecl(typeDecl) {}
    llvm::StringRef getName() const override { return typeDecl->getName(); }
    TypeDecl* instantiate(const llvm::StringMap<Type>& genericArgs);
    TypeDecl* instantiate(llvm::ArrayRef<Type> genericArgs);
    Module* getModule() const override { return typeDecl->getModule(); }
    Location getLocation() const override { return typeDecl->getLocation(); }
    static bool classof(const Decl* d) { return d->kind == DeclKind::TypeTemplate; }

    std::vector<GenericParamDecl> genericParams;
    TypeDecl* typeDecl;
    std::unordered_map<std::vector<Type>, TypeDecl*> instantiations;
};

struct EnumCase : VariableDecl {
    EnumCase(std::string&& name, Expr* value, Type associatedType, AccessLevel accessLevel, Location location);
    llvm::StringRef getName() const override { return name; }
    EnumDecl* getEnumDecl() const { return llvm::cast<EnumDecl>(parent); }
    Location getLocation() const override { return location; }
    Module* getModule() const override { return parent->getModule(); }
    static bool classof(const Decl* d) { return d->kind == DeclKind::EnumCase; }

    std::string name;
    Expr* value;
    Type associatedType;
    Location location;
};

struct EnumDecl : TypeDecl {
    EnumDecl(std::string&& name, std::vector<EnumCase>&& cases, AccessLevel accessLevel, Module& module, const TypeDecl* instantiatedFrom, Location location)
    : TypeDecl(DeclKind::EnumDecl, TypeTag::Enum, std::move(name), accessLevel, module, instantiatedFrom, location), cases(std::move(cases)) {
        for (auto& enumCase : this->cases) {
            enumCase.parent = this;
            enumCase.type = NOTNULL(getType());
        }
    }
    bool hasAssociatedValues() const;
    EnumCase* getCaseByName(llvm::StringRef name);
    // TODO: Select tag type to be able to hold all enum values.
    Type getTagType() const { return Type::getInt(); }
    static bool classof(const Decl* d) { return d->kind == DeclKind::EnumDecl; }

    std::vector<EnumCase> cases;
};

struct VarDecl : VariableDecl, Movable {
    VarDecl(Type type, std::string&& name, Expr* initializer, Decl* parent, AccessLevel accessLevel, Module& module, Location location)
    : VariableDecl(DeclKind::VarDecl, accessLevel, parent, type), name(std::move(name)), initializer(initializer), location(location), module(module) {}
    llvm::StringRef getName() const override { return name; }
    Location getLocation() const override { return location; }
    Module* getModule() const override { return &module; }
    static bool classof(const Decl* d) { return d->kind == DeclKind::VarDecl; }

    std::string name;
    Expr* initializer;
    Location location;
    Module& module;
};

struct ImportDecl : Decl {
    ImportDecl(std::string&& target, Module& module, Location location)
    : Decl(DeclKind::ImportDecl, AccessLevel::None), target(std::move(target)), location(location), module(module) {}
    llvm::StringRef getName() const override { return ""; }
    Location getLocation() const override { return location; }
    Module* getModule() const override { return &module; }
    static bool classof(const Decl* d) { return d->kind == DeclKind::ImportDecl; }

    std::string target;
    Location location;
    Module& module;
    std::string importedHeaderPath;
};

std::vector<Note> getPreviousDefinitionNotes(llvm::ArrayRef<Decl*> decls);

} // namespace cx
