#pragma once

#include <unordered_map>

namespace delta {

struct Type;

template<typename T>
std::vector<T> instantiate(llvm::ArrayRef<T> elements, const std::unordered_map<std::string, Type>& genericArgs) {
    return map(elements, [&](const T& element) { return element->instantiate(genericArgs); });
}

} // namespace delta
