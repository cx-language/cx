#include "decl.h"
#include <algorithm>
#include <unordered_set>
#pragma warning(push, 0)
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/ErrorHandling.h>
#pragma warning(pop)
#include "arena.h"
#include "ast.h"
#include "module.h"

using namespace cx;

FunctionProto FunctionProto::instantiate(const llvm::StringMap<Type>& genericArgs) const {
    auto params = instantiateParams(this->params, genericArgs);
    auto returnType = this->returnType.resolve(genericArgs);
    std::vector<GenericParamDecl> genericParams;
    return FunctionProto(std::string(name), std::move(params), returnType, varArg, external);
}

FunctionDecl* FunctionTemplate::instantiate(const llvm::StringMap<Type>& genericArgs) {
    ASSERT(!genericParams.empty() && !genericArgs.empty());

    auto orderedGenericArgs = map(genericParams, [&](auto& genericParam) { return genericArgs.find(genericParam.getName())->second; });

    auto it = instantiations.find(orderedGenericArgs);
    if (it != instantiations.end()) return it->second;
    auto instantiation = functionDecl->instantiate(genericArgs, orderedGenericArgs);
    return instantiations.emplace(std::move(orderedGenericArgs), instantiation).first->second;
}

static std::optional<Location> findBreakTargetingPackLoop(llvm::ArrayRef<Stmt*> stmts) {
    for (Stmt* stmt : stmts) {
        switch (stmt->kind) {
        case StmtKind::BreakStmt:
            return llvm::cast<BreakStmt>(stmt)->location;
        case StmtKind::IfStmt: {
            auto* ifStmt = llvm::cast<IfStmt>(stmt);
            if (auto loc = findBreakTargetingPackLoop(ifStmt->thenBody)) return loc;
            if (auto loc = findBreakTargetingPackLoop(ifStmt->elseBody)) return loc;
            break;
        }
        case StmtKind::CompoundStmt:
            if (auto loc = findBreakTargetingPackLoop(llvm::cast<CompoundStmt>(stmt)->body)) return loc;
            break;
        default:
            break;
        }
    }
    return std::nullopt;
}

static std::optional<Location> findContinueTargetingPackLoop(llvm::ArrayRef<Stmt*> stmts) {
    for (Stmt* stmt : stmts) {
        switch (stmt->kind) {
        case StmtKind::ContinueStmt:
            return llvm::cast<ContinueStmt>(stmt)->location;
        case StmtKind::IfStmt: {
            auto* ifStmt = llvm::cast<IfStmt>(stmt);
            if (auto loc = findContinueTargetingPackLoop(ifStmt->thenBody)) return loc;
            if (auto loc = findContinueTargetingPackLoop(ifStmt->elseBody)) return loc;
            break;
        }
        case StmtKind::SwitchStmt: {
            auto* switchStmt = llvm::cast<SwitchStmt>(stmt);
            for (auto& switchCase : switchStmt->cases) {
                if (auto loc = findContinueTargetingPackLoop(switchCase.stmts)) return loc;
            }
            if (auto loc = findContinueTargetingPackLoop(switchStmt->defaultStmts)) return loc;
            break;
        }
        case StmtKind::CompoundStmt:
            if (auto loc = findContinueTargetingPackLoop(llvm::cast<CompoundStmt>(stmt)->body)) return loc;
            break;
        default:
            break;
        }
    }
    return std::nullopt;
}

static std::vector<Stmt*> unrollPackLoops(llvm::ArrayRef<Stmt*> stmts, llvm::StringRef packName, llvm::ArrayRef<std::string> expandedNames,
                                          FunctionDecl* parentFunc, Module& module, bool packShadowed) {
    std::vector<Stmt*> result;
    bool shadowedHere = packShadowed;

    for (Stmt* stmt : stmts) {
        switch (stmt->kind) {
        case StmtKind::VarStmt: {
            result.push_back(stmt);
            if (llvm::cast<VarStmt>(stmt)->decl->getName() == packName) shadowedHere = true;
            break;
        }
        case StmtKind::ForEachStmt: {
            auto* forEach = llvm::cast<ForEachStmt>(stmt);
            auto* rangeVar = llvm::dyn_cast<VarExpr>(forEach->range);
            bool isPackLoop = !shadowedHere && rangeVar && rangeVar->identifier == packName;

            if (!isPackLoop) {
                bool nestedShadowed = shadowedHere || forEach->variable->getName() == packName;
                forEach->body = unrollPackLoops(forEach->body, packName, expandedNames, parentFunc, module, nestedShadowed);
                result.push_back(forEach);
                break;
            }

            if (auto loc = findBreakTargetingPackLoop(forEach->body)) {
                ERROR(*loc, "break cannot be used in a loop over a variadic parameter");
            }
            if (auto loc = findContinueTargetingPackLoop(forEach->body)) {
                ERROR(*loc, "continue cannot be used in a loop over a variadic parameter");
            }

            bool loopVarShadowsPack = forEach->variable->getName() == packName;
            for (const std::string& expandedName : expandedNames) {
                auto clonedBody = ::cx::instantiate(forEach->body, llvm::StringMap<Type>());
                clonedBody = unrollPackLoops(clonedBody, packName, expandedNames, parentFunc, module, loopVarShadowsPack);

                std::vector<Stmt*> iteration;
                auto* loopVar = makeAST<VarDecl>(Type(), forEach->variable->getName().str(), makeAST<VarExpr>(std::string(expandedName), forEach->location),
                                                 parentFunc, AccessLevel::None, module, forEach->variable->getLocation());
                iteration.push_back(makeAST<VarStmt>(loopVar));
                for (Stmt* cloned : clonedBody)
                    iteration.push_back(cloned);
                result.push_back(makeAST<CompoundStmt>(std::move(iteration)));
            }
            break;
        }
        case StmtKind::IfStmt: {
            auto* ifStmt = llvm::cast<IfStmt>(stmt);
            ifStmt->thenBody = unrollPackLoops(ifStmt->thenBody, packName, expandedNames, parentFunc, module, shadowedHere);
            ifStmt->elseBody = unrollPackLoops(ifStmt->elseBody, packName, expandedNames, parentFunc, module, shadowedHere);
            result.push_back(ifStmt);
            break;
        }
        case StmtKind::SwitchStmt: {
            auto* switchStmt = llvm::cast<SwitchStmt>(stmt);
            for (auto& switchCase : switchStmt->cases) {
                bool nestedShadowed = shadowedHere || (switchCase.associatedValue && switchCase.associatedValue->getName() == packName);
                switchCase.stmts = unrollPackLoops(switchCase.stmts, packName, expandedNames, parentFunc, module, nestedShadowed);
            }
            switchStmt->defaultStmts = unrollPackLoops(switchStmt->defaultStmts, packName, expandedNames, parentFunc, module, shadowedHere);
            result.push_back(switchStmt);
            break;
        }
        case StmtKind::WhileStmt: {
            auto* whileStmt = llvm::cast<WhileStmt>(stmt);
            whileStmt->body = unrollPackLoops(whileStmt->body, packName, expandedNames, parentFunc, module, shadowedHere);
            result.push_back(whileStmt);
            break;
        }
        case StmtKind::ForStmt: {
            auto* forStmt = llvm::cast<ForStmt>(stmt);
            bool nestedShadowed = shadowedHere || (forStmt->variable && forStmt->variable->decl->getName() == packName);
            forStmt->body = unrollPackLoops(forStmt->body, packName, expandedNames, parentFunc, module, nestedShadowed);
            result.push_back(forStmt);
            break;
        }
        case StmtKind::CompoundStmt: {
            auto* compound = llvm::cast<CompoundStmt>(stmt);
            compound->body = unrollPackLoops(compound->body, packName, expandedNames, parentFunc, module, shadowedHere);
            result.push_back(compound);
            break;
        }
        default:
            result.push_back(stmt);
            break;
        }
    }

    return result;
}

FunctionDecl* FunctionTemplate::instantiateVariadic(const llvm::StringMap<Type>& fixedArgs, const std::vector<llvm::StringMap<Type>>& packArgs,
                                                    std::vector<Type>&& cacheKey) {
    auto it = instantiations.find(cacheKey);
    if (it != instantiations.end()) return it->second;

    ASSERT(functionDecl->hasPack());
    ASSERT(!functionDecl->isMethodDecl() || functionDecl->kind == DeclKind::MethodDecl);
    auto templateParams = functionDecl->getParams();
    llvm::StringRef packName = templateParams.back().getName();
    Type packType = templateParams.back().type;
    Location packLoc = templateParams.back().getLocation();

    std::vector<ParamDecl> expandedParams;
    expandedParams.reserve(templateParams.size() - 1 + packArgs.size());
    for (const ParamDecl& param : templateParams.drop_back()) {
        expandedParams.emplace_back(param.type.resolve(fixedArgs), param.getName().str(), param.isPublic, param.getLocation());
    }

    std::unordered_set<std::string> usedNames;
    for (const ParamDecl& param : expandedParams)
        usedNames.insert(param.getName().str());

    std::vector<std::string> expandedNames;
    expandedNames.reserve(packArgs.size());
    for (size_t i = 0; i < packArgs.size(); ++i) {
        llvm::StringMap<Type> combined = fixedArgs;
        for (auto& entry : packArgs[i])
            combined[entry.getKey()] = entry.getValue();
        Type resolved = packType.resolve(combined);
        std::string name = packName.str() + "_" + std::to_string(i);
        while (!usedNames.insert(name).second)
            name += "_";
        expandedNames.push_back(name);
        expandedParams.emplace_back(resolved, std::string(expandedNames.back()), false, packLoc);
    }

    Type returnType = functionDecl->getReturnType().resolve(fixedArgs);
    FunctionProto proto(functionDecl->getName().str(), std::move(expandedParams), returnType, false, false);

    FunctionDecl* instantiation;
    if (auto* methodDecl = llvm::dyn_cast<MethodDecl>(functionDecl)) {
        instantiation =
            makeAST<MethodDecl>(std::move(proto), *methodDecl->typeDecl, std::vector<Type>(cacheKey), methodDecl->accessLevel, methodDecl->getLocation());
    } else {
        instantiation = makeAST<FunctionDecl>(std::move(proto), std::vector<Type>(cacheKey), functionDecl->accessLevel, *functionDecl->getModule(),
                                              functionDecl->getLocation());
    }

    if (functionDecl->body) {
        auto clonedBody = ::cx::instantiate(*functionDecl->body, fixedArgs);
        instantiation->body = unrollPackLoops(clonedBody, packName, expandedNames, instantiation, *instantiation->getModule(), false);
    }
    instantiation->isPackInstantiation = true;
    return instantiations.emplace(std::move(cacheKey), instantiation).first->second;
}

std::string cx::getQualifiedFunctionName(Type receiver, llvm::StringRef name, llvm::ArrayRef<Type> genericArgs) {
    std::string result;

    if (receiver) {
        result = receiver.toString();
        result += '.';
    }

    result += name;
    appendGenericArgs(result, genericArgs);
    return result;
}

std::string FunctionDecl::getQualifiedName() const {
    Type receiver = getTypeDecl() ? getTypeDecl()->getType() : Type();
    return getQualifiedFunctionName(receiver, getName(), genericArgs);
}

FunctionType* FunctionDecl::getFunctionType() const {
    auto paramTypes = map(getParams(), [](const ParamDecl& p) -> Type { return p.type; });
    return &llvm::cast<FunctionType>(*FunctionType::get(getReturnType(), std::move(paramTypes), isVariadic()));
}

bool FunctionDecl::signatureMatches(const FunctionDecl& other, bool matchReceiver) const {
    if (getName() != other.getName()) return false;
    if (matchReceiver && getTypeDecl() != other.getTypeDecl()) return false;
    if (getReturnType() != other.getReturnType()) return false;
    // Parameter names only matter when they're public (used as argument labels); mirror paramsMatch in module.h.
    auto params = getParams(), otherParams = other.getParams();
    if (params.size() != otherParams.size()) return false;
    return std::equal(params.begin(), params.end(), otherParams.begin(), [](const ParamDecl& a, const ParamDecl& b) {
        if (a.type != b.type) return false;
        if (a.isPack != b.isPack) return false;
        if (a.isPublic && b.isPublic && a.getName() != b.getName()) return false;
        return true;
    });
}

FunctionDecl* FunctionDecl::instantiate(const llvm::StringMap<Type>& genericArgs, llvm::ArrayRef<Type> genericArgsArray) {
    if (auto methodDecl = llvm::dyn_cast<MethodDecl>(this)) {
        return methodDecl->instantiate(genericArgs, genericArgsArray, *getTypeDecl());
    } else {
        auto proto = this->proto.instantiate(genericArgs);
        auto instantiation = makeAST<FunctionDecl>(std::move(proto), genericArgsArray, accessLevel, module, location);
        instantiation->body = ::instantiate(*body, genericArgs);
        return instantiation;
    }
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

MethodDecl::MethodDecl(DeclKind kind, FunctionProto proto, TypeDecl& typeDecl, std::vector<Type>&& genericArgs, AccessLevel accessLevel, Location location)
: FunctionDecl(kind, std::move(proto), std::move(genericArgs), accessLevel, *typeDecl.getModule(), location), typeDecl(&typeDecl) {
    for (auto& param : getParams()) {
        param.parent = this;
    }
}

MethodDecl* MethodDecl::instantiate(const llvm::StringMap<Type>& genericArgs, llvm::ArrayRef<Type> genericArgsArray, TypeDecl& typeDecl) {
    switch (kind) {
    case DeclKind::MethodDecl: {
        auto* methodDecl = llvm::cast<MethodDecl>(this);
        auto proto = methodDecl->proto.instantiate(genericArgs);
        auto instantiation = makeAST<MethodDecl>(std::move(proto), typeDecl, genericArgsArray, accessLevel, methodDecl->getLocation());
        if (methodDecl->body) {
            instantiation->body = ::instantiate(*methodDecl->body, genericArgs);
        }
        return instantiation;
    }
    case DeclKind::ConstructorDecl: {
        auto* constructorDecl = llvm::cast<ConstructorDecl>(this);
        auto params = instantiateParams(constructorDecl->getParams(), genericArgs);
        auto instantiation = makeAST<ConstructorDecl>(typeDecl, std::move(params), accessLevel, constructorDecl->getLocation());
        instantiation->body = ::instantiate(*constructorDecl->body, genericArgs);
        return instantiation;
    }
    case DeclKind::DestructorDecl: {
        auto* destructorDecl = llvm::cast<DestructorDecl>(this);
        auto instantiation = makeAST<DestructorDecl>(typeDecl, destructorDecl->getLocation());
        instantiation->body = ::instantiate(*destructorDecl->body, genericArgs);
        return instantiation;
    }
    default:
        llvm_unreachable("invalid method decl");
    }
}

FieldDecl FieldDecl::instantiate(const llvm::StringMap<Type>& genericArgs, TypeDecl& typeDecl) const {
    auto type = this->type.resolve(genericArgs);
    auto defaultValue = this->defaultValue ? this->defaultValue->instantiate(genericArgs) : nullptr;
    return FieldDecl(type, getName().str(), defaultValue, typeDecl, accessLevel, location);
}

std::vector<ParamDecl> cx::instantiateParams(llvm::ArrayRef<ParamDecl> params, const llvm::StringMap<Type>& genericArgs) {
    return map(params, [&](const ParamDecl& param) {
        ParamDecl result(param.type.resolve(genericArgs), param.getName().str(), param.isPublic, param.getLocation());
        result.isPack = param.isPack;
        result.defaultValue = param.defaultValue ? param.defaultValue->instantiate(genericArgs) : nullptr;
        return result;
    });
}

std::string TypeDecl::getQualifiedName() const {
    return getQualifiedTypeName(getName(), genericArgs);
}

bool TypeDecl::hasInterface(const TypeDecl& interface) const {
    return llvm::any_of(interfaces, [&](Type type) { return type.getDecl() == &interface; });
}

bool TypeDecl::isCopyable() const {
    if (name == "Optional") return genericArgs.front().isImplicitlyCopyable();
    return llvm::any_of(interfaces, [&](Type type) { return type.getName() == "Copyable"; });
}

void TypeDecl::addField(FieldDecl&& field) {
    fields.emplace_back(std::move(field));
}

void TypeDecl::addMethod(Decl* decl) {
    methods.push_back(decl);
}

ConstructorDecl* TypeDecl::addAutogeneratedConstructor() {
    std::vector<ParamDecl> params;
    std::vector<ParamDecl> defaultedParams;
    std::vector<Stmt*> body;

    for (auto& field : fields) {
        llvm::StringRef paramName = field.getName();
        auto* left = makeAST<MemberExpr>(makeAST<VarExpr>("this", field.getLocation()), field.getName().str(), field.getLocation());
        auto* right = makeAST<VarExpr>(paramName.str(), field.getLocation());
        body.push_back(makeAST<ExprStmt>(makeAST<BinaryExpr>(Token::Assignment, left, right, field.getLocation())));

        ParamDecl param(field.type, paramName.str(), false, field.getLocation());
        if (field.defaultValue) {
            param.defaultValue = field.defaultValue;
            defaultedParams.push_back(std::move(param));
        } else {
            params.push_back(std::move(param));
        }
    }

    // Defaulted params come last so calls can omit them positionally.
    for (auto& param : defaultedParams) {
        params.push_back(std::move(param));
    }

    auto* autogeneratedInit = makeAST<ConstructorDecl>(*this, std::move(params), accessLevel, getLocation());
    autogeneratedInit->body = std::move(body);
    addMethod(autogeneratedInit);
    return autogeneratedInit;
}

std::vector<ConstructorDecl*> TypeDecl::getConstructors() const {
    std::vector<ConstructorDecl*> constructors;

    for (auto& decl : methods) {
        if (auto* constructorDecl = llvm::dyn_cast<ConstructorDecl>(decl)) {
            constructors.push_back(constructorDecl);
        }
    }

    return constructors;
}

DestructorDecl* TypeDecl::getDestructor() const {
    for (auto& decl : methods) {
        if (auto* destructorDecl = llvm::dyn_cast<DestructorDecl>(decl)) {
            return destructorDecl;
        }
    }
    return nullptr;
}

Type TypeDecl::getType(Mutability mutability) const {
    return BasicType::get(name, genericArgs, mutability, location);
}

Type TypeDecl::getTypeForPassing() const {
    if ((tag == TypeTag::Struct && !isCopyable()) || tag == TypeTag::Interface) {
        return PointerType::get(getType()).withLocation(location);
    } else {
        return getType();
    }
}

unsigned TypeDecl::getFieldIndex(const FieldDecl* field) const {
    for (const auto& p : llvm::enumerate(fields)) {
        if (&p.value() == field) {
            return static_cast<unsigned>(p.index());
        }
    }
    llvm_unreachable("unknown field");
}

TypeDecl* TypeTemplate::instantiate(const llvm::StringMap<Type>& genericArgs) {
    ASSERT(!genericParams.empty() && !genericArgs.empty());
    auto orderedGenericArgs = map(genericParams, [&](auto& genericParam) { return genericArgs.find(genericParam.getName())->second; });

    auto it = instantiations.find(orderedGenericArgs);
    if (it != instantiations.end()) return it->second;

    auto instantiation = llvm::cast<TypeDecl>(typeDecl->instantiate(genericArgs, orderedGenericArgs));
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

EnumCase::EnumCase(std::string&& name, Expr* value, Type associatedType, AccessLevel accessLevel, Location location)
: VariableDecl(DeclKind::EnumCase, accessLevel, nullptr, Type() /* initialized by EnumDecl constructor */), name(std::move(name)), value(value),
  associatedType(associatedType), location(location) {}

EnumCase* EnumDecl::getCaseByName(llvm::StringRef name) {
    for (auto& enumCase : cases) {
        if (enumCase.getName() == name) {
            return &enumCase;
        }
    }
    return nullptr;
}

bool EnumDecl::hasAssociatedValues() const {
    for (auto& enumCase : cases) {
        if (enumCase.associatedType) {
            return true;
        }
    }
    return false;
}

FieldDecl::FieldDecl(Type type, std::string&& name, Expr* defaultValue, TypeDecl& parent, AccessLevel accessLevel, Location location)
: VariableDecl(DeclKind::FieldDecl, accessLevel, &parent, type), name(std::move(name)), defaultValue(defaultValue), location(location) {}

Module* FieldDecl::getModule() const {
    return getParentDecl()->getModule();
}

std::string FieldDecl::getQualifiedName() const {
    return (llvm::cast<TypeDecl>(getParentDecl())->getQualifiedName() + "." + getName()).str();
}

bool Decl::hasBeenMoved() const {
    switch (kind) {
    case DeclKind::ParamDecl:
        return llvm::cast<ParamDecl>(this)->moved;
    case DeclKind::VarDecl:
        return llvm::cast<VarDecl>(this)->moved;
    default:
        return false;
    }
}

// TODO: Ensure that the same decl isn't instantiated multiple times with same generic args, to avoid duplicate work.
Decl* Decl::instantiate(const llvm::StringMap<Type>& genericArgs, llvm::ArrayRef<Type> genericArgsArray) const {
    switch (kind) {
    case DeclKind::ParamDecl:
        llvm_unreachable("handled in FunctionProto::instantiate()");

    case DeclKind::GenericParamDecl:
        llvm_unreachable("cannot instantiate GenericParamDecl");

    case DeclKind::FunctionDecl:
        llvm_unreachable("handled by FunctionDecl::instantiate()");

    case DeclKind::MethodDecl:
    case DeclKind::ConstructorDecl:
    case DeclKind::DestructorDecl:
        llvm_unreachable("handled via TypeDecl");

    case DeclKind::FunctionTemplate:
        llvm_unreachable("handled via FunctionTemplate::instantiate()");

    case DeclKind::TypeDecl: {
        auto* typeDecl = llvm::cast<TypeDecl>(this);
        auto interfaces = map(typeDecl->interfaces, [&](Type type) { return type.resolve(genericArgs); });
        auto instantiation = makeAST<TypeDecl>(typeDecl->tag, typeDecl->getName().str(), genericArgsArray, std::move(interfaces), accessLevel,
                                               *typeDecl->getModule(), typeDecl, typeDecl->getLocation());
        for (auto& field : typeDecl->fields) {
            auto defaultValue = field.defaultValue ? field.defaultValue->instantiate(genericArgs) : nullptr;
            instantiation->addField(
                FieldDecl(field.type.resolve(genericArgs), field.getName().str(), defaultValue, *instantiation, field.accessLevel, field.getLocation()));
        }

        for (auto& method : typeDecl->methods) {
            if (auto* nonTemplateMethod = llvm::dyn_cast<MethodDecl>(method)) {
                instantiation->addMethod(nonTemplateMethod->instantiate(genericArgs, {}, *instantiation));
            } else {
                auto* functionTemplate = llvm::cast<FunctionTemplate>(method);
                auto* methodDecl = llvm::cast<MethodDecl>(functionTemplate->functionDecl);
                auto methodInstantiation = methodDecl->instantiate(genericArgs, {}, *instantiation);

                std::vector<GenericParamDecl> genericParams;
                genericParams.reserve(functionTemplate->genericParams.size());

                for (auto& genericParam : functionTemplate->genericParams) {
                    genericParams.emplace_back(genericParam.getName().str(), genericParam.getLocation());
                    genericParams.back().constraints = genericParam.constraints;
                }

                auto accessLevel = methodInstantiation->accessLevel;
                instantiation->addMethod(makeAST<FunctionTemplate>(std::move(genericParams), methodInstantiation, accessLevel));
            }
        }

        return instantiation;
    }
    case DeclKind::TypeTemplate:
        llvm_unreachable("handled via TypeTemplate::instantiate()");

    case DeclKind::EnumDecl: {
        auto* enumDecl = llvm::cast<EnumDecl>(this);
        std::vector<EnumCase> cases;
        for (auto& enumCase : enumDecl->cases) {
            cases.emplace_back(enumCase.getName().str(), enumCase.value ? enumCase.value->instantiate(genericArgs) : nullptr,
                               enumCase.associatedType.resolve(genericArgs), enumCase.accessLevel, enumCase.getLocation());
        }
        auto instantiation =
            makeAST<EnumDecl>(enumDecl->getName().str(), std::move(cases), accessLevel, *enumDecl->getModule(), enumDecl, enumDecl->getLocation());
        instantiation->genericArgs = std::vector<Type>(genericArgsArray.begin(), genericArgsArray.end());
        for (auto& enumCase : instantiation->cases) {
            enumCase.type = NOTNULL(instantiation->getType());
        }
        return instantiation;
    }
    case DeclKind::EnumCase:
        llvm_unreachable("handled via EnumDecl");

    case DeclKind::VarDecl: {
        auto* varDecl = llvm::cast<VarDecl>(this);
        auto type = varDecl->type.resolve(genericArgs);
        auto initializer = varDecl->initializer ? varDecl->initializer->instantiate(genericArgs) : nullptr;
        return makeAST<VarDecl>(type, varDecl->getName().str(), initializer, varDecl->parent, accessLevel, *varDecl->getModule(), varDecl->getLocation());
    }
    case DeclKind::FieldDecl:
        llvm_unreachable("handled via TypeDecl");

    case DeclKind::ImportDecl:
        llvm_unreachable("cannot instantiate ImportDecl");
    }
    llvm_unreachable("all cases handled");
}

bool Decl::isGlobal() const {
    if (auto variableDecl = llvm::dyn_cast<VariableDecl>(this)) {
        return variableDecl->parent == nullptr;
    }
    return true;
}

ConstructorDecl::ConstructorDecl(TypeDecl& receiverTypeDecl, std::vector<ParamDecl>&& params, AccessLevel accessLevel, Location location)
: MethodDecl(DeclKind::ConstructorDecl, FunctionProto("init", std::move(params), Type::getVoid(), false, false), receiverTypeDecl, {}, accessLevel, location) {}

DestructorDecl::DestructorDecl(TypeDecl& receiverTypeDecl, Location location)
: MethodDecl(DeclKind::DestructorDecl, FunctionProto("deinit", {}, Type::getVoid(), false, false), receiverTypeDecl, {}, AccessLevel::None, location) {}

std::vector<Note> cx::getPreviousDefinitionNotes(llvm::ArrayRef<Decl*> decls) {
    return map(decls, [](Decl* decl) { return Note{decl->getLocation(), "previous definition here"}; });
}
