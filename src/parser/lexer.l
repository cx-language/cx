%{
    #include <iostream>
    #include <cstdlib>
    #include <vector>
    #include "../ast/expr.h"
    #include "../ast/decl.h"
    #include "parser.hpp"

    #define SAVE_STRING yylval.string = strndup(yytext, yyleng)
    #define SAVE_STRING_LITERAL yylval.string = strndup(yytext + 1, yyleng - 2)
    void saveNumber(int base) {
        switch (yytext[1]) {
            case 'b': yylval.number = strtoll(yytext + 2, 0, base); break;
            case 'o': yylval.number = strtoll(yytext + 2, 0, base); break;
            case 'x': yylval.number = strtoll(yytext + 2, 0, base); break;
            default:  yylval.number = strtoll(yytext,     0, base); break;
        }
    }
    #define TOKEN(token) token

    #pragma GCC diagnostic ignored "-Wdeprecated-register"
    #pragma GCC diagnostic ignored "-Wunused-function"
    #pragma GCC diagnostic ignored "-Wunneeded-internal-declaration"
%}

%option noyywrap

%%

"cast"                  return TOKEN(CAST);
"class"                 return TOKEN(CLASS);
"const"                 return TOKEN(CONST);
"else"                  return TOKEN(ELSE);
"extern"                return TOKEN(EXTERN);
"false"                 return TOKEN(FALSE);
"func"                  return TOKEN(FUNC);
"if"                    return TOKEN(IF);
"init"                  return TOKEN(INIT);
"mutable"               return TOKEN(MUTABLE);
"return"                return TOKEN(RETURN);
"struct"                return TOKEN(STRUCT);
"true"                  return TOKEN(TRUE);
"var"                   return TOKEN(VAR);
"while"                 return TOKEN(WHILE);

[ \t\n]+                ; // skip whitespace
"//".*                  ; // skip line comments
[a-zA-Z_][a-zA-Z_0-9]*  SAVE_STRING; return IDENTIFIER;
\"[^\"]*\"              SAVE_STRING_LITERAL; return STRING_LITERAL;
0b[0-1]*                saveNumber(2); return NUMBER;
0o[0-7]*                saveNumber(8); return NUMBER;
0                       saveNumber(10); return NUMBER;
[1-9][0-9]*             saveNumber(10); return NUMBER;
0x[0-9a-f]*             saveNumber(16); return NUMBER;
0x[0-9A-F]*             saveNumber(16); return NUMBER;

"=="                    return TOKEN(EQ);
"!="                    return TOKEN(NE);
"<"                     return TOKEN(LT);
"<="                    return TOKEN(LE);
">"                     return TOKEN(GT);
">="                    return TOKEN(GE);

"="                     return TOKEN(ASSIGN);
"("                     return TOKEN(LPAREN);
")"                     return TOKEN(RPAREN);
"{"                     return TOKEN(LBRACE);
"}"                     return TOKEN(RBRACE);
"."                     return TOKEN(DOT);
","                     return TOKEN(COMMA);
":"                     return TOKEN(COLON);
";"                     return TOKEN(SEMICOLON);
"->"                    return TOKEN(RARROW);
"+"                     return TOKEN(PLUS);
"-"                     return TOKEN(MINUS);
"*"                     return TOKEN(ASTERISK);
"/"                     return TOKEN(SLASH);
"++"                    return TOKEN(INCREMENT);
"--"                    return TOKEN(DECREMENT);
.                       std::cout << "error: unknown token\n"; yyterminate();

%%
