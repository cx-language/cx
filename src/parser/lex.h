#pragma once

#include <memory>

namespace llvm {
class MemoryBuffer;
}

namespace delta {

struct Token;

void initLexer(std::unique_ptr<llvm::MemoryBuffer> input);
Token lex();

}
