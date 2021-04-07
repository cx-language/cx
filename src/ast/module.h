#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#pragma warning(push, 0)
#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringRef.h>
#pragma warning(pop)
#include "decl.h"

namespace delta {

struct Module;
struct SymbolTable;

/// Container for the AST of a single file.
struct SourceFile {
    explicit SourceFile(llvm::StringRef filePath, Module* parentModule) : filePath(filePath), parentModule(parentModule) {}
    llvm::ArrayRef<Decl*> getTopLevelDecls() const { return topLevelDecls; }
    llvm::StringRef getFilePath() const { return filePath; }
    Module* getModule() const { return parentModule; }
    llvm::ArrayRef<Module*> getImportedModules() const { return importedModules; }
    void setDecls(std::vector<Decl*>&& decls) { topLevelDecls = std::move(decls); }

    void addImportedModule(Module* module) {
        if (!llvm::is_contained(importedModules, module)) {
            importedModules.push_back(module);
        }
    }

private:
    std::string filePath;
    Module* parentModule;
    std::vector<Decl*> topLevelDecls;
    std::vector<Module*> importedModules;
};

struct Scope {
    Decl* parent;
    SymbolTable* symbolTable;
    std::unordered_map<std::string, std::vector<Decl*>> decls;

    Scope(Decl* parent, SymbolTable* symbolTable);
    ~Scope();
};

struct SymbolTable {
    SymbolTable() : globalScope(nullptr, this) {}
    Scope& getCurrentScope() { return *scopes.back(); }
    void add(llvm::StringRef name, Decl* decl) { scopes.back()->decls[name.str()].push_back(decl); }
    void addGlobal(llvm::StringRef name, Decl* decl) { scopes.front()->decls[name.str()].push_back(decl); }
    void addIdentifierReplacement(llvm::StringRef name, llvm::StringRef replacement) { identifierReplacements.try_emplace(name.str(), replacement); }

    llvm::ArrayRef<Decl*> find(llvm::StringRef name) const {
        auto realName = applyIdentifierReplacements(name);
        for (auto& scope : llvm::reverse(scopes)) {
            auto it = scope->decls.find(realName.str());
            if (it != scope->decls.end()) return it->second;
        }
        return {};
    }

    Decl* findOne(llvm::StringRef name) const {
        auto results = find(name);
        if (results.empty()) return nullptr;
        ASSERT(results.size() == 1);
        return results.front();
    }

    llvm::ArrayRef<Decl*> findInCurrentScope(llvm::StringRef name) const {
        if (!scopes.empty()) {
            auto it = scopes.back()->decls.find(applyIdentifierReplacements(name).str());
            if (it != scopes.back()->decls.end()) return it->second;
        }
        return {};
    }

    FunctionDecl* findWithMatchingPrototype(const FunctionDecl& toFind) const {
        for (Decl* decl : find(toFind.getQualifiedName())) {
            if (auto* functionDecl = llvm::dyn_cast<FunctionDecl>(decl)) {
                if (functionDecl->getParams().size() == toFind.getParams().size() &&
                    std::equal(toFind.getParams().begin(), toFind.getParams().end(), functionDecl->getParams().begin(), paramsMatch)) {
                    return functionDecl;
                }
            }
        }
        return nullptr;
    }

private:
    friend struct Scope;
    void pushScope(Scope& scope) { scopes.push_back(&scope); }
    void popScope() { scopes.pop_back(); }

    static bool paramsMatch(const ParamDecl& a, const ParamDecl& b) {
        if (a.getType() != b.getType()) return false;
        if (a.isPublic && b.isPublic && a.getName() != b.getName()) return false;
        return true;
    }

    llvm::StringRef applyIdentifierReplacements(llvm::StringRef name) const {
        llvm::StringRef initialName = name;
        while (true) {
            auto it = identifierReplacements.find(name.str());
            if (it == identifierReplacements.end()) return name;
            if (it->second == initialName) return name; // Break replacement cycle.
            name = it->second;
        }
    }

    std::vector<Scope*> scopes;
    Scope globalScope;
    std::unordered_map<std::string, std::string> identifierReplacements;
};

/// Container for the AST of a whole module, comprised of one or more SourceFiles.
struct Module {
    Module(llvm::StringRef name) : name(name) {}
    void addSourceFile(SourceFile&& file) { sourceFiles.emplace_back(std::move(file)); }
    llvm::ArrayRef<SourceFile> getSourceFiles() const { return sourceFiles; }
    llvm::MutableArrayRef<SourceFile> getSourceFiles() { return sourceFiles; }
    llvm::StringRef getName() const { return name; }
    SymbolTable& getSymbolTable() { return symbolTable; }

    std::vector<Module*> getImportedModules() const {
        std::vector<Module*> importedModules;
        for (auto& sourceFile : getSourceFiles()) {
            for (auto& importedModule : sourceFile.getImportedModules()) {
                importedModules.push_back(importedModule);
            }
        }
        return importedModules;
    }

    void addToSymbolTable(FunctionTemplate& decl);
    void addToSymbolTable(FunctionDecl& decl);
    void addToSymbolTable(TypeTemplate& decl);
    void addToSymbolTable(TypeDecl& decl);
    void addToSymbolTable(EnumDecl& decl);
    void addToSymbolTable(VarDecl& decl);
    void addToSymbolTable(Decl* decl);
    void addIdentifierReplacement(llvm::StringRef source, llvm::StringRef target);

    static std::vector<Module*> getAllImportedModules();
    static std::unordered_map<std::string, Module*>& getAllImportedModulesMap() { return allImportedModules; }
    static Module* getStdlibModule();

private:
    void addToSymbolTableWithName(Decl& decl, llvm::StringRef name);

private:
    std::string name;
    std::vector<SourceFile> sourceFiles;
    SymbolTable symbolTable;
    static std::unordered_map<std::string, Module*> allImportedModules;
};

} // namespace delta
