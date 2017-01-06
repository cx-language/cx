#include <iostream>
#include <cstdio>
#include <vector>
#include "../ast/ast_printer.h"
#include "../ast/decl.h"
#include "parser.hpp"

extern FILE* yyin;
int yyparse();
extern std::vector<Decl> globalAST;

int main(int argc, char** argv) {
    if (argc == 1) {
        std::cout << "error: no input files" << std::endl;
        return 1;
    }

    for (int i = 1; i < argc; ++i) {
        yyin = fopen(argv[i], "rb");

        if (!yyin) {
            std::cout << "error: no such file: '"  << argv[i] << "'" << std::endl;
            return 1;
        }

        int result = yyparse();
        if (result != 0) return result;

        std::cout << globalAST;
        globalAST.clear();
    }
}
