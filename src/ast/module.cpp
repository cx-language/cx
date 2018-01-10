#include "module.h"
#include "mangle.h"

using namespace delta;

llvm::StringMap<std::shared_ptr<Module>> Module::allImportedModules;

std::vector<Module*> Module::getAllImportedModules() {
    return map(allImportedModules, [](const llvm::StringMapEntry<std::shared_ptr<Module>>& p) {
        return p.second.get();
    });
}

llvm::ArrayRef<std::shared_ptr<Module>> Module::getStdlibModules() {
    auto it = allImportedModules.find("std");
    if (it == allImportedModules.end()) return {};
    return it->second;
}

/// Storage for Decls that are not in the AST but are referenced by the symbol table.
static std::vector<std::unique_ptr<Decl>> nonASTDecls;

void Module::addToSymbolTableWithName(Decl& decl, llvm::StringRef name, bool global) {
    if (getSymbolTable().contains(name)) {
        error(decl.getLocation(), "redefinition of '", name, "'");
    }

    if (global) {
        getSymbolTable().addGlobal(name, &decl);
    } else {
        getSymbolTable().add(name, &decl);
    }
}

void Module::addToSymbolTable(FunctionTemplate& decl) {
    if (getSymbolTable().findWithMatchingPrototype(*decl.getFunctionDecl())) {
        error(decl.getLocation(), "redefinition of '", mangle(decl), "'");
    }
    getSymbolTable().addGlobal(mangle(decl), &decl);
}

void Module::addToSymbolTable(FunctionDecl& decl) {
    if (getSymbolTable().findWithMatchingPrototype(decl)) {
        error(decl.getLocation(), "redefinition of '", mangle(decl), "'");
    }
    getSymbolTable().addGlobal(mangle(decl), &decl);
}

void Module::addToSymbolTable(TypeTemplate& decl) {
    addToSymbolTableWithName(decl, decl.getTypeDecl()->getName(), true);
}

void Module::addToSymbolTable(TypeDecl& decl) {
    addToSymbolTableWithName(decl, mangle(decl), true);

    for (auto& memberDecl : decl.getMemberDecls()) {
        if (auto* nonTemplateMethod = llvm::dyn_cast<MethodDecl>(memberDecl.get())) {
            addToSymbolTable(*nonTemplateMethod);
        }
    }
}

void Module::addToSymbolTable(EnumDecl& decl) {
    addToSymbolTableWithName(decl, mangle(decl), true);
}

void Module::addToSymbolTable(VarDecl& decl, bool global) {
    addToSymbolTableWithName(decl, decl.getName(), global);
}

template<typename DeclT>
void Module::addToSymbolTableNonAST(DeclT& decl) {
    std::string name = decl.getName();
    nonASTDecls.push_back(llvm::make_unique<DeclT>(std::move(decl)));
    getSymbolTable().add(std::move(name), nonASTDecls.back().get());

    if (std::is_same<DeclT, TypeDecl>::value) {
        auto& typeDecl = llvm::cast<TypeDecl>(*nonASTDecls.back());
        llvm::cast<BasicType>(*typeDecl.getType()).setDecl(&typeDecl);
    }
}

void Module::addToSymbolTable(FunctionDecl&& decl) {
    addToSymbolTableNonAST(decl);
}

void Module::addToSymbolTable(TypeDecl&& decl) {
    addToSymbolTableNonAST(decl);
}

void Module::addToSymbolTable(VarDecl&& decl) {
    addToSymbolTableNonAST(decl);
}

void Module::addIdentifierReplacement(llvm::StringRef source, llvm::StringRef target) {
    ASSERT(!target.empty());
    getSymbolTable().addIdentifierReplacement(source, target);
}

template<typename ModuleContainer>
static llvm::SmallVector<Decl*, 1> findDeclsInModules(llvm::StringRef name, const ModuleContainer& modules) {
    llvm::SmallVector<Decl*, 1> decls;

    for (auto& module : modules) {
        llvm::ArrayRef<Decl*> matches = module->getSymbolTable().find(name);
        decls.append(matches.begin(), matches.end());
    }

    return decls;
}

template<typename ModuleContainer>
static Decl* findDeclInModules(llvm::StringRef name, SourceLocation location, const ModuleContainer& modules) {
    auto decls = findDeclsInModules(name, modules);

    switch (decls.size()) {
        case 1: return decls[0];
        case 0: return nullptr;
        default: error(location, "ambiguous reference to '", name, "'");
    }
}

Decl& Module::findDecl(llvm::StringRef name, SourceLocation location, SourceFile* currentSourceFile,
                       llvm::ArrayRef<std::pair<FieldDecl*, bool>> currentFieldDecls) const {
    ASSERT(!name.empty());

    if (Decl* match = findDeclInModules(name, location, llvm::makeArrayRef(this))) {
        return *match;
    }

    for (auto& field : currentFieldDecls) {
        if (field.first->getName() == name) {
            return *field.first;
        }
    }

    if (Decl* match = findDeclInModules(name, location, getStdlibModules())) {
        return *match;
    }

    if (currentSourceFile) {
        if (Decl* match = findDeclInModules(name, location, currentSourceFile->getImportedModules())) {
            return *match;
        }
    } else {
        if (Decl* match = findDeclInModules(name, location, getAllImportedModules())) {
            return *match;
        }
    }

    error(location, "unknown identifier '", name, "'");
}

std::vector<Decl*> Module::findDecls(llvm::StringRef name, SourceFile* currentSourceFile,
                                     FunctionDecl* currentFunction, TypeDecl* receiverTypeDecl) const {
    std::vector<Decl*> decls;

    if (!receiverTypeDecl && currentFunction) {
        receiverTypeDecl = currentFunction->getTypeDecl();
    }

    if (receiverTypeDecl) {
        for (auto& decl : receiverTypeDecl->getMemberDecls()) {
            if (auto* functionDecl = llvm::dyn_cast<FunctionDecl>(decl.get())) {
                if (functionDecl->getName() == name) {
                    decls.emplace_back(decl.get());
                }
            } else if (auto* functionTemplate = llvm::dyn_cast<FunctionTemplate>(decl.get())) {
                if (mangle(*functionTemplate) == name) {
                    decls.emplace_back(decl.get());
                }
            }
        }

        for (auto& field : receiverTypeDecl->getFields()) {
            if (field.getName() == name || mangle(field) == name) {
                decls.emplace_back(&field);
            }
        }
    }

    if (getName() != "std") {
        append(decls, findDeclsInModules(name, llvm::makeArrayRef(this)));
    }

    append(decls, findDeclsInModules(name, getStdlibModules()));

    if (currentSourceFile) {
        append(decls, findDeclsInModules(name, currentSourceFile->getImportedModules()));
    } else {
        append(decls, findDeclsInModules(name, getAllImportedModules()));
    }

    return decls;
}
