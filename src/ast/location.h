#pragma once

#include <limits>

namespace delta {

struct SourceLocation {
    using IntegerType = short;

    const char* file;
    IntegerType line;
    IntegerType column;

    SourceLocation(const char* file, IntegerType line, IntegerType column) : file(file), line(line), column(column) {}

    static SourceLocation invalid() {
        auto min = std::numeric_limits<IntegerType>::min();
        return SourceLocation(nullptr, min, min);
    }

    SourceLocation nextColumn() const { return SourceLocation(file, line, column + 1); }
    bool isValid() const { return line > 0 && column > 0; }
};

}
