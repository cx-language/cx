#include "utility.h"
#include <algorithm>
#include <cctype>
#pragma warning(push, 0)
#include <llvm/Support/ErrorOr.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Program.h>
#pragma warning(pop)

using namespace delta;

WarningMode delta::warningMode = WarningMode::Default;

std::string delta::readLineFromFile(SourceLocation location) {
    std::ifstream file(location.file);

    while (--location.line) {
        file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    std::string lineContent;
    std::getline(file, lineContent);
    return lineContent;
}

void delta::renameFile(llvm::Twine sourcePath, llvm::Twine targetPath) {
    if (auto error = llvm::sys::fs::rename(sourcePath, targetPath)) {
        printErrorAndExit("couldn't rename '", sourcePath, "' to '", targetPath, "': ", error.message());
    }
}

void delta::printDiagnostic(SourceLocation location, llvm::StringRef type, llvm::raw_ostream::Colors color,
                            llvm::StringRef message) {
    if (llvm::outs().has_colors()) {
        llvm::outs().changeColor(llvm::raw_ostream::SAVEDCOLOR, true);
    }

    if (location.file && *location.file) {
        llvm::outs() << location.file;
        if (location.isValid()) {
            llvm::outs() << ':' << location.line << ':' << location.column;
        }
        llvm::outs() << ": ";
    }

    printColored(type, color);
    printColored(": ", color);
    printColored(message, llvm::raw_ostream::SAVEDCOLOR);

    if (location.file && *location.file && location.isValid()) {
        auto line = readLineFromFile(location);
        llvm::outs() << '\n' << line << '\n';

        for (char ch : line.substr(0, location.column - 1)) {
            llvm::outs() << (ch != '\t' ? ' ' : '\t');
        }
        printColored('^', llvm::raw_ostream::GREEN);
    }

    llvm::outs() << '\n';
}

void CompileError::print() const {
    printDiagnostic(location, "error", llvm::raw_ostream::RED, message);

    for (auto& note : notes) {
        printDiagnostic(note.getLocation(), "note", llvm::raw_ostream::BLACK, note.getMessage());
    }

    llvm::outs().flush();
}

std::string delta::getCCompilerPath() {
    for (const char* compiler : { "cc", "gcc", "clang", "cl.exe" }) {
        if (auto path = llvm::sys::findProgramByName(compiler)) {
            return std::move(*path);
        }
    }
    return "";
}
