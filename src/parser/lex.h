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
    const char* getFilePath() const;

    static std::vector<std::unique_ptr<llvm::MemoryBuffer>> fileBuffers; // TODO: Make this non-static.

private:
    SourceLocation getCurrentLocation() const;
    char readChar();
    void unreadChar(char ch);
    void readBlockComment(SourceLocation startLocation);
    Token readQuotedLiteral(char delimiter, Token::Kind literalKind);
    Token readNumber();

    const char* currentFilePosition;
    SourceLocation firstLocation;
    SourceLocation lastLocation;
};

} // namespace delta
