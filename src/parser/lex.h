#pragma once

#include <memory>
#include <vector>
#include "../ast/token.h"

namespace llvm {
class MemoryBuffer;
}

namespace delta {

struct SourceLocation;

class Lexer {
public:
    Lexer(std::unique_ptr<llvm::MemoryBuffer> input);
    Token nextToken();

private:
    char readChar();
    void unreadChar(char ch);
    void readBlockComment(SourceLocation startLocation);
    Token readQuotedLiteral(char delimiter, Token::Kind literalKind);
    Token readNumber();

private:
    const char* currentFilePosition;
    static std::vector<std::unique_ptr<llvm::MemoryBuffer>> fileBuffers;
};

}
