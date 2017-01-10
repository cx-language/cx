#include <iostream>
#include <cstdio>
#include <vector>
#include <algorithm>
#include <boost/utility/string_ref.hpp>
#include "../ast/ast_printer.h"
#include "../ast/decl.h"
#include "../parser/parser.hpp"
#include "../sema/typecheck.h"
#include "../cgen/codegen.h"

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
    const bool syntaxOnly = checkFlag("-fsyntax-only", args);
    const bool compileOnly = checkFlag("-c", args);
    const bool printAST = checkFlag("-print-ast", args);
    const bool outputToStdout = checkFlag("-o=stdout", args);

    for (boost::string_ref filePath : args) {
        yyin = fopen(filePath.data(), "rb");

        if (!yyin) {
            std::cout << "error: no such file: '"  << filePath << "'" << std::endl;
            return 1;
        }

        int result = yyparse();
        if (result != 0) return result;

        typecheck(globalAST);

        if (printAST) {
            std::cout << globalAST;
        } else if (!syntaxOnly) {
            cgen::compile(globalAST, outputToStdout ? "stdout" : (std::string(filePath) + ".c"));
        }
        globalAST.clear();
    }

    if (!syntaxOnly && !printAST && !compileOnly) {
        // Compile and link C output.
        std::string command = "cc -Werror";
        for (boost::string_ref filePath : args) {
            command.append(" ");
            command.append(filePath.data(), filePath.size());
            command.append(".c");
        }
        if (!outputToStdout) std::cout << command << '\n';
        const int ccExitStatus = system(command.c_str());

        for (boost::string_ref filePath : args) {
            std::remove((std::string(filePath) + ".c").c_str());
        }

        if (ccExitStatus != 0) exit(ccExitStatus);
    }
}
