#pragma once

#include <iostream>
#include <vector>
#include <memory>

namespace delta {

class Decl;

std::ostream& operator<<(std::ostream& out, const std::vector<std::unique_ptr<Decl>>& ast);

}
