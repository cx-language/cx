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

FunctionDecl* FunctionTemplate::instantiate(llvm::ArrayRef<Type> genericArgs) {
    ASSERT(genericArgs.size() == genericParams.size());
    llvm::StringMap<Type> genericArgsMap;

    for (auto t : llvm::zip_first(genericArgs, genericParams)) {
        genericArgsMap.try_emplace(std::get<1>(t).getName(), std::get<0>(t));
    }

    return instantiate(genericArgsMap);
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

    auto instantiation = llvm::make_unique<FunctionDecl>(std::move(proto), genericArgsArray,
                                                         module, location);
    instantiation->setBody(std::move(body));
    return instantiation;
}

MethodDecl::MethodDecl(DeclKind kind, FunctionProto proto, TypeDecl& typeDecl,
                       SourceLocation location)
: FunctionDecl(kind, std::move(proto), {}, *typeDecl.getModule(), location), typeDecl(&typeDecl),
  mutating(false) {}

Type MethodDecl::getThisType() const {
    if (getTypeDecl()->passByValue() && !isMutating()) {
        return getTypeDecl()->getType(isMutating());
    } else {
        return PointerType::get(getTypeDecl()->getType(isMutating()));
    }
}

MethodDecl* MethodDecl::instantiate(const llvm::StringMap<Type>& genericArgs, TypeDecl& typeDecl) {
    std::unique_ptr<MethodDecl> instantiation;

    switch (getKind()) {
        case DeclKind::MethodDecl: {
            auto* methodDecl = llvm::cast<MethodDecl>(this);
            auto proto = methodDecl->getProto().instantiate(genericArgs);
            instantiation = llvm::make_unique<MethodDecl>(std::move(proto), typeDecl,
                                                          methodDecl->getLocation());
            instantiation->setMutating(methodDecl->isMutating());
            instantiation->setBody(::instantiate(methodDecl->getBody(), genericArgs));
            break;
        }
        case DeclKind::InitDecl: {
            auto* initDecl = llvm::cast<InitDecl>(this);
            auto params = map(initDecl->getParams(), [&](const ParamDecl& param) {
                auto type = param.getType().resolve(genericArgs);
                return ParamDecl(type, param.getName(), param.getLocation());
            });
            auto body = ::instantiate(initDecl->getBody(), genericArgs);
            instantiation = llvm::make_unique<InitDecl>(typeDecl, std::move(params), std::move(body),
                                                        initDecl->getLocation());
            break;
        }
        case DeclKind::DeinitDecl: {
            auto* deinitDecl = llvm::cast<DeinitDecl>(this);
            auto body = ::instantiate(deinitDecl->getBody(), genericArgs);
            instantiation = llvm::make_unique<DeinitDecl>(typeDecl, std::move(body),
                                                          deinitDecl->getLocation());
            break;
        }
        default:
            llvm_unreachable("invalid method decl");
    }

    auto* pointer = instantiation.get();
    typeDecl.addMethod(std::move(instantiation));
    return llvm::cast<MethodDecl>(pointer);
}

void TypeDecl::addField(FieldDecl&& field) {
    fields.emplace_back(std::move(field));
}

void TypeDecl::addMethod(std::unique_ptr<MethodDecl> decl) {
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
    fatalError("unknown field");
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
        case DeclKind::TypeTemplate: return llvm::cast<TypeTemplate>(this)->getLocation();
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

void Decl::markAsMoved() {
    switch (getKind()) {
        case DeclKind::ParamDecl: llvm::cast<ParamDecl>(this)->setMoved(true); break;
        case DeclKind::VarDecl: llvm::cast<VarDecl>(this)->setMoved(true); break;
        default: break;
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

        case DeclKind::TypeDecl: {
            auto* typeDecl = llvm::cast<TypeDecl>(this);
            auto instantiation = llvm::make_unique<TypeDecl>(typeDecl->getTag(), typeDecl->getName(),
                                                             genericArgsArray, *typeDecl->getModule(),
                                                             typeDecl->getLocation());
            for (auto& field : typeDecl->getFields()) {
                instantiation->addField(FieldDecl(field.getType().resolve(genericArgs), field.getName(),
                                                  *instantiation, field.getLocation()));
            }

            for (auto& method : typeDecl->getMethods()) {
                method->instantiate(genericArgs, *instantiation);
            }

            return std::move(instantiation);
        }
        case DeclKind::VarDecl: {
            auto* varDecl = llvm::cast<VarDecl>(this);
            auto type = varDecl->getType().resolve(genericArgs);
            auto initializer = varDecl->getInitializer()
                ? varDecl->getInitializer()->instantiate(genericArgs) : nullptr;
            return llvm::make_unique<VarDecl>(type, varDecl->getName(), std::move(initializer),
                                              *varDecl->getModule(), varDecl->getLocation());
        }
        case DeclKind::FieldDecl:
            llvm_unreachable("handled via TypeDecl");

        case DeclKind::ImportDecl: {
            auto* importDecl = llvm::cast<ImportDecl>(this);
            return llvm::make_unique<ImportDecl>(importDecl->getTarget(), *importDecl->getModule(),
                                                 importDecl->getLocation());
        }
        default:
            llvm_unreachable("all cases handled");
    }
}
