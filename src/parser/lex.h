#pragma once

namespace llvm {
    class StringRef;
}

namespace delta {

struct Token;

void initLexer(llvm::StringRef filePath);
Token lex();

}
