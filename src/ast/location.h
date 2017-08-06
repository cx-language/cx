#pragma once

#include <climits>

namespace delta {

struct SourceLocation {
    const char* file;
    short line;
    short column;

    SourceLocation(const char* file, short line, short column) : file(file), line(line), column(column) {}
    static SourceLocation invalid() { return SourceLocation(nullptr, SHRT_MIN, SHRT_MIN); }
    bool isValid() const { return line > 0 && column > 0; }
};

}
