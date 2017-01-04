%{
    #include <iostream>
    #include <cstdlib>
    #include "parser.hpp"

    #define SAVE_STRING yylval.string = strndup(yytext, yyleng)
    #define SAVE_NUMBER(base) yylval.number = strtoll(yytext, 0, base)
    #define TOKEN(token) token

    #pragma GCC diagnostic ignored "-Wdeprecated-register"
%}

%option noyywrap

%%

"const"                 return TOKEN(CONST);
"else"                  return TOKEN(ELSE);
"extern"                return TOKEN(EXTERN);
"func"                  return TOKEN(FUNC);
"if"                    return TOKEN(IF);
"return"                return TOKEN(RETURN);
"var"                   return TOKEN(VAR);

[ \t\n]+                ; // skip whitespace
[a-zA-Z_][a-zA-Z_0-9]*  SAVE_STRING; return IDENTIFIER;
0b[0-1]*                SAVE_NUMBER(2); return NUMBER;
0o[0-7]*                SAVE_NUMBER(8); return NUMBER;
0                       SAVE_NUMBER(10); return NUMBER;
[1-9][0-9]*             SAVE_NUMBER(10); return NUMBER;
0x[0-9a-f]*             SAVE_NUMBER(16); return NUMBER;
0x[0-9A-F]*             SAVE_NUMBER(16); return NUMBER;

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
