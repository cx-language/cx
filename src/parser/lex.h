#pragma once

namespace delta {

struct Token;

void initLexer(const char* filePath);
Token lex();

}
