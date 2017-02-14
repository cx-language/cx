#pragma once

#include <climits>

namespace delta {

struct SrcLoc {
    const char* file;
    short line;
    short column;

    SrcLoc(const char* file, short line, short column) : file(file), line(line), column(column) { }
    static SrcLoc invalid() { return SrcLoc(nullptr, SHRT_MIN, SHRT_MIN); }
    bool isValid() const { return line > 0 && column > 0; }
};

}
