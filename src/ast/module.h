#pragma once

#include <algorithm>
#include <optional>
#include <string>
#include <vector>
#pragma warning(push, 0)
#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringMap.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/MemoryBuffer.h>
#pragma warning(pop)
#include "decl.h"

namespace cx {

struct Module;
struct SymbolTable;

/// Container for the AST of a single file.
struct SourceFile {
    void addImportedModule(Module* module) {
        if (!llvm::is_contained(importedModules, module)) {
            importedModules.push_back(module);
        }
    }

    std::string filePath;
    Module* parentModule;
    std::vector<Decl*> topLevelDecls;
    std::vector<Module*> importedModules;
};

struct Scope {
    Decl* parent;
    SymbolTable* symbolTable;
    llvm::StringMap<std::vector<Decl*>> decls;

    Scope(Decl* parent, SymbolTable* symbolTable);
    ~Scope();
};

struct SymbolTable {
    SymbolTable() : globalScope(nullptr, this) {}
    Scope& getCurrentScope() { return *scopes.back(); }
    void add(llvm::StringRef name, Decl* decl) { scopes.back()->decls[name].push_back(decl); }
    void addGlobal(llvm::StringRef name, Decl* decl) { scopes.front()->decls[name].push_back(decl); }
    void removeGlobal(llvm::StringRef name, Decl* decl) {

        auto it = scopes.front()->decls.find(name);
        if (it == scopes.front()->decls.end()) return;
        auto& decls = it->second;
        decls.erase(std::remove(decls.begin(), decls.end(), decl), decls.end());
    }
    void addIdentifierReplacement(llvm::StringRef name, llvm::StringRef replacement) { identifierReplacements.try_emplace(name, replacement); }

    llvm::ArrayRef<Decl*> findFirst(llvm::StringRef name) const {
        ASSERT(!name.empty());
        auto realName = applyIdentifierReplacements(name);
        for (auto& scope : llvm::reverse(scopes)) {
            auto it = scope->decls.find(realName);
            if (it != scope->decls.end()) return it->second;
        }
        return {};
    }

    Decl* findOne(llvm::StringRef name) const {
        auto results = findFirst(name);
        if (results.empty()) return nullptr;
        ASSERT(results.size() == 1);
        return results.front();
    }

    llvm::SmallVector<Decl*, 8> findInAllScopes(llvm::StringRef name) const {
        ASSERT(!name.empty());
        llvm::SmallVector<Decl*, 8> decls;
        auto realName = applyIdentifierReplacements(name);
        for (auto& scope : llvm::reverse(scopes)) {
            auto it = scope->decls.find(realName);
            if (it != scope->decls.end()) llvm::append_range(decls, it->second);
        }
        return decls;
    }

    llvm::ArrayRef<Decl*> findInTopLevelScope(llvm::StringRef name) const {
        ASSERT(!name.empty());
        auto it = scopes.front()->decls.find(applyIdentifierReplacements(name));
        if (it != scopes.front()->decls.end()) return it->second;
        return {};
    }

    llvm::ArrayRef<Decl*> findInCurrentScope(llvm::StringRef name) const {
        ASSERT(!name.empty());
        if (!scopes.empty()) {
            auto it = scopes.back()->decls.find(applyIdentifierReplacements(name));
            if (it != scopes.back()->decls.end()) return it->second;
        }
        return {};
    }

    FunctionDecl* findWithMatchingPrototype(const FunctionDecl& toFind) const {
        for (Decl* decl : findFirst(toFind.getQualifiedName())) {
            if (auto* functionDecl = llvm::dyn_cast<FunctionDecl>(decl)) {
                if (functionDecl->getParams().size() == toFind.getParams().size()
                    && std::equal(toFind.getParams().begin(), toFind.getParams().end(), functionDecl->getParams().begin(), paramsMatch)) {
                    return functionDecl;
                }
            }
        }
        return nullptr;
    }

    // Hides local scopes so a re-checked default value resolves names as at
    // its declaration (globals only). Scopes pushed inside the window stay
    // visible, and lookup additions land in the seed scope, discarded on exit.
    struct LocalScopeGuard {
        LocalScopeGuard(SymbolTable& table, Decl* thisDecl) : table(table), saved(table.scopes) {
            seed.emplace(nullptr, &table);
            table.scopes.erase(table.scopes.begin() + 1, table.scopes.end() - 1);
            if (thisDecl) table.add("this", thisDecl);
        }
        ~LocalScopeGuard() {
            seed.reset();
            table.scopes = std::move(saved);
        }
        LocalScopeGuard(const LocalScopeGuard&) = delete;
        LocalScopeGuard& operator=(const LocalScopeGuard&) = delete;

    private:
        SymbolTable& table;
        std::vector<Scope*> saved;
        std::optional<Scope> seed;
    };

private:
    friend struct Scope;
    void pushScope(Scope& scope) { scopes.push_back(&scope); }
    void popScope() { scopes.pop_back(); }

    static bool paramsMatch(const ParamDecl& a, const ParamDecl& b) {
        if (a.type != b.type) return false;
        if (a.isPack != b.isPack) return false;
        if (a.isPublic && b.isPublic && a.getName() != b.getName()) return false;
        return true;
    }

    llvm::StringRef applyIdentifierReplacements(llvm::StringRef name) const {
        llvm::StringRef initialName = name;
        while (true) {
            auto it = identifierReplacements.find(name);
            if (it == identifierReplacements.end()) return name;
            if (it->second == initialName) return name; // Break replacement cycle.
            name = it->second;
        }
    }

    std::vector<Scope*> scopes;
    Scope globalScope;
    llvm::StringMap<std::string> identifierReplacements;
};

/// Container for the AST of a whole module, comprised of one or more SourceFiles.
struct Module {
    // Keep this constructor rather than aggregate-initializing: `new Module(...)`
    // would otherwise reference the implicit vector destructor without emitting
    // it, breaking the link (observed in the WebAssembly build).
    Module(std::string&& name) : name(std::move(name)) {}
    void addSourceFile(SourceFile&& file) { sourceFiles.emplace_back(std::move(file)); }

    std::vector<Module*> getImportedModules() const {
        std::vector<Module*> importedModules;
        for (auto& sourceFile : sourceFiles) {
            for (auto& importedModule : sourceFile.importedModules) {
                importedModules.push_back(importedModule);
            }
        }
        return importedModules;
    }

    void addToSymbolTable(FunctionTemplate& decl);
    void addToSymbolTable(FunctionDecl& decl);
    void addToSymbolTable(TypeTemplate& decl);
    void addToSymbolTable(TypeDecl& decl);
    void addToSymbolTable(TypeAliasDecl& decl);
    void addToSymbolTable(EnumDecl& decl);
    void addToSymbolTable(VarDecl& decl);
    void addToSymbolTable(Decl* decl);
    void addIdentifierReplacement(llvm::StringRef source, llvm::StringRef target);
    void print(llvm::raw_ostream& stream) const;

    static std::vector<Module*> getAllImportedModules();
    static Module* findImportedModule(llvm::StringRef name);
    static void registerImportedModule(llvm::StringRef name, Module* module);
    static Module* getStdlibModule();
    /// Forgets all imported modules (their AST is freed by resetAstArena).
    static void resetImportedModules();

private:
    // Returns true when the name was already defined and a redefinition was reported.
    bool addToSymbolTableWithName(Decl& decl, llvm::StringRef name);

    static llvm::StringMap<Module*> allImportedModules;

public:
    std::string name;
    std::vector<SourceFile> sourceFiles;
    bool isCHeaderImport = false;
    bool isCxxHeaderImport = false;
    SymbolTable symbolTable;
    std::vector<std::unique_ptr<llvm::MemoryBuffer>> fileBuffers;
};

} // namespace cx
