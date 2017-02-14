#pragma once

#include <vector>

namespace delta {

using AST = std::vector<class Decl>;

std::ostream& operator<<(std::ostream& out, const AST& ast);

}
