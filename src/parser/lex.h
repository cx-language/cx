#pragma once

#include "../ast/token.h"
#include <vector>
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
    // Suspended string content: the lexer returns to it after an interpolation.
    struct StringFrame {
        char delimiter;
        // Start of the current chunk content (quote-free), set when resuming.
        const char* contentBegin = nullptr;
        // Opening quote, for error columns.
        const char* quotePos = nullptr;
    };
    // An in-progress `${...}` or `$identifier` code span.
    struct CodeFrame {
        bool braceForm = false;
        int braceDepth = 0;
        // `$identifier` ends after a single token.
        bool firstTokenDone = false;
    };
    struct LexFrame {
        bool isCode = false;
        StringFrame stringFrame;
        CodeFrame codeFrame;
    };

    Location getCurrentLocation() const;
    char readChar();
    void unreadChar(char ch);
    void readBlockComment(Location startLocation);
    Token readQuotedLiteral(char delimiter, Token::Kind literalKind);
    Token readNumber();
    // Lexes one token when no suspended state remains.
    Token lexToken();
    // Resumes a suspended string after an interpolation.
    Token lexStringResume();
    // Lexes one token inside an interpolation code span.
    Token lexCodeToken();

    llvm::MemoryBufferRef buffer;
    const char* currentFilePosition;
    Location firstLocation;
    Location lastLocation;
    // Interleaved string/code frames; back() is innermost.
    std::vector<LexFrame> frameStack;
    // Set when a string loop stops at an interpolation trigger.
    bool pendingInterpStart = false;
    bool pendingInterpBraceForm = false;
};

} // namespace cx
