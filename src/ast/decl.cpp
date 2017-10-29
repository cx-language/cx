#include <llvm/ADT/StringRef.h>
#include <llvm/Support/ErrorHandling.h>
#include "decl.h"
#include "mangle.h"
#include "../support/utility.h"

using namespace delta;

Module* Decl::getModule() const {
    switch (getKind()) {
        case DeclKind::ParamDecl: return llvm::cast<ParamDecl>(this)->getParent()->getModule();
        case DeclKind::GenericParamDecl: return llvm::cast<GenericParamDecl>(this)->getParent()->getModule();
        case DeclKind::FunctionDecl: return llvm::cast<FunctionDecl>(this)->getModule();
        case DeclKind::MethodDecl: return llvm::cast<MethodDecl>(this)->getTypeDecl()->getModule();
        case DeclKind::InitDecl: return llvm::cast<InitDecl>(this)->getTypeDecl()->getModule();
        case DeclKind::DeinitDecl: return llvm::cast<DeinitDecl>(this)->getTypeDecl()->getModule();
        case DeclKind::FunctionTemplate: return llvm::cast<FunctionTemplate>(this)->getFunctionDecl()->getModule();
        case DeclKind::TypeDecl: return llvm::cast<TypeDecl>(this)->getModule();
        case DeclKind::TypeTemplate: return llvm::cast<TypeTemplate>(this)->getModule();
        case DeclKind::VarDecl: return llvm::cast<VarDecl>(this)->getModule();
        case DeclKind::FieldDecl: return llvm::cast<FieldDecl>(this)->getParent()->getModule();
        case DeclKind::ImportDecl: return llvm::cast<ImportDecl>(this)->getModule();
    }
    llvm_unreachable("all cases handled");
}

FunctionProto FunctionProto::instantiate(const llvm::StringMap<Type> genericArgs) const {
    auto params = map(getParams(), [&](const ParamDecl& param) {
        auto type = param.getType().resolve(genericArgs);
        return ParamDecl(type, param.getName(), param.getLocation());
    });
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

const FunctionType* FunctionDecl::getFunctionType() const {
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
        instantiation = llvm::make_unique<MethodDecl>(std::move(proto), *getTypeDecl(),
                                                      genericArgsArray, location);
        llvm::cast<MethodDecl>(*instantiation).setMutating(isMutating());
    } else {
        instantiation = llvm::make_unique<FunctionDecl>(std::move(proto), genericArgsArray,
                                                        module, location);
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

MethodDecl::MethodDecl(DeclKind kind, FunctionProto proto, TypeDecl& typeDecl,
                       std::vector<Type>&& genericArgs, SourceLocation location)
: FunctionDecl(kind, std::move(proto), std::move(genericArgs), *typeDecl.getModule(), location),
  typeDecl(&typeDecl), mutating(false) {}

std::unique_ptr<MethodDecl> MethodDecl::instantiate(const llvm::StringMap<Type>& genericArgs, TypeDecl& typeDecl) {
    switch (getKind()) {
        case DeclKind::MethodDecl: {
            auto* methodDecl = llvm::cast<MethodDecl>(this);
            auto proto = methodDecl->getProto().instantiate(genericArgs);
            auto instantiation = llvm::make_unique<MethodDecl>(std::move(proto), typeDecl, std::vector<Type>(),
                                                               methodDecl->getLocation());
            instantiation->setMutating(methodDecl->isMutating());
            if (methodDecl->hasBody()) {
                instantiation->setBody(::instantiate(methodDecl->getBody(), genericArgs));
            }
            return instantiation;
        }
        case DeclKind::InitDecl: {
            auto* initDecl = llvm::cast<InitDecl>(this);
            auto params = map(initDecl->getParams(), [&](const ParamDecl& param) {
                auto type = param.getType().resolve(genericArgs);
                return ParamDecl(type, param.getName(), param.getLocation());
            });
            auto instantiation = llvm::make_unique<InitDecl>(typeDecl, std::move(params), initDecl->getLocation());
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
        case TypeTag::Struct: case TypeTag::Union:
            return getType(isMutable);
        case TypeTag::Class: case TypeTag::Interface:
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

SourceLocation Decl::getLocation() const {
    switch (getKind()) {
        case DeclKind::ParamDecl: return llvm::cast<ParamDecl>(this)->getLocation();
        case DeclKind::FunctionDecl:
        case DeclKind::MethodDecl:
        case DeclKind::InitDecl:
        case DeclKind::DeinitDecl: return llvm::cast<FunctionDecl>(this)->getLocation();
        case DeclKind::GenericParamDecl: return llvm::cast<GenericParamDecl>(this)->getLocation();
        case DeclKind::FunctionTemplate: return llvm::cast<FunctionTemplate>(this)->getFunctionDecl()->getLocation();
        case DeclKind::TypeDecl: return llvm::cast<TypeDecl>(this)->getLocation();
        case DeclKind::TypeTemplate: return llvm::cast<TypeTemplate>(this)->getTypeDecl()->getLocation();
        case DeclKind::VarDecl: return llvm::cast<VarDecl>(this)->getLocation();
        case DeclKind::FieldDecl: return llvm::cast<FieldDecl>(this)->getLocation();
        case DeclKind::ImportDecl: return llvm::cast<ImportDecl>(this)->getLocation();
    }
    llvm_unreachable("all cases handled");
}

bool Decl::hasBeenMoved() const {
    switch (getKind()) {
        case DeclKind::ParamDecl: return llvm::cast<ParamDecl>(this)->isMoved();
        case DeclKind::VarDecl: return llvm::cast<VarDecl>(this)->isMoved();
        default: return false;
    }
}

std::unique_ptr<Decl> Decl::instantiate(const llvm::StringMap<Type>& genericArgs,
                                        llvm::ArrayRef<Type> genericArgsArray) const {
    switch (getKind()) {
        case DeclKind::ParamDecl:
            llvm_unreachable("handled in FunctionProto::instantiate()");

        case DeclKind::GenericParamDecl:
            llvm_unreachable("cannot instantiate GenericParamDecl");

        case DeclKind::FunctionDecl:
            llvm_unreachable("handled by FunctionDecl::instantiate()");

        case DeclKind::MethodDecl:
            llvm_unreachable("handled via TypeDecl");

        case DeclKind::InitDecl:
            llvm_unreachable("handled via TypeDecl");

        case DeclKind::DeinitDecl:
            llvm_unreachable("handled via TypeDecl");

        case DeclKind::FunctionTemplate:
            llvm_unreachable("handled via FunctionTemplate::instantiate()");

        case DeclKind::TypeDecl: {
            auto* typeDecl = llvm::cast<TypeDecl>(this);
            auto interfaces = map(typeDecl->getInterfaces(), [&](Type type) {
                return type.resolve(genericArgs);
            });
            auto instantiation = llvm::make_unique<TypeDecl>(typeDecl->getTag(), typeDecl->getName(),
                                                             genericArgsArray, std::move(interfaces),
                                                             *typeDecl->getModule(), typeDecl->getLocation());
            for (auto& field : typeDecl->getFields()) {
                instantiation->addField(FieldDecl(field.getType().resolve(genericArgs), field.getName(),
                                                  *instantiation, field.getLocation()));
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

                        for (auto& constraint : genericParam.getConstraints()) {
                            genericParams.back().addConstraint(std::string(constraint));
                        }
                    }

                    auto p = llvm::make_unique<FunctionTemplate>(std::move(genericParams),
                                                                 std::move(methodInstantiation));
                    instantiation->addMethod(std::move(p));
                }
            }

            return std::move(instantiation);
        }
        case DeclKind::TypeTemplate:
            llvm_unreachable("handled via TypeTemplate::instantiate()");

        case DeclKind::VarDecl: {
            auto* varDecl = llvm::cast<VarDecl>(this);
            auto type = varDecl->getType().resolve(genericArgs);
            auto initializer = varDecl->getInitializer()
                ? varDecl->getInitializer()->instantiate(genericArgs) : nullptr;
            return llvm::make_unique<VarDecl>(type, varDecl->getName(), std::move(initializer),
                                              varDecl->getParent(), *varDecl->getModule(),
                                              varDecl->getLocation());
        }
        case DeclKind::FieldDecl:
            llvm_unreachable("handled via TypeDecl");

        case DeclKind::ImportDecl:
            llvm_unreachable("cannot instantiate ImportDecl");
    }
    llvm_unreachable("all cases handled");
}
