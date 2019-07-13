#include "decl.h"
#pragma warning(push, 0)
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/ErrorHandling.h>
#pragma warning(pop)
#include "mangle.h"
#include "../support/utility.h"

using namespace delta;

FunctionProto FunctionProto::instantiate(const llvm::StringMap<Type>& genericArgs) const {
    auto params = instantiateParams(getParams(), genericArgs);
    auto returnType = getReturnType().resolve(genericArgs);
    std::vector<GenericParamDecl> genericParams;
    return FunctionProto(getName().str(), std::move(params), returnType, isVarArg(), isExtern());
}

FunctionDecl* FunctionTemplate::instantiate(const llvm::StringMap<Type>& genericArgs) {
    ASSERT(!genericParams.empty() && !genericArgs.empty());

    auto orderedGenericArgs = map(genericParams, [&](auto& genericParam) { return genericArgs.find(genericParam.getName())->second; });

    auto it = instantiations.find(orderedGenericArgs);
    if (it != instantiations.end()) return it->second;
    auto instantiation = getFunctionDecl()->instantiate(genericArgs, orderedGenericArgs);
    return instantiations.emplace(std::move(orderedGenericArgs), instantiation).first->second;
}

std::string delta::getQualifiedFunctionName(Type receiver, llvm::StringRef name, llvm::ArrayRef<Type> genericArgs) {
    std::string result;

    if (receiver) {
        result = receiver.toString(true);
        result += '.';
    }

    result += name;
    appendGenericArgs(result, genericArgs);
    return result;
}

std::string FunctionDecl::getQualifiedName() const {
    Type receiver = getTypeDecl() ? getTypeDecl()->getType() : Type();
    return getQualifiedFunctionName(receiver, getName(), getGenericArgs());
}

FunctionType* FunctionDecl::getFunctionType() const {
    auto paramTypes = map(getParams(), [](const ParamDecl& p) -> Type { return p.getType(); });
    return &llvm::cast<FunctionType>(*FunctionType::get(getReturnType(), std::move(paramTypes)));
}

bool FunctionDecl::signatureMatches(const FunctionDecl& other, bool matchReceiver) const {
    if (getName() != other.getName()) return false;
    if (matchReceiver && getTypeDecl() != other.getTypeDecl()) return false;
    if (getReturnType() != other.getReturnType()) return false;
    if (getParams() != other.getParams()) return false;
    return true;
}

FunctionDecl* FunctionDecl::instantiate(const llvm::StringMap<Type>& genericArgs, llvm::ArrayRef<Type> genericArgsArray) {
    auto proto = getProto().instantiate(genericArgs);
    auto body = ::instantiate(getBody(), genericArgs);

    FunctionDecl* instantiation;

    if (isMethodDecl()) {
        instantiation = new MethodDecl(std::move(proto), *getTypeDecl(), genericArgsArray, getAccessLevel(), location);
    } else {
        instantiation = new FunctionDecl(std::move(proto), genericArgsArray, getAccessLevel(), module, location);
    }

    instantiation->setBody(std::move(body));
    return instantiation;
}

bool FunctionTemplate::isReferenced() const {
    if (Decl::isReferenced()) {
        return true;
    }

    for (auto& instantiation : instantiations) {
        if (instantiation.second->isReferenced()) {
            return true;
        }
    }

    return false;
}

MethodDecl::MethodDecl(DeclKind kind, FunctionProto proto, TypeDecl& typeDecl, std::vector<Type>&& genericArgs, AccessLevel accessLevel, SourceLocation location)
: FunctionDecl(kind, std::move(proto), std::move(genericArgs), accessLevel, *typeDecl.getModule(), location), typeDecl(&typeDecl) {}

MethodDecl* MethodDecl::instantiate(const llvm::StringMap<Type>& genericArgs, TypeDecl& typeDecl) {
    switch (getKind()) {
        case DeclKind::MethodDecl: {
            auto* methodDecl = llvm::cast<MethodDecl>(this);
            auto proto = methodDecl->getProto().instantiate(genericArgs);
            auto instantiation = new MethodDecl(std::move(proto), typeDecl, std::vector<Type>(), getAccessLevel(), methodDecl->getLocation());
            if (methodDecl->hasBody()) {
                instantiation->setBody(::instantiate(methodDecl->getBody(), genericArgs));
            }
            return instantiation;
        }
        case DeclKind::InitDecl: {
            auto* initDecl = llvm::cast<InitDecl>(this);
            auto params = instantiateParams(initDecl->getParams(), genericArgs);
            auto instantiation = new InitDecl(typeDecl, std::move(params), getAccessLevel(), initDecl->getLocation());
            instantiation->setBody(::instantiate(initDecl->getBody(), genericArgs));
            return instantiation;
        }
        case DeclKind::DeinitDecl: {
            auto* deinitDecl = llvm::cast<DeinitDecl>(this);
            auto instantiation = new DeinitDecl(typeDecl, deinitDecl->getLocation());
            instantiation->setBody(::instantiate(deinitDecl->getBody(), genericArgs));
            return instantiation;
        }
        default:
            llvm_unreachable("invalid method decl");
    }
}

std::vector<ParamDecl> delta::instantiateParams(llvm::ArrayRef<ParamDecl> params, const llvm::StringMap<Type>& genericArgs) {
    return map(params, [&](auto& param) { return ParamDecl(param.getType().resolve(genericArgs), param.getName(), param.getLocation()); });
}

std::string TypeDecl::getQualifiedName() const {
    return getQualifiedTypeName(getName(), getGenericArgs());
}

bool TypeDecl::hasInterface(const TypeDecl& interface) const {
    return llvm::any_of(getInterfaces(), [&](Type type) { return type.getDecl() == &interface; });
}

bool TypeDecl::isCopyable() const {
    return llvm::any_of(getInterfaces(), [&](Type type) { return type.getName() == "Copyable"; });
}

void TypeDecl::addField(FieldDecl&& field) {
    fields.emplace_back(std::move(field));
}

void TypeDecl::addMethod(Decl* decl) {
    methods.push_back(decl);
}

DeinitDecl* TypeDecl::getDeinitializer() const {
    for (auto& decl : getMemberDecls()) {
        if (auto* deinitDecl = llvm::dyn_cast<DeinitDecl>(decl)) {
            return deinitDecl;
        }
    }
    return nullptr;
}

Type TypeDecl::getType(Mutability mutability) const {
    return BasicType::get(name, genericArgs, mutability);
}

Type TypeDecl::getTypeForPassing() const {
    if ((tag == TypeTag::Struct && !isCopyable()) || tag == TypeTag::Interface) {
        return PointerType::get(getType());
    } else {
        return getType();
    }
}

unsigned TypeDecl::getFieldIndex(llvm::StringRef fieldName) const {
    for (auto p : llvm::enumerate(fields)) {
        if (p.value().getName() == fieldName) {
            return static_cast<unsigned>(p.index());
        }
    }
    llvm_unreachable("unknown field");
}

TypeDecl* TypeTemplate::instantiate(const llvm::StringMap<Type>& genericArgs) {
    ASSERT(!genericParams.empty() && !genericArgs.empty());

    auto orderedGenericArgs = map(getGenericParams(),
                                  [&](const GenericParamDecl& genericParam) { return genericArgs.find(genericParam.getName())->second; });

    auto it = instantiations.find(orderedGenericArgs);
    if (it != instantiations.end()) return it->second;

    auto instantiation = llvm::cast<TypeDecl>(getTypeDecl()->instantiate(genericArgs, orderedGenericArgs));
    return instantiations.emplace(std::move(orderedGenericArgs), instantiation).first->second;
}

TypeDecl* TypeTemplate::instantiate(llvm::ArrayRef<Type> genericArgs) {
    ASSERT(genericArgs.size() == genericParams.size());
    llvm::StringMap<Type> genericArgsMap;

    for (auto&& [genericArg, genericParam] : llvm::zip_first(genericArgs, genericParams)) {
        genericArgsMap[genericParam.getName()] = genericArg;
    }

    return instantiate(genericArgsMap);
}

EnumCase::EnumCase(std::string&& name, Expr* value, AccessLevel accessLevel, SourceLocation location)
: VariableDecl(DeclKind::EnumCase, accessLevel, nullptr, Type() /* initialized by EnumDecl constructor */), name(std::move(name)),
  value(value), location(location) {}

EnumCase* EnumDecl::getCaseByName(llvm::StringRef name) {
    for (auto& enumCase : cases) {
        if (enumCase.getName() == name) {
            return &enumCase;
        }
    }
    return nullptr;
}

std::string FieldDecl::getQualifiedName() const {
    return (llvm::cast<TypeDecl>(getParent())->getQualifiedName() + "." + getName()).str();
}

bool Decl::hasBeenMoved() const {
    switch (getKind()) {
        case DeclKind::ParamDecl:
            return llvm::cast<ParamDecl>(this)->isMoved();
        case DeclKind::VarDecl:
            return llvm::cast<VarDecl>(this)->isMoved();
        default:
            return false;
    }
}

Decl* Decl::instantiate(const llvm::StringMap<Type>& genericArgs, llvm::ArrayRef<Type> genericArgsArray) const {
    switch (getKind()) {
        case DeclKind::ParamDecl:
            llvm_unreachable("handled in FunctionProto::instantiate()");

        case DeclKind::GenericParamDecl:
            llvm_unreachable("cannot instantiate GenericParamDecl");

        case DeclKind::FunctionDecl:
            llvm_unreachable("handled by FunctionDecl::instantiate()");

        case DeclKind::MethodDecl:
        case DeclKind::InitDecl:
        case DeclKind::DeinitDecl:
            llvm_unreachable("handled via TypeDecl");

        case DeclKind::FunctionTemplate:
            llvm_unreachable("handled via FunctionTemplate::instantiate()");

        case DeclKind::TypeDecl: {
            auto* typeDecl = llvm::cast<TypeDecl>(this);
            auto interfaces = map(typeDecl->getInterfaces(), [&](Type type) { return type.resolve(genericArgs); });
            auto instantiation = new TypeDecl(typeDecl->getTag(), typeDecl->getName(), genericArgsArray, std::move(interfaces),
                                              getAccessLevel(), *typeDecl->getModule(), typeDecl->getLocation());
            for (auto& field : typeDecl->getFields()) {
                instantiation->addField(FieldDecl(field.getType().resolve(genericArgs), field.getName(), *instantiation,
                                                  field.getAccessLevel(), field.getLocation()));
            }

            for (auto& method : typeDecl->getMethods()) {
                if (auto* nonTemplateMethod = llvm::dyn_cast<MethodDecl>(method)) {
                    instantiation->addMethod(nonTemplateMethod->instantiate(genericArgs, *instantiation));
                } else {
                    auto* functionTemplate = llvm::cast<FunctionTemplate>(method);
                    auto* methodDecl = llvm::cast<MethodDecl>(functionTemplate->getFunctionDecl());
                    auto methodInstantiation = methodDecl->MethodDecl::instantiate(genericArgs, *instantiation);

                    std::vector<GenericParamDecl> genericParams;
                    genericParams.reserve(functionTemplate->getGenericParams().size());

                    for (auto& genericParam : functionTemplate->getGenericParams()) {
                        genericParams.emplace_back(genericParam.getName().str(), genericParam.getLocation());
                        genericParams.back().setConstraints(genericParam.getConstraints());
                    }

                    auto accessLevel = methodInstantiation->getAccessLevel();
                    instantiation->addMethod(new FunctionTemplate(std::move(genericParams), methodInstantiation, accessLevel));
                }
            }

            return instantiation;
        }
        case DeclKind::TypeTemplate:
            llvm_unreachable("handled via TypeTemplate::instantiate()");

        case DeclKind::EnumDecl:
        case DeclKind::EnumCase:
            llvm_unreachable("EnumDecl cannot be generic or declared inside another generic type");

        case DeclKind::VarDecl: {
            auto* varDecl = llvm::cast<VarDecl>(this);
            auto type = varDecl->getType().resolve(genericArgs);
            auto initializer = varDecl->getInitializer() ? varDecl->getInitializer()->instantiate(genericArgs) : nullptr;
            return new VarDecl(type, varDecl->getName(), initializer, varDecl->getParent(), getAccessLevel(), *varDecl->getModule(),
                               varDecl->getLocation());
        }
        case DeclKind::FieldDecl:
            llvm_unreachable("handled via TypeDecl");

        case DeclKind::ImportDecl:
            llvm_unreachable("cannot instantiate ImportDecl");
    }
    llvm_unreachable("all cases handled");
}
