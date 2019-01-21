#pragma once

#include <memory>
#include <string>
#include <vector>
#pragma warning(push, 0)
#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringMap.h>
#include <llvm/ADT/StringRef.h>
#pragma warning(pop)
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
    bool containsInCurrentScope(const std::string& name) const { return !findInCurrentScope(name).empty(); }

    llvm::ArrayRef<Decl*> find(const std::string& name) const {
        auto realName = applyIdentifierReplacements(name);

        for (const auto& scope : llvm::reverse(scopes)) {
            auto it = scope.find(realName);
            if (it != scope.end()) return it->second;
        }
        return {};
    }

    Decl* findOne(const std::string& name) const {
        auto results = find(name);
        switch (results.size()) {
            case 1:
                return results[0];
            case 0:
                return nullptr;
            default:
                throw std::runtime_error("Found multiple occurrences of '" + name + "'");
        }
    }

    llvm::ArrayRef<Decl*> findInCurrentScope(const std::string& name) const {
        if (!scopes.empty()) {
            auto it = scopes.back().find(applyIdentifierReplacements(name));
            if (it != scopes.back().end()) return it->second;
        }
        return {};
    }

    FunctionDecl* findWithMatchingPrototype(const FunctionDecl& toFind) const {
        for (Decl* decl : find(toFind.getQualifiedName())) {
            auto* functionDecl = llvm::dyn_cast<FunctionDecl>(decl);
            if (!functionDecl || functionDecl->getParams().size() != toFind.getParams().size()) continue;
            if (functionDecl->isMutating() != toFind.isMutating()) continue;
            if (std::equal(toFind.getParams().begin(), toFind.getParams().end(), functionDecl->getParams().begin(),
                           [](auto& a, auto& b) { return a.getName() == b.getName() && a.getType() == b.getType(); })) {
                return functionDecl;
            }
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
    Module(llvm::StringRef name, std::vector<std::string>&& defines = {}) : name(name), defines(std::move(defines)) {}
    void addSourceFile(SourceFile&& file) { sourceFiles.emplace_back(std::move(file)); }
    llvm::ArrayRef<SourceFile> getSourceFiles() const { return sourceFiles; }
    llvm::MutableArrayRef<SourceFile> getSourceFiles() { return sourceFiles; }
    llvm::StringRef getName() const { return name; }
    const SymbolTable& getSymbolTable() const { return symbolTable; }
    SymbolTable& getSymbolTable() { return symbolTable; }
    bool isDefined(llvm::StringRef define) const { return llvm::is_contained(defines, define); }

    std::vector<Module*> getImportedModules() const {
        std::vector<Module*> importedModules;
        for (auto& sourceFile : getSourceFiles()) {
            for (auto& importedModule : sourceFile.getImportedModules()) {
                importedModules.push_back(importedModule.get());
            }
        }
        return importedModules;
    }

    void addToSymbolTable(FunctionTemplate& decl);
    void addToSymbolTable(FunctionDecl& decl);
    void addToSymbolTable(FunctionDecl&& decl);
    void addToSymbolTable(TypeTemplate& decl);
    void addToSymbolTable(TypeDecl& decl);
    void addToSymbolTable(TypeDecl&& decl);
    void addToSymbolTable(EnumDecl& decl);
    void addToSymbolTable(VarDecl& decl, bool global);
    void addToSymbolTable(VarDecl&& decl);
    void addIdentifierReplacement(llvm::StringRef source, llvm::StringRef target);

    static std::vector<Module*> getAllImportedModules();
    static llvm::StringMap<std::shared_ptr<Module>>& getAllImportedModulesMap() { return allImportedModules; }
    static llvm::ArrayRef<std::shared_ptr<Module>> getStdlibModules();

private:
    void addToSymbolTableWithName(Decl& decl, llvm::StringRef name, bool global);
    template<typename DeclT>
    void addToSymbolTableNonAST(DeclT& decl);

private:
    std::string name;
    std::vector<SourceFile> sourceFiles;
    std::vector<std::string> defines;
    SymbolTable symbolTable;
    static llvm::StringMap<std::shared_ptr<Module>> allImportedModules;
};

} // namespace delta
