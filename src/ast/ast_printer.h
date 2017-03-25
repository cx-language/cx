#pragma once

#include <ostream>

namespace delta {

class Module;

std::ostream& operator<<(std::ostream& out, const Module& module);

}
