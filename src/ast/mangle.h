#pragma once

#include <string>

namespace cx {

struct FunctionDecl;

std::string mangleFunctionDecl(const FunctionDecl& functionDecl);

} // namespace cx
