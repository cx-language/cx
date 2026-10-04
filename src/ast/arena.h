#pragma once

#include <new>
#include <type_traits>
#include <utility>
#include <vector>
#pragma warning(push, 0)
#include <llvm/Support/Allocator.h>
#include <llvm/Support/StringSaver.h>
#pragma warning(pop)

namespace cx {

// Owns all AST nodes (Expr, Stmt, Decl, TypeBase) and interned strings, freed in bulk at process exit
// (or by resetAstArena for the language server). Never `delete` a pointer returned from here.
// Not thread-safe; the compiler is single-threaded.
inline llvm::BumpPtrAllocator& astAllocator() {
    static llvm::BumpPtrAllocator allocator;
    return allocator;
}

inline llvm::UniqueStringSaver& stringSaver() {
    static auto* saver = new llvm::UniqueStringSaver(astAllocator());
    return *saver;
}

/// Returns process-lifetime storage for a canonical copy of value.
inline llvm::StringRef internString(llvm::StringRef value) {
    return stringSaver().save(value);
}

struct AstDtorEntry {
    void* ptr;
    void (*destroy)(void*);
};

/// Every non-trivial arena node, for resetAstArena. Nodes own malloc'd
/// memory (vectors, strings) that slab freeing alone would leak.
inline std::vector<AstDtorEntry>& astDtorRegistry() {
    static auto* registry = new std::vector<AstDtorEntry>();
    return *registry;
}

/// Frees all AST nodes and interned strings. Every pointer returned from this
/// header dangles afterwards; the caller must have dropped them all (the LSP
/// session drops its cached modules first). The allocator is reusable right
/// after: the next compilation interns from scratch, exactly like a fresh
/// process would.
inline void resetAstArena() {
    auto& registry = astDtorRegistry();
    for (auto it = registry.rbegin(); it != registry.rend(); ++it) {
        it->destroy(it->ptr);
    }
    registry.clear();
    // Destroying the saver only frees its own DenseSet buckets (malloced);
    // the interned bytes live in the allocator slabs freed below.
    auto* saver = &stringSaver();
    saver->~UniqueStringSaver();
    astAllocator().Reset();
    new (saver) llvm::UniqueStringSaver(astAllocator());
}

/// Allocates an AST node of type T from the global AST arena.
template<typename T, typename... Args> T* makeAST(Args&&... args) {
    void* mem = astAllocator().Allocate(sizeof(T), alignof(T));
    T* node = new (mem) T(std::forward<Args>(args)...);
    if constexpr (!std::is_trivially_destructible_v<T>) {
        astDtorRegistry().push_back({node, [](void* ptr) { static_cast<T*>(ptr)->~T(); }});
    }
    return node;
}

} // namespace cx
