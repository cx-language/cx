#pragma once

#include "../ast/token.h"
#pragma warning(push, 0)
#include <llvm/Support/MemoryBuffer.h>
#pragma warning(pop)

namespace cx {

struct Location;

struct Lexer {
    Lexer(llvm::MemoryBufferRef input);
    Token nextToken();
    const char* getFilePath() const;

private:
    Location getCurrentLocation() const;
    char readChar();
    void unreadChar(char ch);
    void readBlockComment(Location startLocation);
    Token readQuotedLiteral(char delimiter, Token::Kind literalKind);
    Token readNumber();

    llvm::MemoryBufferRef buffer;
    const char* currentFilePosition;
    Location firstLocation;
    Location lastLocation;
};

} // namespace cx
