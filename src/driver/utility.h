#pragma once

#include <fstream>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/raw_ostream.h>
#include "../ast/srcloc.h"

namespace delta {

class CompileError {
public:
    CompileError(SrcLoc srcLoc, std::string&& message) : srcLoc(srcLoc), message(std::move(message)) { }
    void print() const;

private:
    SrcLoc srcLoc;
    std::string message;
};

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
    std::string message;
    llvm::raw_string_ostream messageStream(message);
    using expander = int[];
    (void)expander{0, (void(messageStream << std::forward<Args>(args)), 0)...};
    throw CompileError(srcLoc, std::move(messageStream.str()));
}

inline void CompileError::print() const {
    if (llvm::outs().has_colors())
        llvm::outs().changeColor(llvm::raw_ostream::SAVEDCOLOR, true);

    if (srcLoc.file && *srcLoc.file) {
        llvm::outs() << srcLoc.file;
        if (srcLoc.isValid()) llvm::outs() << ':' << srcLoc.line << ':' << srcLoc.column;
        llvm::outs() << ": ";
    }

    if (llvm::outs().has_colors())
        llvm::outs().changeColor(llvm::raw_ostream::RED, true);

    llvm::outs() << "error: ";

    if (llvm::outs().has_colors())
        llvm::outs().resetColor().changeColor(llvm::raw_ostream::SAVEDCOLOR, true);

    llvm::outs() << message;

    if (llvm::outs().has_colors())
        llvm::outs().resetColor();

    if (srcLoc.file && *srcLoc.file && srcLoc.isValid()) {
        // Output caret.
        std::ifstream file(srcLoc.file);
        int lineNumber = srcLoc.line;
        while (--lineNumber) file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

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
}

[[noreturn]] inline void fatalError(const char* message) {
    llvm::errs() << "FATAL ERROR: " << message << '\n';
    abort();
}

}
