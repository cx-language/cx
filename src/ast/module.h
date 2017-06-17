#pragma once

#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/STLExtras.h>
#include "decl.h"

namespace delta {

class Module;

/// Container for the AST of a single file.
class SourceFile {
public:
    explicit SourceFile(llvm::StringRef filePath,
                        std::vector<std::unique_ptr<Decl>>&& topLevelDecls)
    : filePath(filePath), topLevelDecls(std::move(topLevelDecls)) { }
    llvm::ArrayRef<std::unique_ptr<Decl>> getTopLevelDecls() const { return topLevelDecls; }
    llvm::StringRef getFilePath() const { return filePath; }
    llvm::ArrayRef<std::shared_ptr<Module>> getImportedModules() const { return importedModules; }

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
    bool contains(const std::string& name) const { return !find(name).empty(); }

    llvm::ArrayRef<Decl*> find(const std::string& name) const {
        for (const auto& scope : llvm::reverse(scopes)) {
            auto it = scope.find(name);
            if (it != scope.end()) return it->second;
        }
        return {};
    }

    template<typename T>
    T* findWithMatchingParams(const T& toFind) const {
        for (Decl* decl : find(mangle(toFind))) {
            T* t = llvm::dyn_cast<T>(decl);
            if (!t || t->params.size() != toFind.params.size()) continue;
            if (std::equal(toFind.params.begin(), toFind.params.end(), t->params.begin(),
                           [](const ParamDecl& a, const ParamDecl& b) { return a.name == b.name; }))
                return t;
        }
        return nullptr;
    }

private:
    std::vector<std::unordered_map<std::string, llvm::SmallVector<Decl*, 1>>> scopes;
};

/// Container for the AST of a whole module, comprised of one or more SourceFiles.
class Module {
public:
    Module(llvm::StringRef name) : name(name) { }
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
