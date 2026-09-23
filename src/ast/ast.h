#pragma once

#pragma warning(push, 0)
#include <llvm/ADT/StringMap.h>
#pragma warning(pop)

namespace cx {

struct Type;

template<typename T> std::vector<T> instantiate(const std::vector<T>& elements, const llvm::StringMap<GenericArg>& genericArgs) {
    return map(elements, [&](const T& element) { return element->instantiate(genericArgs); });
}

} // namespace cx
