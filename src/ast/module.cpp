#include "module.h"
#include "ast-print.h"

using namespace cx;

llvm::StringMap<Module*> Module::allImportedModules;

std::vector<Module*> Module::getAllImportedModules() {
    return map(allImportedModules, [](auto& p) { return p.second; });
}

Module* Module::getStdlibModule() {
    auto it = allImportedModules.find("std");
    if (it == allImportedModules.end()) return nullptr;
    return it->second;
}

void Module::addToSymbolTableWithName(Decl& decl, llvm::StringRef name) {
    if (auto existing = symbolTable.findInCurrentScope(name); !existing.empty()) {
        REPORT_ERROR_WITH_NOTES(decl.getLocation(), getPreviousDefinitionNotes(existing), "redefinition of '" << name << "'");
    }

    if (decl.isGlobal()) {
        symbolTable.addGlobal(name, &decl);
    } else {
        symbolTable.add(name, &decl);
    }
}

void Module::addToSymbolTable(FunctionTemplate& decl) {
    if (auto existing = symbolTable.findWithMatchingPrototype(*decl.functionDecl)) {
        REPORT_ERROR_WITH_NOTES(decl.getLocation(), getPreviousDefinitionNotes(existing), "redefinition of '" << decl.getQualifiedName() << "'");
    }
    symbolTable.addGlobal(decl.getQualifiedName(), &decl);
}

void Module::addToSymbolTable(FunctionDecl& decl) {
    if (auto existing = symbolTable.findWithMatchingPrototype(decl)) {
        REPORT_ERROR_WITH_NOTES(decl.getLocation(), getPreviousDefinitionNotes(existing), "redefinition of '" << decl.getQualifiedName() << "'");
    }
    symbolTable.addGlobal(decl.getQualifiedName(), &decl);
}

void Module::addToSymbolTable(TypeTemplate& decl) {
    addToSymbolTableWithName(decl, decl.typeDecl->getName());
}

void Module::addToSymbolTable(TypeDecl& decl) {
    llvm::cast<BasicType>(decl.getType().typeBase)->decl = &decl;
    addToSymbolTableWithName(decl, decl.getQualifiedName());

    for (auto& memberDecl : decl.methods) {
        if (auto* nonTemplateMethod = llvm::dyn_cast<MethodDecl>(memberDecl)) {
            addToSymbolTable(*nonTemplateMethod);
        }
    }
}

void Module::addToSymbolTable(EnumDecl& decl) {
    llvm::cast<BasicType>(decl.getType().typeBase)->decl = &decl;
    addToSymbolTableWithName(decl, decl.getQualifiedName());
}

void Module::addToSymbolTable(VarDecl& decl) {
    addToSymbolTableWithName(decl, decl.getName());
}

void Module::addToSymbolTable(Decl* decl) {
    symbolTable.add(decl->getName(), decl);

    if (auto* typeDecl = llvm::dyn_cast<TypeDecl>(decl)) {
        llvm::cast<BasicType>(*typeDecl->getType()).decl = typeDecl;
    }
}

void Module::addIdentifierReplacement(llvm::StringRef source, llvm::StringRef target) {
    ASSERT(!target.empty());
    symbolTable.addIdentifierReplacement(source, target);
}

void Module::print(llvm::raw_ostream& stream) const {
    for (auto& sourceFile : sourceFiles) {
        for (auto* topLevelDecl : sourceFile.topLevelDecls) {
            stream << *topLevelDecl << "\n";
        }
    }
}

Scope::Scope(Decl* parent, SymbolTable* symbolTable) : parent(parent), symbolTable(symbolTable) {
    symbolTable->pushScope(*this);
}

Scope::~Scope() {
    symbolTable->popScope();
}
