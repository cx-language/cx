#include "utility.h"

using namespace delta;

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
}
