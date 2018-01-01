#include <algorithm>
#include <cctype>
#pragma warning(push, 0)
#include <llvm/Support/ErrorOr.h>
#include <llvm/Support/Program.h>
#pragma warning(pop)
#include "utility.h"

using namespace delta;

bool delta::treatWarningsAsErrors;

void delta::skipWhitespace(llvm::StringRef& string) {
    string = string.drop_while([](unsigned char c) { return std::isspace(c); });
}

llvm::StringRef delta::readWord(llvm::StringRef& string) {
    auto word = string.take_while([](unsigned char c) { return std::isgraph(c); });
    string = string.drop_front(word.size());
    return word;
}

llvm::StringRef delta::readLine(llvm::StringRef& string) {
    auto word = string.take_while([](unsigned char c) { return c != '\n'; });
    string = string.drop_front(std::min(word.size() + 1, string.size()));
    return word;
}

std::string delta::readLineFromFile(SourceLocation location) {
    std::ifstream file(location.file);

    while (--location.line) {
        file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    std::string lineContent;
    std::getline(file, lineContent);
    return lineContent;
}

void delta::printDiagnostic(SourceLocation location, llvm::StringRef type,
                            llvm::raw_ostream::Colors color, llvm::StringRef message) {
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
    printErrorAndExit("couldn't find C compiler");
}
