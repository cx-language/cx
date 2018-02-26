#pragma once

#include <string>

namespace delta {

class FunctionDecl;

std::string mangleFunctionDecl(const FunctionDecl& functionDecl);

} // namespace delta
