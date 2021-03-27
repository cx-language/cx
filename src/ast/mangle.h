#pragma once

#include <string>

namespace delta {

struct FunctionDecl;

std::string mangleFunctionDecl(const FunctionDecl& functionDecl);

} // namespace delta
