#pragma once

#include <fstream>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/raw_ostream.h>
#include "../ast/srcloc.h"

namespace delta {

inline std::ostream& operator<<(std::ostream& stream, llvm::StringRef string) {
    return stream.write(string.data(), string.size());
}

template<typename... Args>
[[noreturn]] inline void printErrorAndExit(Args&&... args) {
    if (llvm::outs().has_colors())
        llvm::outs().changeColor(llvm::raw_ostream::RED, true);

    llvm::outs() << "error: ";

    if (llvm::outs().has_colors())
        llvm::outs().resetColor().changeColor(llvm::raw_ostream::SAVEDCOLOR, true);

    using expander = int[];
    (void)expander{0, (void(llvm::outs() << std::forward<Args>(args)), 0)...};

    if (llvm::outs().has_colors())
        llvm::outs().resetColor();

    llvm::outs() << '\n';
    exit(1);
}

template<typename... Args>
[[noreturn]] inline void error(SrcLoc srcLoc, Args&&... args) {
    if (llvm::outs().has_colors())
        llvm::outs().changeColor(llvm::raw_ostream::SAVEDCOLOR, true);

    if (srcLoc.file) {
        llvm::outs() << srcLoc.file << ':';
        if (srcLoc.isValid()) llvm::outs() << srcLoc.line << ':' << srcLoc.column << ':';
    } else {
        llvm::outs() << "<unknown file>:";
    }

    if (llvm::outs().has_colors())
        llvm::outs().changeColor(llvm::raw_ostream::RED, true);

    llvm::outs() << " error: ";

    if (llvm::outs().has_colors())
        llvm::outs().resetColor().changeColor(llvm::raw_ostream::SAVEDCOLOR, true);

    using expander = int[];
    (void)expander{0, (void(llvm::outs() << std::forward<Args>(args)), 0)...};

    if (llvm::outs().has_colors())
        llvm::outs().resetColor();

    if (srcLoc.file && srcLoc.isValid()) {
        // Output caret.
        std::ifstream file(srcLoc.file);
        while (--srcLoc.line) file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::string line;
        std::getline(file, line);
        llvm::outs() << '\n' << line << '\n';
        for (char ch : line.substr(0, srcLoc.column - 1))
            llvm::outs() << (ch != '\t' ? ' ' : '\t');

        if (llvm::outs().has_colors())
            llvm::outs().changeColor(llvm::raw_ostream::GREEN, true);

        llvm::outs() << '^';

        if (llvm::outs().has_colors())
            llvm::outs().resetColor();
    }

    llvm::outs() << '\n';
    exit(1);
}

[[noreturn]] inline void fatalError(const char* message) {
    llvm::errs() << "FATAL ERROR: " << message << '\n';
    abort();
}

}
