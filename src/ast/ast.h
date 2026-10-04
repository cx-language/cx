#pragma once

#include "arena.h"
#pragma warning(push, 0)
#include <llvm/ADT/StringMap.h>
#pragma warning(pop)

namespace cx {

struct Type;
struct GenericArg;

/// Like map(), but the result lives in the AST arena. For building AST node
/// members; temporaries that outlive the arena (or feed malloc owners) keep
/// using map().
template<typename SourceContainer, typename Mapper> auto mapAst(const SourceContainer& source, Mapper mapper) -> AstVector<decltype(mapper(*source.begin()))> {
    AstVector<decltype(mapper(*source.begin()))> result;
    result.reserve(source.size());
    for (auto& element : source) {
        result.emplace_back(mapper(element));
    }
    return result;
}

template<typename T> AstVector<T> instantiate(const AstVector<T>& elements, const llvm::StringMap<GenericArg>& genericArgs) {
    return mapAst(elements, [&](const T& element) { return element->instantiate(genericArgs); });
}

} // namespace cx
