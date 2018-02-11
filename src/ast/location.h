#pragma once

#include <limits>

namespace delta {

struct SourceLocation {
    using IntegerType = short;

    const char* file;
    IntegerType line;
    IntegerType column;

    SourceLocation()
    : SourceLocation(nullptr, std::numeric_limits<IntegerType>::min(), std::numeric_limits<IntegerType>::min()) {}
    SourceLocation(const char* file, IntegerType line, IntegerType column) : file(file), line(line), column(column) {}
    SourceLocation nextColumn() const { return SourceLocation(file, line, column + 1); }
    bool isValid() const { return line > 0 && column > 0; }
};

}
