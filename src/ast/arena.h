#pragma once

#include <new>
#include <utility>
#pragma warning(push, 0)
#include <llvm/Support/Allocator.h>
#pragma warning(pop)

namespace cx {

// Owns all AST nodes (Expr, Stmt, Decl, TypeBase), freed in bulk at process exit.
// Never `delete` a pointer returned from here; destructors never run.
// Not thread-safe; the compiler is single-threaded.
inline llvm::BumpPtrAllocator& astAllocator() {
    static llvm::BumpPtrAllocator allocator;
    return allocator;
}

/// Allocates an AST node of type T from the global AST arena.
template<typename T, typename... Args> T* makeAST(Args&&... args) {
    void* mem = astAllocator().Allocate(sizeof(T), alignof(T));
    return new (mem) T(std::forward<Args>(args)...);
}

} // namespace cx
