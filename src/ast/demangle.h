#pragma once

#include <string>

namespace llvm {
class StringRef;
}

namespace cx {

// Demangles a _CX1 symbol to human-readable form (see mangle.h for the
// grammar), e.g. "main.C.foo(int32) -> void". Returns the input unchanged
// when it is not a demangleable cx symbol. Never fails otherwise.
std::string demangle(llvm::StringRef symbol);
// Demangles every _CX1 symbol token in the line, leaving other text as-is,
// so crash logs can be piped straight through.
std::string demangleLine(llvm::StringRef line);

} // namespace cx
