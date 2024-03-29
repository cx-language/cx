#pragma once

#include <string>
#include <vector>

namespace cx {

struct CompileOptions {
    bool noUnusedWarnings;
    std::vector<std::string> importSearchPaths;
    std::vector<std::string> frameworkSearchPaths;
    std::vector<std::string> defines;
    std::vector<std::string> cflags;
};

} // namespace cx
