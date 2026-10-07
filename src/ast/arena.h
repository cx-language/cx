#pragma once

#include <limits>
#include <new>
#include <stdexcept>
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

/// Frees all AST nodes and interned strings. Node destructors never run, so
/// nodes must not own malloc'd memory: dynamic members use AstVector and
/// interned StringRefs. A malloc'd member would leak silently at reset; the
/// LSan CI gate (test/lsp/check_lsan.py over repeated LSP analyses) fails
/// on per-reset growth, so run it when adding node members. Every pointer
/// returned from this header dangles afterwards; the caller must have
/// dropped them all (the LSP session drops its cached modules first). The
/// allocator is reusable right after: the next compilation interns from
/// scratch, exactly like a fresh process would.
inline void resetAstArena() {
    // Destroying the saver only frees its own DenseSet buckets (malloced);
    // the interned bytes live in the allocator slabs freed below.
    auto* saver = &stringSaver();
    saver->~UniqueStringSaver();
    astAllocator().Reset();
    new (saver) llvm::UniqueStringSaver(astAllocator());
}

/// std allocator over the AST arena: allocate() bumps, deallocate() is a
/// no-op (resetAstArena frees in bulk). Stateless: all instances compare
/// equal, so moves transfer buffers and copies deep-copy into the arena.
template<typename T> struct AstAllocator {
    using value_type = T;
    using is_always_equal = std::true_type;
    using propagate_on_container_move_assignment = std::true_type;

    template<typename U> struct rebind {
        using other = AstAllocator<U>;
    };

    AstAllocator() = default;
    template<typename U> AstAllocator(const AstAllocator<U>&) {}

    T* allocate(size_t count) {
        if (count > max_size()) throw std::length_error("AstAllocator: too many elements");
        if (count == 0) return nullptr;
        return static_cast<T*>(astAllocator().Allocate(count * sizeof(T), alignof(T)));
    }
    void deallocate(T*, size_t) {}
    size_t max_size() const { return std::numeric_limits<size_t>::max() / sizeof(T); }

    bool operator==(const AstAllocator&) const { return true; }
};

/// std::vector over the AST arena. For AST node members: node and elements
/// stay adjacent in the slab, and resetAstArena frees every buffer in bulk.
/// Buffers are never freed individually: growth abandons the old buffer
/// (bounded by the geometric series). Element pointers are invalidated by
/// growth like std::vector, and dangle at the next resetAstArena; never store
/// them past it. Not thread-safe, like the rest of the compiler.
template<typename T> using AstVector = std::vector<T, AstAllocator<T>>;

/// Allocates an AST node of type T from the global AST arena. The destructor
/// never runs (see resetAstArena), so T must not own malloc'd memory.
template<typename T, typename... Args> T* makeAST(Args&&... args) {
    void* mem = astAllocator().Allocate(sizeof(T), alignof(T));
    return new (mem) T(std::forward<Args>(args)...);
}

} // namespace cx
