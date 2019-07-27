#pragma once

#include <string>
#include <vector>

namespace delta {

struct CompileOptions {
    std::vector<std::string> disabledWarnings;
    std::vector<std::string> importSearchPaths;
    std::vector<std::string> frameworkSearchPaths;
    std::vector<std::string> defines;
    std::vector<std::string> cflags;
};

} // namespace delta
