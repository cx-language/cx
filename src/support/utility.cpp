#include "utility.h"

using namespace delta;

void CompileError::print() const {
    if (llvm::outs().has_colors()) {
        llvm::outs().changeColor(llvm::raw_ostream::SAVEDCOLOR, true);
    }

    if (srcLoc.file && *srcLoc.file) {
        llvm::outs() << srcLoc.file;
        if (srcLoc.isValid()) llvm::outs() << ':' << srcLoc.line << ':' << srcLoc.column;
        llvm::outs() << ": ";
    }

    printColored("error: ", llvm::raw_ostream::RED);
    printColored(message, llvm::raw_ostream::SAVEDCOLOR);

    if (srcLoc.file && *srcLoc.file && srcLoc.isValid()) {
        std::ifstream file(srcLoc.file);
        for (auto line = srcLoc.line; --line;) {
            file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }

        std::string line;
        std::getline(file, line);
        llvm::outs() << '\n' << line << '\n';

        for (char ch : line.substr(0, srcLoc.column - 1)) {
            llvm::outs() << (ch != '\t' ? ' ' : '\t');
        }
        printColored('^', llvm::raw_ostream::GREEN);
    }

    llvm::outs() << '\n';
}
