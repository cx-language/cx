#include "decl.h"
#include <algorithm>
#include <unordered_set>
#pragma warning(push, 0)
#include <llvm/ADT/STLExtras.h>
#include <llvm/ADT/ScopeExit.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/ErrorHandling.h>
#pragma warning(pop)
#include "arena.h"
#include "ast.h"
#include "module.h"

using namespace cx;

FunctionProto FunctionProto::instantiate(const llvm::StringMap<GenericArg>& genericArgs) const {
    auto params = instantiateParams(this->params, genericArgs);
    auto returnType = this->returnType.resolve(genericArgs);
    std::vector<GenericParamDecl> genericParams;
    return FunctionProto(name, std::move(params), returnType, varArg, external, cppLinkage);
}

FunctionDecl* FunctionTemplate::instantiate(const llvm::StringMap<GenericArg>& genericArgs) {
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

static AstVector<Stmt*> unrollPackLoops(llvm::ArrayRef<Stmt*> stmts, llvm::StringRef packName, llvm::ArrayRef<std::string> expandedNames,
                                        FunctionDecl* parentFunc, Module& module, bool packShadowed) {
    AstVector<Stmt*> result;
    bool shadowedHere = packShadowed;

    for (Stmt* stmt : stmts) {
        switch (stmt->kind) {
        case StmtKind::VarStmt: {
            result.push_back(stmt);
            if (llvm::any_of(llvm::cast<VarStmt>(stmt)->decls, [&](auto* decl) { return decl->getName() == packName; })) shadowedHere = true;
            break;
        }
        case StmtKind::ForEachStmt: {
            auto* forEach = llvm::cast<ForEachStmt>(stmt);
            auto* rangeVar = llvm::dyn_cast<VarExpr>(forEach->range);
            bool isPackLoop = !shadowedHere && rangeVar && rangeVar->identifier == packName;

            if (!isPackLoop) {
                bool nestedShadowed =
                    shadowedHere || forEach->variable->getName() == packName || (forEach->indexVariable && forEach->indexVariable->getName() == packName);
                forEach->body = unrollPackLoops(forEach->body, packName, expandedNames, parentFunc, module, nestedShadowed);
                result.push_back(forEach);
                break;
            }

            if (auto loc = findBreakTargetingPackLoop(forEach->body)) {
                ERROR_RANGE(*loc, getIdentifierEndLocation(*loc, "break"), "break cannot be used in a loop over a variadic parameter");
            }
            if (auto loc = findContinueTargetingPackLoop(forEach->body)) {
                ERROR_RANGE(*loc, getIdentifierEndLocation(*loc, "continue"), "continue cannot be used in a loop over a variadic parameter");
            }

            bool loopVarShadowsPack = forEach->variable->getName() == packName || (forEach->indexVariable && forEach->indexVariable->getName() == packName);
            for (size_t i = 0; i < expandedNames.size(); i++) {
                auto clonedBody = ::cx::instantiate(forEach->body, llvm::StringMap<GenericArg>());
                clonedBody = unrollPackLoops(clonedBody, packName, expandedNames, parentFunc, module, loopVarShadowsPack);

                AstVector<Stmt*> iteration;
                auto* loopVar = makeAST<VarDecl>(Type(), forEach->variable->getName(), makeAST<VarExpr>(expandedNames[i], forEach->location), parentFunc,
                                                 AccessLevel::None, module, forEach->variable->getLocation());
                iteration.push_back(makeAST<VarStmt>(AstVector<VarDecl*>{loopVar}));
                if (forEach->indexVariable) {
                    auto* indexVar =
                        makeAST<VarDecl>(Type(), forEach->indexVariable->getName(), makeAST<IntLiteralExpr>(llvm::APSInt::get(i), forEach->location),
                                         parentFunc, AccessLevel::None, module, forEach->indexVariable->getLocation());
                    iteration.push_back(makeAST<VarStmt>(AstVector<VarDecl*>{indexVar}));
                }
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
        case StmtKind::DoWhileStmt: {
            auto* doWhileStmt = llvm::cast<DoWhileStmt>(stmt);
            doWhileStmt->body = unrollPackLoops(doWhileStmt->body, packName, expandedNames, parentFunc, module, shadowedHere);
            result.push_back(doWhileStmt);
            break;
        }
        case StmtKind::ForStmt: {
            auto* forStmt = llvm::cast<ForStmt>(stmt);
            bool nestedShadowed =
                shadowedHere || (forStmt->variable && llvm::any_of(forStmt->variable->decls, [&](auto* decl) { return decl->getName() == packName; }));
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

FunctionDecl* FunctionTemplate::instantiateVariadic(const llvm::StringMap<GenericArg>& fixedArgs, const std::vector<llvm::StringMap<GenericArg>>& packArgs,
                                                    std::vector<GenericArg>&& cacheKey) {
    auto it = instantiations.find(cacheKey);
    if (it != instantiations.end()) return it->second;

    ASSERT(functionDecl->hasPack());
    ASSERT(!functionDecl->isMethodDecl() || functionDecl->kind == DeclKind::MethodDecl);
    auto templateParams = functionDecl->getParams();
    llvm::StringRef packName = templateParams.back().getName();
    Type packType = templateParams.back().type;
    Location packLoc = templateParams.back().getLocation();

    AstVector<ParamDecl> expandedParams;
    expandedParams.reserve(templateParams.size() - 1 + packArgs.size());
    for (const ParamDecl& param : templateParams.drop_back()) {
        expandedParams.emplace_back(param.type.resolve(fixedArgs), param.getName(), param.isPublic, param.getLocation());
    }

    std::unordered_set<std::string> usedNames;
    for (const ParamDecl& param : expandedParams)
        usedNames.insert(param.getName().str());

    std::vector<std::string> expandedNames;
    expandedNames.reserve(packArgs.size());
    for (size_t i = 0; i < packArgs.size(); ++i) {
        llvm::StringMap<GenericArg> combined = fixedArgs;
        for (auto& entry : packArgs[i])
            combined[entry.getKey()] = entry.getValue();
        Type resolved = packType.resolve(combined);
        std::string name = packName.str() + "_" + std::to_string(i);
        while (!usedNames.insert(name).second)
            name += "_";
        expandedNames.push_back(name);
        expandedParams.emplace_back(resolved, expandedNames.back(), false, packLoc);
    }

    Type returnType = functionDecl->getReturnType().resolve(fixedArgs);
    FunctionProto proto(functionDecl->getName(), std::move(expandedParams), returnType, false, false);

    FunctionDecl* instantiation;
    if (auto* methodDecl = llvm::dyn_cast<MethodDecl>(functionDecl)) {
        instantiation = makeAST<MethodDecl>(std::move(proto), *methodDecl->typeDecl, AstVector<GenericArg>(cacheKey.begin(), cacheKey.end()),
                                            methodDecl->accessLevel, methodDecl->getLocation());
    } else {
        instantiation = makeAST<FunctionDecl>(std::move(proto), AstVector<GenericArg>(cacheKey.begin(), cacheKey.end()), functionDecl->accessLevel,
                                              *functionDecl->getModule(), functionDecl->getLocation());
    }

    if (functionDecl->body) {
        auto clonedBody = ::cx::instantiate(*functionDecl->body, fixedArgs);
        instantiation->body = unrollPackLoops(clonedBody, packName, expandedNames, instantiation, *instantiation->getModule(), false);
    }
    instantiation->isPackInstantiation = true;
    instantiation->disabledChecks = functionDecl->disabledChecks;
    return instantiations.emplace(std::move(cacheKey), instantiation).first->second;
}

std::string cx::getQualifiedFunctionName(Type receiver, llvm::StringRef name, llvm::ArrayRef<GenericArg> genericArgs) {
    std::string result;

    if (receiver) {
        result = receiver.getQualifiedTypeName();
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
    auto paramTypes = mapAst(getParams(), [](const ParamDecl& p) -> Type { return p.type; });
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

FunctionDecl* FunctionDecl::instantiate(const llvm::StringMap<GenericArg>& genericArgs, llvm::ArrayRef<GenericArg> genericArgsArray) {
    if (auto methodDecl = llvm::dyn_cast<MethodDecl>(this)) {
        return methodDecl->instantiate(genericArgs, genericArgsArray, *getTypeDecl());
    } else {
        auto proto = this->proto.instantiate(genericArgs);
        auto instantiation =
            makeAST<FunctionDecl>(std::move(proto), AstVector<GenericArg>(genericArgsArray.begin(), genericArgsArray.end()), accessLevel, module, location);
        instantiation->body = ::instantiate(*body, genericArgs);
        instantiation->disabledChecks = disabledChecks;
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

MethodDecl::MethodDecl(DeclKind kind, FunctionProto proto, TypeDecl& typeDecl, AstVector<GenericArg>&& genericArgs, AccessLevel accessLevel, Location location)
: FunctionDecl(kind, std::move(proto), std::move(genericArgs), accessLevel, *typeDecl.getModule(), location), typeDecl(&typeDecl) {
    for (auto& param : getParams()) {
        param.parent = this;
    }
}

MethodDecl* MethodDecl::instantiate(const llvm::StringMap<GenericArg>& genericArgs, llvm::ArrayRef<GenericArg> genericArgsArray, TypeDecl& typeDecl) {
    switch (kind) {
    case DeclKind::MethodDecl: {
        auto* methodDecl = llvm::cast<MethodDecl>(this);
        auto proto = methodDecl->proto.instantiate(genericArgs);
        auto instantiation = makeAST<MethodDecl>(std::move(proto), typeDecl, AstVector<GenericArg>(genericArgsArray.begin(), genericArgsArray.end()),
                                                 accessLevel, methodDecl->getLocation());
        instantiation->isImplicit = methodDecl->isImplicit;
        instantiation->disabledChecks = methodDecl->disabledChecks;
        if (methodDecl->body) {
            instantiation->body = ::instantiate(*methodDecl->body, genericArgs);
        }
        return instantiation;
    }
    case DeclKind::ConstructorDecl: {
        auto* constructorDecl = llvm::cast<ConstructorDecl>(this);
        auto params = instantiateParams(constructorDecl->getParams(), genericArgs);
        auto instantiation = makeAST<ConstructorDecl>(typeDecl, std::move(params), accessLevel, constructorDecl->getLocation());
        instantiation->isImplicit = constructorDecl->isImplicit;
        instantiation->disabledChecks = constructorDecl->disabledChecks;
        instantiation->body = ::instantiate(*constructorDecl->body, genericArgs);
        return instantiation;
    }
    case DeclKind::DestructorDecl: {
        auto* destructorDecl = llvm::cast<DestructorDecl>(this);
        auto instantiation = makeAST<DestructorDecl>(typeDecl, destructorDecl->getLocation());
        instantiation->body = ::instantiate(*destructorDecl->body, genericArgs);
        instantiation->disabledChecks = destructorDecl->disabledChecks;
        return instantiation;
    }
    default:
        llvm_unreachable("invalid method decl");
    }
}

FieldDecl FieldDecl::instantiate(const llvm::StringMap<GenericArg>& genericArgs, TypeDecl& typeDecl) const {
    auto type = this->type.resolve(genericArgs);
    auto defaultValue = this->defaultValue ? this->defaultValue->instantiate(genericArgs) : nullptr;
    return FieldDecl(type, getName(), defaultValue, typeDecl, accessLevel, location, isManuallyDestroy);
}

AstVector<ParamDecl> cx::instantiateParams(llvm::ArrayRef<ParamDecl> params, const llvm::StringMap<GenericArg>& genericArgs) {
    return mapAst(params, [&](const ParamDecl& param) {
        ParamDecl result(param.type.resolve(genericArgs), param.getName(), param.isPublic, param.getLocation());
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

// In-progress isCopyable queries. Infinite-size types are already an error, but checking
// continues after it, so field cycles must still terminate: re-entry answers non-copyable.
static thread_local std::vector<const TypeDecl*> copyableQueries;

bool TypeDecl::isCopyable() const {
    // Copyable by default; a destructor or a non-copyable field opts out. Explicit
    // ': Copyable' lists are rejected in typechecking, so the interfaces play no role here.
    if (llvm::is_contained(copyableQueries, this)) return false;
    copyableQueries.push_back(this);
    llvm::scope_exit pop([&] { copyableQueries.pop_back(); });
    if (getDestructor()) return false;
    if (auto* enumDecl = llvm::dyn_cast<EnumDecl>(this)) {
        return llvm::all_of(enumDecl->cases, [](auto& enumCase) { return !enumCase.associatedType || enumCase.associatedType.isImplicitlyCopyable(); });
    }
    return llvm::all_of(fields, [](auto& field) { return field.type.isImplicitlyCopyable(); });
}

void TypeDecl::addField(FieldDecl&& field) {
    fields.emplace_back(std::move(field));
}

void TypeDecl::addMethod(Decl* decl) {
    methods.push_back(decl);
}

ConstructorDecl* TypeDecl::addAutogeneratedConstructor() {
    AstVector<ParamDecl> params;
    AstVector<ParamDecl> defaultedParams;
    AstVector<Stmt*> body;

    for (auto& field : fields) {
        llvm::StringRef paramName = field.getName();
        auto* left = makeAST<MemberExpr>(makeAST<VarExpr>("this", field.getLocation()), field.getName(), field.getLocation());
        auto* right = makeAST<VarExpr>(paramName, field.getLocation());
        body.push_back(makeAST<ExprStmt>(makeAST<BinaryExpr>(Token::Assignment, left, right, field.getLocation())));

        ParamDecl param(field.type, paramName, false, field.getLocation());
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
    autogeneratedInit->isAutogenerated = true;
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

DestructorDecl* TypeDecl::getOrSynthesizeDefaultDestructor() {
    ASSERT(!getDestructor());

    auto synthesize = [&] {
        auto destructor = makeAST<DestructorDecl>(*this, getLocation());
        destructor->body = AstVector<Stmt*>();
        return destructor;
    };

    if (auto* enumDecl = llvm::dyn_cast<EnumDecl>(this)) {
        return enumDecl->hasDestructiblePayload() ? synthesize() : nullptr;
    }

    for (auto& field : fields) {
        if (field.isManuallyDestroy) continue;
        if (field.type.needsDestruction()) {
            // Union members overlap, so destroying each is wrong; sema only
            // allows trivially-destructible or '@manuallyDestroy' members.
            ASSERT(!isUnion());
            return synthesize();
        }
    }

    return nullptr;
}

Type TypeDecl::getType() const {
    return BasicType::get(name, genericArgs, location);
}

unsigned TypeDecl::getFieldIndex(const FieldDecl* field) const {
    for (const auto& p : llvm::enumerate(fields)) {
        if (&p.value() == field) {
            return static_cast<unsigned>(p.index());
        }
    }
    llvm_unreachable("unknown field");
}

TypeDecl* TypeTemplate::instantiate(const llvm::StringMap<GenericArg>& genericArgs) {
    ASSERT(!genericParams.empty() && !genericArgs.empty());
    auto orderedGenericArgs = map(genericParams, [&](auto& genericParam) { return genericArgs.find(genericParam.getName())->second; });

    auto it = instantiations.find(orderedGenericArgs);
    if (it != instantiations.end()) return it->second;

    auto instantiation = llvm::cast<TypeDecl>(typeDecl->instantiate(genericArgs, orderedGenericArgs));
    return instantiations.emplace(std::move(orderedGenericArgs), instantiation).first->second;
}

TypeDecl* TypeTemplate::instantiate(llvm::ArrayRef<GenericArg> genericArgs) {
    ASSERT(genericArgs.size() == genericParams.size());
    llvm::StringMap<GenericArg> genericArgsMap;

    for (auto&& [genericArg, genericParam] : llvm::zip_first(genericArgs, genericParams)) {
        genericArgsMap[genericParam.getName()] = genericArg;
    }

    return instantiate(genericArgsMap);
}

EnumCase::EnumCase(llvm::StringRef name, Expr* value, Type associatedType, AccessLevel accessLevel, Location location)
: VariableDecl(DeclKind::EnumCase, accessLevel, nullptr, Type() /* initialized by EnumDecl constructor */), name(internString(name)), value(value),
  associatedType(associatedType), location(location) {}

void EnumDecl::addCase(EnumCase&& enumCase) {
    enumCase.parent = this;
    enumCase.type = NOTNULL(getType());
    cases.push_back(std::move(enumCase));
}

EnumCase* EnumDecl::getCaseByName(llvm::StringRef name) {
    for (auto& enumCase : cases) {
        if (enumCase.getName() == name) {
            return &enumCase;
        }
    }
    return nullptr;
}

bool EnumDecl::isPayloadView(Type declared, Type viewed) {
    if (!declared.isEnumType() || declared.isOptionalType()) return false;
    auto* enumDecl = llvm::cast<EnumDecl>(declared.getDecl());
    if (llvm::any_of(enumDecl->cases, [&](auto& enumCase) { return enumCase.associatedType && enumCase.associatedType == viewed; })) return true;
    return viewed.isAnonymousStructType() && viewed.getAnonymousStructElements().empty()
        && llvm::any_of(enumDecl->cases, [](auto& enumCase) { return !enumCase.associatedType; });
}

bool EnumDecl::hasAssociatedValues() const {
    return llvm::any_of(cases, [](auto& enumCase) { return bool(enumCase.associatedType); });
}

bool EnumDecl::hasDestructiblePayload() const {
    return llvm::any_of(cases, [](auto& enumCase) { return enumCase.associatedType && enumCase.associatedType.needsDestruction(); });
}

FieldDecl::FieldDecl(Type type, llvm::StringRef name, Expr* defaultValue, TypeDecl& parent, AccessLevel accessLevel, Location location, bool isManuallyDestroy)
: VariableDecl(DeclKind::FieldDecl, accessLevel, &parent, type), name(internString(name)), defaultValue(defaultValue), location(location),
  isManuallyDestroy(isManuallyDestroy) {}

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

static void instantiateMethods(TypeDecl& instantiation, llvm::ArrayRef<Decl*> methods, const llvm::StringMap<GenericArg>& genericArgs) {
    for (Decl* method : methods) {
        if (auto* nonTemplateMethod = llvm::dyn_cast<MethodDecl>(method)) {
            instantiation.addMethod(nonTemplateMethod->instantiate(genericArgs, {}, instantiation));
        } else {
            auto* functionTemplate = llvm::cast<FunctionTemplate>(method);
            auto* methodDecl = llvm::cast<MethodDecl>(functionTemplate->functionDecl);
            auto methodInstantiation = methodDecl->instantiate(genericArgs, {}, instantiation);

            AstVector<GenericParamDecl> genericParams;
            genericParams.reserve(functionTemplate->genericParams.size());

            for (auto& genericParam : functionTemplate->genericParams) {
                genericParams.emplace_back(genericParam.getName(), genericParam.getLocation());
                for (Type constraint : genericParam.constraints) {
                    genericParams.back().constraints.push_back(constraint.resolve(genericArgs));
                }
                genericParams.back().isValueParam = genericParam.isValueParam;
                genericParams.back().valueType = genericParam.valueType.resolve(genericArgs);
            }

            auto accessLevel = methodInstantiation->accessLevel;
            instantiation.addMethod(makeAST<FunctionTemplate>(std::move(genericParams), methodInstantiation, accessLevel));
        }
    }
}

// TODO: Ensure that the same decl isn't instantiated multiple times with same generic args, to avoid duplicate work.
Decl* Decl::instantiate(const llvm::StringMap<GenericArg>& genericArgs, llvm::ArrayRef<GenericArg> genericArgsArray) const {
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
        auto interfaces = mapAst(typeDecl->interfaces, [&](Type type) { return type.resolve(genericArgs); });
        auto instantiation = makeAST<TypeDecl>(typeDecl->tag, typeDecl->getName(), AstVector<GenericArg>(genericArgsArray.begin(), genericArgsArray.end()),
                                               std::move(interfaces), accessLevel, *typeDecl->getModule(), typeDecl, typeDecl->getLocation());
        for (auto& field : typeDecl->fields) {
            auto defaultValue = field.defaultValue ? field.defaultValue->instantiate(genericArgs) : nullptr;
            instantiation->addField(FieldDecl(field.type.resolve(genericArgs), field.getName(), defaultValue, *instantiation, field.accessLevel,
                                              field.getLocation(), field.isManuallyDestroy));
        }

        instantiateMethods(*instantiation, typeDecl->methods, genericArgs);

        return instantiation;
    }
    case DeclKind::TypeTemplate:
        llvm_unreachable("handled via TypeTemplate::instantiate()");

    case DeclKind::TypeAliasDecl:
        llvm_unreachable("type aliases resolve to their aliased type");

    case DeclKind::EnumDecl: {
        auto* enumDecl = llvm::cast<EnumDecl>(this);
        AstVector<EnumCase> cases;
        for (auto& enumCase : enumDecl->cases) {
            cases.emplace_back(enumCase.getName(), enumCase.value ? enumCase.value->instantiate(genericArgs) : nullptr,
                               enumCase.associatedType.resolve(genericArgs), enumCase.accessLevel, enumCase.getLocation());
        }
        auto interfaces = mapAst(enumDecl->interfaces, [&](Type type) { return type.resolve(genericArgs); });
        auto instantiation = makeAST<EnumDecl>(enumDecl->getName(), std::move(cases), std::move(interfaces), accessLevel, *enumDecl->getModule(), enumDecl,
                                               enumDecl->getLocation());
        instantiation->genericArgs = AstVector<GenericArg>(genericArgsArray.begin(), genericArgsArray.end());
        for (auto& enumCase : instantiation->cases) {
            enumCase.type = NOTNULL(instantiation->getType());
        }
        instantiateMethods(*instantiation, enumDecl->methods, genericArgs);
        return instantiation;
    }
    case DeclKind::EnumCase:
        llvm_unreachable("handled via EnumDecl");

    case DeclKind::VarDecl: {
        auto* varDecl = llvm::cast<VarDecl>(this);
        auto type = varDecl->type.resolve(genericArgs);
        auto initializer = varDecl->initializer ? varDecl->initializer->instantiate(genericArgs) : nullptr;
        auto* instantiation =
            makeAST<VarDecl>(type, varDecl->getName(), initializer, varDecl->parent, accessLevel, *varDecl->getModule(), varDecl->getLocation());
        instantiation->isManuallyDestroy = varDecl->isManuallyDestroy;
        instantiation->isConst = varDecl->isConst;
        return instantiation;
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

ConstructorDecl::ConstructorDecl(TypeDecl& receiverTypeDecl, AstVector<ParamDecl>&& params, AccessLevel accessLevel, Location location)
: MethodDecl(DeclKind::ConstructorDecl, FunctionProto("init", std::move(params), Type::getVoid(), false, false), receiverTypeDecl, {}, accessLevel, location) {}

DestructorDecl::DestructorDecl(TypeDecl& receiverTypeDecl, Location location)
: MethodDecl(DeclKind::DestructorDecl, FunctionProto("deinit", {}, Type::getVoid(), false, false), receiverTypeDecl, {}, AccessLevel::None, location) {}

std::vector<Note> cx::getPreviousDefinitionNotes(llvm::ArrayRef<Decl*> decls) {
    return map(decls, [](Decl* decl) { return Note{decl->getLocation(), "previous definition here"}; });
}

static bool isOpaquePlaceholder(const TypeDecl& decl) {
    if ((!decl.isStruct() && !decl.isUnion()) || !decl.fields.empty() || !decl.staticConsts.empty() || !decl.interfaces.empty() || decl.packed) return false;
    // Empty structs still carry an autogenerated constructor; only user methods disqualify.
    return llvm::all_of(decl.methods, [](Decl* method) {
        auto* ctor = llvm::dyn_cast<ConstructorDecl>(method);
        return ctor && ctor->isAutogenerated;
    });
}

static bool isFromCHeader(const TypeDecl& decl) {
    Module* module = decl.getModule();
    return module && module->isCHeaderImport;
}

void cx::bindTypeSpelling(Type type, TypeDecl& decl) {
    auto& node = llvm::cast<BasicType>(*type);
    TypeDecl* existing = node.decl;
    if (!existing) {
        node.decl = &decl;
        return;
    }
    if (existing == &decl) return;
    // Duplicates from C headers resolve to the last one; cx types keep last-wins.
    if (isFromCHeader(*existing) == isFromCHeader(decl)) {
        node.decl = &decl;
        return;
    }
    bool existingPlaceholder = isOpaquePlaceholder(*existing);
    bool declPlaceholder = isOpaquePlaceholder(decl);
    if ((existingPlaceholder || declPlaceholder) && existing->tag == decl.tag) {
        if (existingPlaceholder) node.decl = &decl;
        return;
    }
    REPORT_ERROR_WITH_NOTES(decl.getLocation(), getPreviousDefinitionNotes({existing}), "redefinition of '" << decl.getQualifiedName() << "'");
    node.decl = &decl;
}
