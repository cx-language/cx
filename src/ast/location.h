#pragma once

#include <limits>
#pragma warning(push, 0)
#include <llvm/Support/raw_ostream.h>
#pragma warning(pop)

namespace cx {

// A location in source code.
struct Location {
    using IntegerType = short;

    const char* file = nullptr;
    IntegerType line = std::numeric_limits<IntegerType>::min();
    IntegerType column = std::numeric_limits<IntegerType>::min();

    Location nextColumn() const { return Location(file, line, column + 1); }
    bool isValid() const { return line > 0 && column > 0; }

    bool print() const {
        if (file && *file) {
            // Directory builds discover sources as ./file.cx; omit the leading ./ for readability.
            llvm::StringRef path(file);
            while (path.consume_front("./") || path.consume_front(".\\")) {
            }
            llvm::errs() << path;
            if (isValid()) {
                llvm::errs() << ':' << line << ':' << column;
            }
            return true;
        }
        return false;
    }
};

} // namespace cx
