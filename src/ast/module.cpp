#include "module.h"
#include "ast-print.h"
#include <algorithm>

using namespace cx;

llvm::StringMap<Module*> Module::allImportedModules;

std::vector<Module*> Module::getAllImportedModules() {
    auto modules = map(allImportedModules, [](auto& p) { return p.second; });
    // Emit std first: its globals and functions must live in its own IR module.
    // Otherwise a dependent emitted first pulls std code into its module via
    // getFunction, while std globals stay in std, and the LLVM backend caches
    // globals per IR object, producing cross-module references.
    for (size_t i = 1; i < modules.size(); ++i) {
        if (modules[i]->name == "std") {
            std::rotate(modules.begin(), modules.begin() + i, modules.begin() + i + 1);
            break;
        }
    }
    return modules;
}

Module* Module::findImportedModule(llvm::StringRef name) {
    auto it = allImportedModules.find(name);
    if (it == allImportedModules.end()) return nullptr;
    return it->second;
}

void Module::registerImportedModule(llvm::StringRef name, Module* module) {
    allImportedModules[name] = module;
}

Module* Module::getStdlibModule() {
    return findImportedModule("std");
}

void Module::resetImportedModules() {
    allImportedModules.clear();
}

bool Module::addToSymbolTableWithName(Decl& decl, llvm::StringRef name) {
    if (auto existing = symbolTable.findInCurrentScope(name); !existing.empty()) {
        REPORT_ERROR_WITH_NOTES(decl.getLocation(), getPreviousDefinitionNotes(existing), "redefinition of '" << name << "'");
        return true;
    }

    if (decl.isGlobal()) {
        symbolTable.addGlobal(name, &decl);
    } else {
        symbolTable.add(name, &decl);
    }
    return false;
}

template<typename DeclT> static bool rejectMatchingPrototype(SymbolTable& symbolTable, DeclT& decl, const FunctionDecl& proto) {
    if (auto existing = symbolTable.findWithMatchingPrototype(proto)) {
        REPORT_ERROR_WITH_NOTES(decl.getLocation(), getPreviousDefinitionNotes(existing), "redefinition of '" << decl.getQualifiedName() << "'");
        return true;
    }
    return false;
}

void Module::addToSymbolTable(FunctionTemplate& decl) {
    rejectMatchingPrototype(symbolTable, decl, *decl.functionDecl);
    symbolTable.addGlobal(decl.getQualifiedName(), &decl);
}

void Module::addToSymbolTable(FunctionDecl& decl) {
    if (!rejectMatchingPrototype(symbolTable, decl, decl) && decl.isExtern()) {
        // C has no overloading: same-name externs share one symbol even with different signatures.
        for (Decl* candidate : symbolTable.findFirst(decl.getQualifiedName())) {
            if (auto* existing = llvm::dyn_cast<FunctionDecl>(candidate); existing && existing->isExtern()) {
                REPORT_ERROR_WITH_NOTES(decl.getLocation(), getPreviousDefinitionNotes(existing), "redefinition of '" << decl.getQualifiedName() << "'");
                break;
            }
        }
    }
    symbolTable.addGlobal(decl.getQualifiedName(), &decl);
}

void Module::addToSymbolTable(TypeTemplate& decl) {
    addToSymbolTableWithName(decl, decl.typeDecl->getName());
}

void Module::addToSymbolTable(TypeDecl& decl) {
    if (addToSymbolTableWithName(decl, decl.getQualifiedName())) return;
    bindTypeSpelling(decl.getType(), decl);

    for (auto& memberDecl : decl.methods) {
        if (auto* nonTemplateMethod = llvm::dyn_cast<MethodDecl>(memberDecl)) {
            addToSymbolTable(*nonTemplateMethod);
        }
    }
}

void Module::addToSymbolTable(TypeAliasDecl& decl) {
    addToSymbolTableWithName(decl, decl.getName());
}

void Module::addToSymbolTable(EnumDecl& decl) {
    addToSymbolTable(static_cast<TypeDecl&>(decl));
}

void Module::addToSymbolTable(VarDecl& decl) {
    addToSymbolTableWithName(decl, decl.getName());
}

void Module::addToSymbolTable(Decl* decl) {
    symbolTable.add(decl->getName(), decl);

    if (auto* typeDecl = llvm::dyn_cast<TypeDecl>(decl)) {
        bindTypeSpelling(typeDecl->getType(), *typeDecl);
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
