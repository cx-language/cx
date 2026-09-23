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
template<> struct hash<std::vector<cx::GenericArg>> {
    size_t operator()(llvm::ArrayRef<cx::GenericArg> args) const {
        if (args.empty()) return 0; // Variadic instantiation with an empty pack.
        size_t hashValue = reinterpret_cast<size_t>(args[0].type.typeBase) ^ static_cast<size_t>(args[0].type.mutability)
                         ^ (args[0].intValue ? std::hash<int64_t>{}(*args[0].intValue) : 0);

        for (auto arg : args.drop_front()) {
            hashValue ^= reinterpret_cast<size_t>(arg.type.typeBase) ^ static_cast<size_t>(arg.type.mutability)
                       ^ (arg.intValue ? std::hash<int64_t>{}(*arg.intValue) : 0);
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
struct VarDecl;
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
    Decl* instantiate(const llvm::StringMap<GenericArg>& genericArgs, llvm::ArrayRef<GenericArg> genericArgsArray) const;

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

    /// The implicit `this` is a pointer at runtime even with a by-value declared type
    /// (Copyable struct or union receiver), so capturing it carries the pointer.
    bool isReferenceCapture() const { return getName() == "this"; }
    /// The closure field and hidden parameter type when capturing this variable.
    Type getCaptureType() const {
        // Capturing a borrow stores the address like capturing a pointer; the closure never owns the value.
        if (type.isReferenceType()) return type.getPointee().getPointerTo();
        return isReferenceCapture() ? type.removePointer().getPointerTo() : type;
    }

protected:
    VariableDecl(DeclKind kind, AccessLevel accessLevel, Decl* parent, Type type) : Decl(kind, accessLevel), parent(parent), type(type) {}
};

struct ParamDecl : VariableDecl, Movable {
    ParamDecl(Type type, llvm::StringRef name, bool isPublic, Location location)
    : VariableDecl(DeclKind::ParamDecl, AccessLevel::None, nullptr /* initialized by FunctionDecl constructor */, type), name(internString(name)),
      location(location), isPublic(isPublic) {}
    llvm::StringRef getName() const override { return name; }
    Module* getModule() const override { return nullptr; }
    Location getLocation() const override { return location; }
    static bool classof(const Decl* d) { return d->kind == DeclKind::ParamDecl; }
    bool operator==(const ParamDecl& other) const { return type == other.type && getName() == other.getName() && isPack == other.isPack; }

    llvm::StringRef name;
    Location location;
    bool isPublic;
    bool isPack = false;
    Expr* defaultValue = nullptr;
};

std::vector<ParamDecl> instantiateParams(llvm::ArrayRef<ParamDecl> params, const llvm::StringMap<GenericArg>& genericArgs);

struct GenericParamDecl : Decl {
    GenericParamDecl(llvm::StringRef name, Location location)
    : Decl(DeclKind::GenericParamDecl, AccessLevel::None), name(internString(name)), location(location) {}
    llvm::StringRef getName() const override { return name; }
    Module* getModule() const override { return nullptr; }
    Location getLocation() const override { return location; }
    static bool classof(const Decl* d) { return d->kind == DeclKind::GenericParamDecl; }

    llvm::StringRef name;
    llvm::SmallVector<Type, 1> constraints;
    // Set for integer parameters (declared as e.g. `int N`); valueType is the integer type.
    bool isValueParam = false;
    Type valueType;
    Location location;
};

struct FunctionProto {
    FunctionProto(llvm::StringRef name = {}, std::vector<ParamDecl> params = {}, Type returnType = {}, bool varArg = false, bool external = false)
    : name(internString(name)), params(std::move(params)), returnType(returnType), varArg(varArg), external(external) {}
    FunctionProto instantiate(const llvm::StringMap<GenericArg>& genericArgs) const;

    llvm::StringRef name;
    std::vector<ParamDecl> params;
    Type returnType;
    bool varArg;
    bool external;
    std::string asmLabel;
};

std::string getQualifiedFunctionName(Type receiver, llvm::StringRef name, llvm::ArrayRef<GenericArg> genericArgs);

struct FunctionDecl : Decl {
    FunctionDecl(FunctionProto&& proto, std::vector<GenericArg>&& genericArgs, AccessLevel accessLevel, Module& module, Location location)
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
    FunctionDecl* instantiate(const llvm::StringMap<GenericArg>& genericArgs, llvm::ArrayRef<GenericArg> genericArgsArray);
    static bool classof(const Decl* d) { return d->isFunctionDecl(); }

    FunctionProto proto;
    std::vector<GenericArg> genericArgs;
    std::optional<std::vector<Stmt*>> body;
    Location location;
    Module& module;
    bool typechecked;
    bool isPackInstantiation = false;
    // Set by the `test` marker; collected and run by `cx test`.
    bool isTest = false;
    // Enclosing function for lambdas, null otherwise. Set during typechecking.
    FunctionDecl* parentFunction = nullptr;
    // Outer locals and parameters captured by value, in first-use order. Only lambdas capture.
    // Codegen passes these as hidden leading parameters; the AST params only hold user parameters.
    std::vector<VariableDecl*> captures;

protected:
    FunctionDecl(DeclKind kind, FunctionProto&& proto, std::vector<GenericArg>&& genericArgs, AccessLevel accessLevel, Module& module, Location location)
    : Decl(kind, accessLevel), proto(std::move(proto)), genericArgs(std::move(genericArgs)), location(location), module(module), typechecked(false) {}
};

struct MethodDecl : FunctionDecl {
    MethodDecl(FunctionProto proto, TypeDecl& receiverTypeDecl, std::vector<GenericArg>&& genericArgs, AccessLevel accessLevel, Location location)
    : MethodDecl(DeclKind::MethodDecl, std::move(proto), receiverTypeDecl, std::move(genericArgs), accessLevel, location) {}
    TypeDecl* getTypeDecl() const override { return typeDecl; }
    MethodDecl* instantiate(const llvm::StringMap<GenericArg>& genericArgs, llvm::ArrayRef<GenericArg> genericArgsArray, TypeDecl& typeDecl);
    static bool classof(const Decl* d) { return d->isMethodDecl(); }
    TypeDecl* typeDecl;

protected:
    MethodDecl(DeclKind kind, FunctionProto proto, TypeDecl& typeDecl, std::vector<GenericArg>&& genericArgs, AccessLevel accessLevel, Location location);
};

struct ConstructorDecl : MethodDecl {
    ConstructorDecl(TypeDecl& receiverTypeDecl, std::vector<ParamDecl>&& params, AccessLevel accessLevel, Location location);
    static bool classof(const Decl* d) { return d->kind == DeclKind::ConstructorDecl; }

    bool isAutogenerated = false;
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
    FunctionDecl* instantiate(const llvm::StringMap<GenericArg>& genericArgs);
    FunctionDecl* instantiateVariadic(const llvm::StringMap<GenericArg>& fixedArgs, const std::vector<llvm::StringMap<GenericArg>>& packArgs,
                                      std::vector<GenericArg>&& cacheKey);
    Module* getModule() const override { return functionDecl->getModule(); }
    Location getLocation() const override { return functionDecl->getLocation(); }

    std::vector<GenericParamDecl> genericParams;
    FunctionDecl* functionDecl;
    std::unordered_map<std::vector<GenericArg>, FunctionDecl*> instantiations;
};

struct FieldDecl : VariableDecl {
    FieldDecl(Type type, llvm::StringRef name, Expr* defaultValue, TypeDecl& parent, AccessLevel accessLevel, Location location);
    llvm::StringRef getName() const override { return name; }
    std::string getQualifiedName() const;
    TypeDecl* getParentDecl() const { return llvm::cast<TypeDecl>(VariableDecl::parent); }
    Module* getModule() const override;
    Location getLocation() const override { return location; }
    FieldDecl instantiate(const llvm::StringMap<GenericArg>& genericArgs, TypeDecl& typeDecl) const;
    static bool classof(const Decl* d) { return d->kind == DeclKind::FieldDecl; }

    llvm::StringRef name;
    Expr* defaultValue;
    Location location;
};

enum class TypeTag { Struct, Interface, Union, Enum };

/// A non-template function declaration or a function template instantiation.
struct TypeDecl : Decl {
    TypeDecl(TypeTag tag, llvm::StringRef name, std::vector<GenericArg>&& genericArgs, std::vector<Type>&& interfaces, AccessLevel accessLevel, Module& module,
             const TypeDecl* instantiatedFrom, Location location)
    : Decl(DeclKind::TypeDecl, accessLevel), tag(tag), name(internString(name)), genericArgs(std::move(genericArgs)), interfaces(std::move(interfaces)),
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
    bool isClosure() const { return isStruct() && getName().starts_with("__closure"); }
    Type getTypeForPassing() const;
    bool passByValue() const { return ((isStruct() || tag == TypeTag::Enum) && isCopyable()) || isUnion(); }
    bool isStruct() const { return tag == TypeTag::Struct; }
    bool isInterface() const { return tag == TypeTag::Interface; }
    bool isUnion() const { return tag == TypeTag::Union; }
    unsigned getFieldIndex(const FieldDecl* field) const;
    Module* getModule() const override { return &module; }
    static bool classof(const Decl* d) { return d->isTypeDecl(); }
    TypeDecl(DeclKind kind, TypeTag tag, llvm::StringRef name, AccessLevel accessLevel, Module& module, const TypeDecl* instantiatedFrom, Location location)
    : Decl(kind, accessLevel), tag(tag), name(internString(name)), location(location), module(module), instantiatedFrom(instantiatedFrom) {}

    TypeTag tag;
    llvm::StringRef name;
    std::vector<GenericArg> genericArgs;
    std::vector<Type> interfaces;
    std::vector<FieldDecl> fields;
    std::vector<Decl*> methods;
    // Constants scoped under the type name, accessed as `Type.constant`.
    std::vector<VarDecl*> staticConsts;
    Location location;
    Module& module;
    const TypeDecl* instantiatedFrom;
    bool packed = false;
};

struct TypeTemplate : Decl {
    TypeTemplate(std::vector<GenericParamDecl>&& genericParams, TypeDecl* typeDecl, AccessLevel accessLevel)
    : Decl(DeclKind::TypeTemplate, accessLevel), genericParams(std::move(genericParams)), typeDecl(typeDecl) {}
    llvm::StringRef getName() const override { return typeDecl->getName(); }
    TypeDecl* instantiate(const llvm::StringMap<GenericArg>& genericArgs);
    TypeDecl* instantiate(llvm::ArrayRef<GenericArg> genericArgs);
    Module* getModule() const override { return typeDecl->getModule(); }
    Location getLocation() const override { return typeDecl->getLocation(); }
    static bool classof(const Decl* d) { return d->kind == DeclKind::TypeTemplate; }

    std::vector<GenericParamDecl> genericParams;
    TypeDecl* typeDecl;
    std::unordered_map<std::vector<GenericArg>, TypeDecl*> instantiations;
};

struct EnumCase : VariableDecl {
    EnumCase(llvm::StringRef name, Expr* value, Type associatedType, AccessLevel accessLevel, Location location);
    llvm::StringRef getName() const override { return name; }
    EnumDecl* getEnumDecl() const { return llvm::cast<EnumDecl>(parent); }
    Location getLocation() const override { return location; }
    Module* getModule() const override { return parent->getModule(); }
    static bool classof(const Decl* d) { return d->kind == DeclKind::EnumCase; }

    llvm::StringRef name;
    Expr* value;
    Type associatedType;
    Location location;
};

struct EnumDecl : TypeDecl {
    EnumDecl(llvm::StringRef name, std::vector<EnumCase>&& cases, std::vector<Type>&& interfaces, AccessLevel accessLevel, Module& module,
             const TypeDecl* instantiatedFrom, Location location)
    : TypeDecl(DeclKind::EnumDecl, TypeTag::Enum, name, accessLevel, module, instantiatedFrom, location), cases(std::move(cases)) {
        this->interfaces = std::move(interfaces);
        for (auto& enumCase : this->cases) {
            enumCase.parent = this;
            enumCase.type = NOTNULL(getType());
        }
    }
    void addCase(EnumCase&& enumCase);
    bool hasAssociatedValues() const;
    EnumCase* getCaseByName(llvm::StringRef name);
    // TODO: Select tag type to be able to hold all enum values.
    Type getTagType() const { return Type::getInt(); }
    static bool classof(const Decl* d) { return d->kind == DeclKind::EnumDecl; }

    std::vector<EnumCase> cases;
};

struct VarDecl : VariableDecl, Movable {
    VarDecl(Type type, llvm::StringRef name, Expr* initializer, Decl* parent, AccessLevel accessLevel, Module& module, Location location)
    : VariableDecl(DeclKind::VarDecl, accessLevel, parent, type), name(internString(name)), initializer(initializer), location(location), module(module) {}
    llvm::StringRef getName() const override { return name; }
    Location getLocation() const override { return location; }
    Module* getModule() const override { return &module; }
    static bool classof(const Decl* d) { return d->kind == DeclKind::VarDecl; }

    llvm::StringRef name;
    Expr* initializer;
    Location location;
    Module& module;
};

struct ImportDecl : Decl {
    ImportDecl(llvm::StringRef target, Module& module, Location location)
    : Decl(DeclKind::ImportDecl, AccessLevel::None), target(internString(target)), location(location), module(module) {}
    llvm::StringRef getName() const override { return ""; }
    Location getLocation() const override { return location; }
    Module* getModule() const override { return &module; }
    static bool classof(const Decl* d) { return d->kind == DeclKind::ImportDecl; }

    llvm::StringRef target;
    Location location;
    Module& module;
    std::string importedHeaderPath;
};

std::vector<Note> getPreviousDefinitionNotes(llvm::ArrayRef<Decl*> decls);

} // namespace cx
