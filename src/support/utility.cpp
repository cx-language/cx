#include "utility.h"

using namespace delta;

void CompileError::print() const {
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

    printColored("error: ", llvm::raw_ostream::RED);
    printColored(message, llvm::raw_ostream::SAVEDCOLOR);

    if (location.file && *location.file && location.isValid()) {
        std::ifstream file(location.file);
        for (auto line = location.line; --line;) {
            file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }

        std::string line;
        std::getline(file, line);
        llvm::outs() << '\n' << line << '\n';

        for (char ch : line.substr(0, location.column - 1)) {
            llvm::outs() << (ch != '\t' ? ' ' : '\t');
        }
        printColored('^', llvm::raw_ostream::GREEN);
    }

    llvm::outs() << '\n';
}
