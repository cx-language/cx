#include "module.h"

using namespace delta;

llvm::StringMap<Module*> Module::allImportedModules;

std::vector<Module*> Module::getAllImportedModules() {
    return map(allImportedModules, [](auto& p) { return p.second; });
}

Module* Module::getStdlibModule() {
    auto it = allImportedModules.find("std");
    if (it == allImportedModules.end()) return nullptr;
    return it->second;
}

void Module::addToSymbolTableWithName(Decl& decl, llvm::StringRef name, bool global) {
    if (getSymbolTable().contains(name)) {
        ERROR(decl.getLocation(), "redefinition of '" << name << "'");
    }

    if (global) {
        getSymbolTable().addGlobal(name, &decl);
    } else {
        getSymbolTable().add(name, &decl);
    }
}

void Module::addToSymbolTable(FunctionTemplate& decl) {
    if (getSymbolTable().findWithMatchingPrototype(*decl.getFunctionDecl())) {
        ERROR(decl.getLocation(), "redefinition of '" << decl.getQualifiedName() << "'");
    }
    getSymbolTable().addGlobal(decl.getQualifiedName(), &decl);
}

void Module::addToSymbolTable(FunctionDecl& decl) {
    if (getSymbolTable().findWithMatchingPrototype(decl)) {
        ERROR(decl.getLocation(), "redefinition of '" << decl.getQualifiedName() << "'");
    }
    getSymbolTable().addGlobal(decl.getQualifiedName(), &decl);
}

void Module::addToSymbolTable(TypeTemplate& decl) {
    addToSymbolTableWithName(decl, decl.getTypeDecl()->getName(), true);
}

void Module::addToSymbolTable(TypeDecl& decl) {
    llvm::cast<BasicType>(decl.getType().getBase())->setDecl(&decl);
    addToSymbolTableWithName(decl, decl.getQualifiedName(), true);

    for (auto& memberDecl : decl.getMethods()) {
        if (auto* nonTemplateMethod = llvm::dyn_cast<MethodDecl>(memberDecl)) {
            addToSymbolTable(*nonTemplateMethod);
        }
    }
}

void Module::addToSymbolTable(EnumDecl& decl) {
    llvm::cast<BasicType>(decl.getType().getBase())->setDecl(&decl);
    addToSymbolTableWithName(decl, decl.getName(), true);
}

void Module::addToSymbolTable(VarDecl& decl, bool global) {
    addToSymbolTableWithName(decl, decl.getName(), global);
}

void Module::addToSymbolTable(Decl* decl) {
    getSymbolTable().add(decl->getName(), decl);

    if (auto* typeDecl = llvm::dyn_cast<TypeDecl>(decl)) {
        llvm::cast<BasicType>(*typeDecl->getType()).setDecl(typeDecl);
    }
}

void Module::addIdentifierReplacement(llvm::StringRef source, llvm::StringRef target) {
    ASSERT(!target.empty());
    getSymbolTable().addIdentifierReplacement(source, target);
}
