#pragma once

#pragma warning(push, 0)
#include <llvm/ADT/StringMap.h>
#pragma warning(pop)

namespace delta {

struct Type;

template<typename T>
std::vector<T> instantiate(llvm::ArrayRef<T> elements, const llvm::StringMap<Type>& genericArgs) {
    return map(elements, [&](const T& element) { return element->instantiate(genericArgs); });
}

} // namespace delta
