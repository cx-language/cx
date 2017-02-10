#pragma once

#include <climits>

struct SrcLoc {
    short line;
    short column;

    SrcLoc(short line, short column) : line(line), column(column) { }
    static SrcLoc invalid() { return SrcLoc(SHRT_MIN, SHRT_MIN); }
    bool isValid() const { return line > 0 && column > 0; }
};
