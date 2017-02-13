#pragma once

#include <fstream>
#include <llvm/ADT/StringRef.h>

inline std::ostream& operator<<(std::ostream& stream, llvm::StringRef string) {
    return stream.write(string.data(), string.size());
}

template<typename... Args>
[[noreturn]] inline void error(SrcLoc srcLoc, Args&&... args) {
    if (srcLoc.file) {
        std::cout << srcLoc.file << ':';
        if (srcLoc.isValid()) std::cout << srcLoc.line << ':' << srcLoc.column << ':';
    } else {
        std::cout << "<unknown file>:";
    }
    
    std::cout << " error: ";
    using expander = int[];
    (void)expander{0, (void(std::cout << std::forward<Args>(args)), 0)...};

    if (srcLoc.file && srcLoc.isValid()) {
        // Output caret.
        std::ifstream file(srcLoc.file);
        while (--srcLoc.line) file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::string line;
        std::getline(file, line);
        std::cout << '\n' << line << '\n';
        while (--srcLoc.column) std::cout << ' ';
        std::cout << '^';
    }

    std::cout << '\n';
    exit(1);
}
