#include <iostream>
#include <cstdio>
#include <vector>
#include <algorithm>
#include <boost/utility/string_ref.hpp>
#include "../ast/ast_printer.h"
#include "../ast/decl.h"
#include "../parser/parser.hpp"

extern FILE* yyin;
int yyparse();
extern std::vector<Decl> globalAST;

/// If `args` contains `flag`, removes it and returns true, otherwise returns false.
bool checkFlag(boost::string_ref flag, std::vector<boost::string_ref>& args) {
    const auto it = std::find(args.begin(), args.end(), flag);
    const bool contains = it != args.end();
    if (contains) args.erase(it);
    return contains;
}

int main(int argc, char** argv) {
    if (argc == 1) {
        std::cout << "error: no input files" << std::endl;
        return 1;
    }

    std::vector<boost::string_ref> args(argv + 1, argv + argc);
    const bool printAST = checkFlag("-print-ast", args);

    for (boost::string_ref filePath : args) {
        yyin = fopen(filePath.data(), "rb");

        if (!yyin) {
            std::cout << "error: no such file: '"  << filePath << "'" << std::endl;
            return 1;
        }

        int result = yyparse();
        if (result != 0) return result;

        if (printAST) {
            std::cout << globalAST;
        }
        globalAST.clear();
    }
}
