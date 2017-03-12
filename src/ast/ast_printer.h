#pragma once

#include <vector>

namespace delta {

class Decl;

std::ostream& operator<<(std::ostream& out, const std::vector<std::unique_ptr<Decl>>& ast);

}
