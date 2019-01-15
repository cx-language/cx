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

    auto orderedGenericArgs = map(genericParams, [&](const GenericParamDecl& genericParam) {
        return genericArgs.find(genericParam.getName())->second;
    });

    auto it = instantiations.find(orderedGenericArgs);
    if (it != instantiations.end()) return it->second.get();
    auto instantiation = getFunctionDecl()->instantiate(genericArgs, orderedGenericArgs);
    return instantiations.emplace(std::move(orderedGenericArgs), std::move(instantiation)).first->second.get();
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
    if (isMutating() != other.isMutating()) return false;
    if (getReturnType() != other.getReturnType()) return false;
    if (getParams() != other.getParams()) return false;
    return true;
}

std::unique_ptr<FunctionDecl> FunctionDecl::instantiate(const llvm::StringMap<Type>& genericArgs,
                                                        llvm::ArrayRef<Type> genericArgsArray) {
    auto proto = getProto().instantiate(genericArgs);
    auto body = ::instantiate(getBody(), genericArgs);

    std::unique_ptr<FunctionDecl> instantiation;

    if (isMethodDecl()) {
        instantiation = llvm::make_unique<MethodDecl>(std::move(proto), *getTypeDecl(), genericArgsArray,
                                                      getAccessLevel(), location);
        llvm::cast<MethodDecl>(*instantiation).setMutating(isMutating());
    } else {
        instantiation = llvm::make_unique<FunctionDecl>(std::move(proto), genericArgsArray, getAccessLevel(), module,
                                                        location);
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

MethodDecl::MethodDecl(DeclKind kind, FunctionProto proto, TypeDecl& typeDecl, std::vector<Type>&& genericArgs,
                       AccessLevel accessLevel, SourceLocation location)
: FunctionDecl(kind, std::move(proto), std::move(genericArgs), accessLevel, *typeDecl.getModule(), location),
  typeDecl(&typeDecl), mutating(false) {}

std::unique_ptr<MethodDecl> MethodDecl::instantiate(const llvm::StringMap<Type>& genericArgs, TypeDecl& typeDecl) {
    switch (getKind()) {
        case DeclKind::MethodDecl: {
            auto* methodDecl = llvm::cast<MethodDecl>(this);
            auto proto = methodDecl->getProto().instantiate(genericArgs);
            auto instantiation = llvm::make_unique<MethodDecl>(std::move(proto), typeDecl, std::vector<Type>(),
                                                               getAccessLevel(), methodDecl->getLocation());
            instantiation->setMutating(methodDecl->isMutating());
            if (methodDecl->hasBody()) {
                instantiation->setBody(::instantiate(methodDecl->getBody(), genericArgs));
            }
            return instantiation;
        }
        case DeclKind::InitDecl: {
            auto* initDecl = llvm::cast<InitDecl>(this);
            auto params = instantiateParams(initDecl->getParams(), genericArgs);
            auto instantiation = llvm::make_unique<InitDecl>(typeDecl, std::move(params), getAccessLevel(),
                                                             initDecl->getLocation());
            instantiation->setBody(::instantiate(initDecl->getBody(), genericArgs));
            return std::move(instantiation);
        }
        case DeclKind::DeinitDecl: {
            auto* deinitDecl = llvm::cast<DeinitDecl>(this);
            auto instantiation = llvm::make_unique<DeinitDecl>(typeDecl, deinitDecl->getLocation());
            instantiation->setBody(::instantiate(deinitDecl->getBody(), genericArgs));
            return std::move(instantiation);
        }
        default:
            llvm_unreachable("invalid method decl");
    }
}

std::vector<ParamDecl> delta::instantiateParams(llvm::ArrayRef<ParamDecl> params, const llvm::StringMap<Type>& genericArgs) {
    return map(params, [&](const ParamDecl& param) {
        return ParamDecl(param.getType().resolve(genericArgs), param.getName(), param.getLocation());
    });
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

void TypeDecl::addMethod(std::unique_ptr<Decl> decl) {
    methods.emplace_back(std::move(decl));
}

DeinitDecl* TypeDecl::getDeinitializer() const {
    for (auto& decl : getMemberDecls()) {
        if (auto* deinitDecl = llvm::dyn_cast<DeinitDecl>(decl.get())) {
            return deinitDecl;
        }
    }
    return nullptr;
}

Type TypeDecl::getType(bool isMutable) const {
    return BasicType::get(name, genericArgs, isMutable);
}

Type TypeDecl::getTypeForPassing(bool isMutable) const {
    switch (tag) {
        case TypeTag::Struct:
            if (isCopyable()) {
                return getType(isMutable);
            } else {
                return PointerType::get(getType(isMutable));
            }
        case TypeTag::Union:
        case TypeTag::Enum:
            return getType(isMutable);
        case TypeTag::Interface:
            return PointerType::get(getType(isMutable));
    }
    llvm_unreachable("invalid type tag");
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

    auto orderedGenericArgs = map(getGenericParams(), [&](const GenericParamDecl& genericParam) {
        return genericArgs.find(genericParam.getName())->second;
    });

    auto it = instantiations.find(orderedGenericArgs);
    if (it != instantiations.end()) return it->second.get();

    auto instantiation = llvm::cast<TypeDecl>(getTypeDecl()->instantiate(genericArgs, orderedGenericArgs));
    return instantiations.emplace(std::move(orderedGenericArgs), std::move(instantiation)).first->second.get();
}

TypeDecl* TypeTemplate::instantiate(llvm::ArrayRef<Type> genericArgs) {
    ASSERT(genericArgs.size() == genericParams.size());
    llvm::StringMap<Type> genericArgsMap;

    for (auto t : llvm::zip_first(genericArgs, genericParams)) {
        genericArgsMap[std::get<1>(t).getName()] = std::get<0>(t);
    }

    return instantiate(genericArgsMap);
}

const EnumCase* EnumDecl::getCaseByName(llvm::StringRef name) const {
    for (auto& enumCase : cases) {
        if (enumCase.getName() == name) {
            return &enumCase;
        }
    }
    return nullptr;
}

std::string FieldDecl::getQualifiedName() const {
    return (getParent()->getQualifiedName() + "." + getName()).str();
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

std::unique_ptr<Decl> Decl::instantiate(const llvm::StringMap<Type>& genericArgs, llvm::ArrayRef<Type> genericArgsArray) const {
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
            auto instantiation = llvm::make_unique<TypeDecl>(typeDecl->getTag(), typeDecl->getName(), genericArgsArray,
                                                             std::move(interfaces), getAccessLevel(),
                                                             *typeDecl->getModule(), typeDecl->getLocation());
            for (auto& field : typeDecl->getFields()) {
                instantiation->addField(FieldDecl(field.getType().resolve(genericArgs), field.getName(), *instantiation,
                                                  field.getAccessLevel(), field.getLocation()));
            }

            for (auto& method : typeDecl->getMethods()) {
                if (auto* nonTemplateMethod = llvm::dyn_cast<MethodDecl>(method.get())) {
                    instantiation->addMethod(nonTemplateMethod->instantiate(genericArgs, *instantiation));
                } else {
                    auto* functionTemplate = llvm::cast<FunctionTemplate>(method.get());
                    auto* methodDecl = llvm::cast<MethodDecl>(functionTemplate->getFunctionDecl());
                    auto methodInstantiation = methodDecl->MethodDecl::instantiate(genericArgs, *instantiation);

                    std::vector<GenericParamDecl> genericParams;
                    genericParams.reserve(functionTemplate->getGenericParams().size());

                    for (auto& genericParam : functionTemplate->getGenericParams()) {
                        genericParams.emplace_back(genericParam.getName().str(), genericParam.getLocation());

                        for (Type constraint : genericParam.getConstraints()) {
                            genericParams.back().addConstraint(constraint);
                        }
                    }

                    auto accessLevel = methodInstantiation->getAccessLevel();
                    instantiation->addMethod(llvm::make_unique<FunctionTemplate>(
                        std::move(genericParams), std::move(methodInstantiation), accessLevel));
                }
            }

            return std::move(instantiation);
        }
        case DeclKind::TypeTemplate:
            llvm_unreachable("handled via TypeTemplate::instantiate()");

        case DeclKind::EnumDecl:
            llvm_unreachable("EnumDecl cannot be generic or declared inside another generic type");

        case DeclKind::VarDecl: {
            auto* varDecl = llvm::cast<VarDecl>(this);
            auto type = varDecl->getType().resolve(genericArgs);
            auto initializer = varDecl->getInitializer() ? varDecl->getInitializer()->instantiate(genericArgs) : nullptr;
            return llvm::make_unique<VarDecl>(type, varDecl->getName(), std::move(initializer), varDecl->getParent(),
                                              getAccessLevel(), *varDecl->getModule(), varDecl->getLocation());
        }
        case DeclKind::FieldDecl:
            llvm_unreachable("handled via TypeDecl");

        case DeclKind::ImportDecl:
            llvm_unreachable("cannot instantiate ImportDecl");
    }
    llvm_unreachable("all cases handled");
}
