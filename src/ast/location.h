#pragma once

#include <limits>
#pragma warning(push, 0)
#include <llvm/Support/raw_ostream.h>
#pragma warning(pop)

namespace delta {

struct SourceLocation {
    using IntegerType = short;

    const char* file;
    IntegerType line;
    IntegerType column;

    SourceLocation() : SourceLocation(nullptr, std::numeric_limits<IntegerType>::min(), std::numeric_limits<IntegerType>::min()) {}
    SourceLocation(const char* file, IntegerType line, IntegerType column) : file(file), line(line), column(column) {}
    SourceLocation nextColumn() const { return SourceLocation(file, line, column + 1); }
    bool isValid() const { return line > 0 && column > 0; }

    bool print() const {
        if (file && *file) {
            llvm::outs() << file;
            if (isValid()) {
                llvm::outs() << ':' << line << ':' << column;
            }
            return true;
        }
        return false;
    }
};

} // namespace delta
