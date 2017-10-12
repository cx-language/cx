#pragma once

#include <vector>
#include <memory>
#include <string>
#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringMap.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/STLExtras.h>
#include "decl.h"

namespace delta {

class Module;

/// Container for the AST of a single file.
class SourceFile {
public:
    explicit SourceFile(llvm::StringRef filePath) : filePath(filePath) {}
    llvm::ArrayRef<std::unique_ptr<Decl>> getTopLevelDecls() const { return topLevelDecls; }
    llvm::StringRef getFilePath() const { return filePath; }
    llvm::ArrayRef<std::shared_ptr<Module>> getImportedModules() const { return importedModules; }
    void setDecls(std::vector<std::unique_ptr<Decl>>&& decls) { topLevelDecls = std::move(decls); }

    void addImportedModule(std::shared_ptr<Module> module) {
        if (!llvm::is_contained(importedModules, module)) {
            importedModules.push_back(module);
        }
    }

private:
    std::string filePath;
    std::vector<std::unique_ptr<Decl>> topLevelDecls;
    std::vector<std::shared_ptr<Module>> importedModules;
};

class SymbolTable {
public:
    SymbolTable() : scopes(1) {}
    void pushScope() { scopes.emplace_back(); }
    void popScope() { scopes.pop_back(); }
    void add(llvm::StringRef name, Decl* decl) { scopes.back()[name].push_back(decl); }
    void addGlobal(llvm::StringRef name, Decl* decl) { scopes.front()[name].push_back(decl); }
    void addIdentifierReplacement(llvm::StringRef name, llvm::StringRef replacement) {
        identifierReplacements.try_emplace(name, replacement);
    }
    bool contains(const std::string& name) const { return !find(name).empty(); }

    llvm::ArrayRef<Decl*> find(const std::string& name) const {
        auto realName = applyIdentifierReplacements(name);

        for (const auto& scope : llvm::reverse(scopes)) {
            auto it = scope.find(realName);
            if (it != scope.end()) return it->second;
        }
        return {};
    }

    template<typename T>
    T* findWithMatchingParams(const T& toFind) const {
        for (Decl* decl : find(mangle(toFind))) {
            T* t = llvm::dyn_cast<T>(decl);
            if (!t || t->getParams().size() != toFind.getParams().size()) continue;
            if (std::equal(toFind.getParams().begin(), toFind.getParams().end(), t->getParams().begin(),
                           [](const ParamDecl& a, const ParamDecl& b) {
                               return a.getName() == b.getName() && a.getType() == b.getType();
                           }))
                return t;
        }
        return nullptr;
    }

private:
    llvm::StringRef applyIdentifierReplacements(llvm::StringRef name) const {
        llvm::StringRef initialName = name;
        while (true) {
            auto it = identifierReplacements.find(name);
            if (it == identifierReplacements.end()) return name;
            if (it->second == initialName) return name; // Break replacement cycle.
            name = it->second;
        }
    }

    std::vector<llvm::StringMap<std::vector<Decl*>>> scopes;
    llvm::StringMap<std::string> identifierReplacements;
};

/// Container for the AST of a whole module, comprised of one or more SourceFiles.
class Module {
public:
    Module(llvm::StringRef name) : name(name) {}
    void addSourceFile(SourceFile&& file) { sourceFiles.emplace_back(std::move(file)); }
    llvm::ArrayRef<SourceFile> getSourceFiles() const { return sourceFiles; }
    llvm::MutableArrayRef<SourceFile> getSourceFiles() { return sourceFiles; }
    llvm::StringRef getName() const { return name; }
    const SymbolTable& getSymbolTable() const { return symbolTable; }
    SymbolTable& getSymbolTable() { return symbolTable; }

    std::vector<Module*> getImportedModules() const {
        std::vector<Module*> importedModules;
        for (auto& sourceFile : getSourceFiles()) {
            for (auto& importedModule : sourceFile.getImportedModules()) {
                importedModules.push_back(importedModule.get());
            }
        }
        return importedModules;
    }

private:
    std::string name;
    std::vector<SourceFile> sourceFiles;
    SymbolTable symbolTable;
};

}
