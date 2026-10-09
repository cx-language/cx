#pragma once

#include <string>
#include <utility>
#include <vector>
#pragma warning(push, 0)
#include <llvm/Support/Casting.h>
#pragma warning(pop)
#include "../support/utility.h"
#include "expr.h"
#include "location.h"
#include "stmt.h"
#include "type.h"

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
struct TypeAliasDecl;

enum class DeclKind {
    GenericParamDecl,
    FunctionDecl,
    MethodDecl,
    ConstructorDecl,
    DestructorDecl,
    FunctionTemplate,
    TypeDecl,
    TypeTemplate,
    TypeAliasDecl,
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
    bool isTypeAliasDecl() const { return kind == DeclKind::TypeAliasDecl; }
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

    // Lazily checked declarations in imported modules transition Unchecked ->
    // CheckingSignature -> SignatureChecked -> CheckingBody -> Checked. The
    // in-progress states break re-entrant cycles (recursive types/functions);
    // the main module is still checked eagerly up front.
    enum class CheckState : uint8_t { Unchecked, CheckingSignature, SignatureChecked, CheckingBody, Checked };
    CheckState checkState = CheckState::Unchecked;

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

    bool isReferenceCapture() const { return getName() == "this"; }
    /// The closure field and hidden parameter type when capturing this variable.
    Type getCaptureType() const {
        // Capturing a borrow stores the address like capturing a pointer; the closure never owns the value.
        return type.isReferenceType() ? type.getPointee().getPointerTo() : type;
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
    // True for lambda borrow parameters bound from constant storage (see
    // callOnConstReceiver): writes through them are rejected like any
    // other write to a constant.
    bool isConst = false;
    bool isPack = false;
    // Mangle-only const on extern "C++" pointer/reference parameters; the cx type stays mutable.
    bool cxxConstPointee = false;
    Expr* defaultValue = nullptr;
};

AstVector<ParamDecl> instantiateParams(llvm::ArrayRef<ParamDecl> params, const llvm::StringMap<GenericArg>& genericArgs);

struct GenericParamDecl : Decl {
    GenericParamDecl(llvm::StringRef name, Location location)
    : Decl(DeclKind::GenericParamDecl, AccessLevel::None), name(internString(name)), location(location) {}
    llvm::StringRef getName() const override { return name; }
    Module* getModule() const override { return nullptr; }
    Location getLocation() const override { return location; }
    static bool classof(const Decl* d) { return d->kind == DeclKind::GenericParamDecl; }

    llvm::StringRef name;
    AstVector<Type> constraints;
    // Set for integer parameters (declared as e.g. `int N`); valueType is the integer type.
    bool isValueParam = false;
    Type valueType;
    Location location;
};

struct FunctionProto {
    FunctionProto(llvm::StringRef name = {}, AstVector<ParamDecl> params = {}, Type returnType = {}, bool varArg = false, bool external = false,
                  bool cppLinkage = false)
    : name(internString(name)), params(std::move(params)), returnType(returnType), varArg(varArg), external(external), cppLinkage(cppLinkage) {}
    FunctionProto instantiate(const llvm::StringMap<GenericArg>& genericArgs) const;

    llvm::StringRef name;
    AstVector<ParamDecl> params;
    Type returnType;
    bool varArg;
    bool external;
    // Set for `extern "C++"` declarations, which use Itanium name mangling.
    bool cppLinkage;
    llvm::StringRef asmLabel;
};

std::string getQualifiedFunctionName(Type receiver, llvm::StringRef name, llvm::ArrayRef<GenericArg> genericArgs);

struct FunctionDecl : Decl {
    FunctionDecl(FunctionProto&& proto, AstVector<GenericArg>&& genericArgs, AccessLevel accessLevel, Module& module, Location location)
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
    AstVector<GenericArg> genericArgs;
    std::optional<AstVector<Stmt*>> body;
    Location location;
    Module& module;
    bool isPackInstantiation = false;
    // Set by the `test` marker; collected and run by `cx test`.
    bool isTest = false;
    // Safety checks disabled for this function body by `@unchecked`-family attributes.
    DisabledChecks disabledChecks = DisabledChecks::None;
    // Set by the `implicit` marker on single-parameter constructors and
    // parameterless member functions; enables implicit conversions.
    bool isImplicit = false;
    // The program entry point: the single non-generic, non-method "main" in
    // the main module. Only it lowers to the raw "main" symbol with argc/argv
    // handling; any other function named "main" mangles like an ordinary one.
    bool isEntryPoint = false;
    // Enclosing function for lambdas, null otherwise. Set during typechecking.
    FunctionDecl* parentFunction = nullptr;
    // Outer locals and parameters captured by value, in first-use order. Only lambdas capture.
    // Codegen passes these as hidden leading parameters; the AST params only hold user parameters.
    AstVector<VariableDecl*> captures;

protected:
    FunctionDecl(DeclKind kind, FunctionProto&& proto, AstVector<GenericArg>&& genericArgs, AccessLevel accessLevel, Module& module, Location location)
    : Decl(kind, accessLevel), proto(std::move(proto)), genericArgs(std::move(genericArgs)), location(location), module(module) {}
};

struct MethodDecl : FunctionDecl {
    MethodDecl(FunctionProto proto, TypeDecl& receiverTypeDecl, AstVector<GenericArg>&& genericArgs, AccessLevel accessLevel, Location location)
    : MethodDecl(DeclKind::MethodDecl, std::move(proto), receiverTypeDecl, std::move(genericArgs), accessLevel, location) {}
    TypeDecl* getTypeDecl() const override { return typeDecl; }
    MethodDecl* instantiate(const llvm::StringMap<GenericArg>& genericArgs, llvm::ArrayRef<GenericArg> genericArgsArray, TypeDecl& typeDecl);
    static bool classof(const Decl* d) { return d->isMethodDecl(); }
    TypeDecl* typeDecl;
    // Interface this method was copied from by ensureInterfaces, null otherwise.
    // Copies keep their origin locations; implementer-relative diagnostics use this.
    const TypeDecl* copiedFromInterface = nullptr;

protected:
    MethodDecl(DeclKind kind, FunctionProto proto, TypeDecl& typeDecl, AstVector<GenericArg>&& genericArgs, AccessLevel accessLevel, Location location);
};

struct ConstructorDecl : MethodDecl {
    ConstructorDecl(TypeDecl& receiverTypeDecl, AstVector<ParamDecl>&& params, AccessLevel accessLevel, Location location);
    static bool classof(const Decl* d) { return d->kind == DeclKind::ConstructorDecl; }

    bool isAutogenerated = false;
};

struct DestructorDecl : MethodDecl {
    DestructorDecl(TypeDecl& receiverTypeDecl, Location location);
    static bool classof(const Decl* d) { return d->kind == DeclKind::DestructorDecl; }
};

struct FunctionTemplate : Decl {
    FunctionTemplate(AstVector<GenericParamDecl>&& genericParams, FunctionDecl* functionDecl, AccessLevel accessLevel)
    : Decl(DeclKind::FunctionTemplate, accessLevel), genericParams(std::move(genericParams)), functionDecl(functionDecl) {}
    llvm::StringRef getName() const override { return functionDecl->getName(); }
    std::string getQualifiedName() const { return functionDecl->getQualifiedName(); }
    bool isReferenced() const override;
    static bool classof(const Decl* d) { return d->isFunctionTemplate(); }
    FunctionDecl* instantiate(const llvm::StringMap<GenericArg>& genericArgs);
    FunctionDecl* instantiateVariadic(const llvm::StringMap<GenericArg>& fixedArgs, const std::vector<llvm::StringMap<GenericArg>>& packArgs,
                                      AstVector<GenericArg>&& cacheKey);
    Module* getModule() const override { return functionDecl->getModule(); }
    Location getLocation() const override { return functionDecl->getLocation(); }

    AstVector<GenericParamDecl> genericParams;
    FunctionDecl* functionDecl;
    // Linear instantiation cache; templates have few instantiations, hash it if lookup regresses.
    AstVector<std::pair<AstVector<GenericArg>, FunctionDecl*>> instantiations;
};

struct FieldDecl : VariableDecl {
    FieldDecl(Type type, llvm::StringRef name, Expr* defaultValue, TypeDecl& parent, AccessLevel accessLevel, Location location,
              bool isManuallyDestroy = false);
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
    // Set by '@manuallyDestroy': the destructor skips this field, so the
    // struct's own destructor must destroy it explicitly.
    bool isManuallyDestroy = false;
    // Set by the C importer for generated members standing in for anonymous
    // structs/unions. The name (unnamed_N) exists only on the cx side; C
    // promotes the members, so the C backend addresses them directly.
    bool isAnonymousMember = false;
};

enum class TypeTag { Struct, Interface, Union, Enum };

/// A non-template function declaration or a function template instantiation.
struct TypeDecl : Decl {
    TypeDecl(TypeTag tag, llvm::StringRef name, AstVector<GenericArg>&& genericArgs, AstVector<Type>&& interfaces, AccessLevel accessLevel, Module& module,
             const TypeDecl* instantiatedFrom, Location location)
    : Decl(DeclKind::TypeDecl, accessLevel), tag(tag), name(internString(name)), genericArgs(std::move(genericArgs)), interfaces(std::move(interfaces)),
      location(location), module(module), instantiatedFrom(instantiatedFrom) {}
    llvm::StringRef getName() const override { return name; }
    std::string getQualifiedName() const;
    bool hasInterface(const TypeDecl& interface) const;
    bool implementsInterface(llvm::StringRef name) const;
    bool isCopyable() const;
    Location getLocation() const override { return location; }
    void addField(FieldDecl&& field);
    void addMethod(Decl* decl);
    ConstructorDecl* addAutogeneratedConstructor();
    std::vector<ConstructorDecl*> getConstructors() const;
    std::vector<FunctionTemplate*> getConstructorTemplates() const;
    DestructorDecl* getDestructor() const;
    // Synthesizes a default destructor destroying owning fields except
    // '@manuallyDestroy' ones (or the active enum payload), or null when
    // nothing needs destruction. Shared by sema (explicit `.deinit()` calls)
    // and irgen (implicit destruction).
    DestructorDecl* getOrSynthesizeDefaultDestructor();
    Type getType() const;
    bool isClosure() const { return isStruct() && getName().starts_with("__closure"); }
    // Whether values of this type are copied rather than moved. Independent of
    // receiver passing: method receivers are always T&.
    bool isStoredByValue() const { return (isStruct() || tag == TypeTag::Enum || isUnion()) && isCopyable(); }
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
    AstVector<GenericArg> genericArgs;
    AstVector<Type> interfaces;
    AstVector<FieldDecl> fields;
    AstVector<Decl*> methods;
    // Constants scoped under the type name, accessed as `Type.constant`.
    AstVector<VarDecl*> staticConsts;
    Location location;
    Module& module;
    const TypeDecl* instantiatedFrom;
    bool packed = false;
    // Set by the C importer for records without a tag: generated records standing
    // in for anonymous structs/unions, and typedef-named ones. Neither can be
    // referenced as `struct Name`, so the C backend emits its own definitions
    // instead of using the header's.
    bool isAnonymousRecord = false;
    // Interface method materialization runs once: the main-module
    // prepass and lazy use both funnel through ensureInterfaces.
    bool interfacesEnsured = false;
};

struct TypeTemplate : Decl {
    TypeTemplate(AstVector<GenericParamDecl>&& genericParams, TypeDecl* typeDecl, AccessLevel accessLevel)
    : Decl(DeclKind::TypeTemplate, accessLevel), genericParams(std::move(genericParams)), typeDecl(typeDecl) {}
    llvm::StringRef getName() const override { return typeDecl->getName(); }
    TypeDecl* instantiate(const llvm::StringMap<GenericArg>& genericArgs);
    TypeDecl* instantiate(llvm::ArrayRef<GenericArg> genericArgs);
    Module* getModule() const override { return typeDecl->getModule(); }
    Location getLocation() const override { return typeDecl->getLocation(); }
    static bool classof(const Decl* d) { return d->kind == DeclKind::TypeTemplate; }

    AstVector<GenericParamDecl> genericParams;
    TypeDecl* typeDecl;
    // Linear instantiation cache; templates have few instantiations, hash it if lookup regresses.
    AstVector<std::pair<AstVector<GenericArg>, TypeDecl*>> instantiations;
};

struct TypeAliasDecl : Decl {
    TypeAliasDecl(llvm::StringRef name, Type aliasedType, AccessLevel accessLevel, Module& module, Location location)
    : Decl(DeclKind::TypeAliasDecl, accessLevel), name(internString(name)), aliasedType(aliasedType), location(location), module(module) {}
    llvm::StringRef getName() const override { return name; }
    Module* getModule() const override { return &module; }
    Location getLocation() const override { return location; }
    static bool classof(const Decl* d) { return d->kind == DeclKind::TypeAliasDecl; }

    llvm::StringRef name;
    Type aliasedType;
    Location location;
    Module& module;
    bool cycleReported = false;
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
    EnumDecl(llvm::StringRef name, AstVector<EnumCase>&& cases, AstVector<Type>&& interfaces, AccessLevel accessLevel, Module& module,
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
    // True when `viewed` is a value-narrowed view of a `declared`-typed variable: one case's
    // associated type, or `()` when the enum has a payloadless case. Optional unwrapping is a
    // separate narrowing with its own handling, so optionals never match here.
    static bool isPayloadView(Type declared, Type viewed);
    bool hasDestructiblePayload() const;
    EnumCase* getCaseByName(llvm::StringRef name);
    // TODO: Select tag type to be able to hold all enum values.
    Type getTagType() const { return Type::getInt32(); }
    static bool classof(const Decl* d) { return d->kind == DeclKind::EnumDecl; }

    AstVector<EnumCase> cases;
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
    // True for lowered for-loop element variables yielding a borrow: the variable aliases
    // the element instead of copying it out, so reference types are preserved, not dereferenced.
    bool isForLoopElement = false;
    // True for the lowered for-loop iterator temp: its locations all collapse
    // to the loop line, so it gets no view-freeze record of its own; the loop
    // over its range is frozen separately for the whole loop.
    bool isForLoopIterator = false;
    // True for bindings established implicitly by the compiler (switch-case and `is`
    // payload bindings, comparison temporaries): codegen binds them to their values,
    // so a borrow-typed binding needs no initializer.
    bool isImplicitlyBound = false;
    // Set by '@manuallyDestroy': scope exit does not run this variable's destructor.
    // Assignment still destroys the previous value.
    bool isManuallyDestroy = false;
    // True for constant bindings: 'const' declarator, static consts, C consts.
    bool isConst = false;
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
    llvm::StringRef importedHeaderPath;
};

std::vector<Note> getPreviousDefinitionNotes(llvm::ArrayRef<Decl*> decls);

// Binds a type spelling to its declaration. Type nodes are interned by spelling
// across the whole compilation, so registering a different declaration under an
// already-bound spelling (e.g. a cx struct colliding with an imported C struct)
// would silently resolve one to the other; that is a redefinition error.
void bindTypeSpelling(Type type, TypeDecl& decl);

} // namespace cx
