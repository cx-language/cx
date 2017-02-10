#pragma once

#include <llvm/ADT/StringRef.h>

inline std::ostream& operator<<(std::ostream& stream, llvm::StringRef string) {
    return stream.write(string.data(), string.size());
}

template<typename... Args>
[[noreturn]] inline void error(SrcLoc srcLoc, Args&&... args) {
    if (srcLoc.isValid()) std::cout << srcLoc.line << ':' << srcLoc.column << ": ";
    std::cout << "error: ";
    using expander = int[];
    (void)expander{0, (void(std::cout << std::forward<Args>(args)), 0)...};
    std::cout << '\n';
    exit(1);
}
