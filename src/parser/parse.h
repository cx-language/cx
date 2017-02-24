#pragma once

#include <vector>
#include <memory>

namespace delta {

class Decl;

void parse(const char* filePath, std::vector<std::unique_ptr<Decl>>& outputAST);

}
