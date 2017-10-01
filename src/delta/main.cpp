#include <string>
#include <vector>
#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/raw_ostream.h>
#include "../driver/driver.h"
#include "../driver/repl.h"
#include "../support/utility.h"

using namespace delta;

static std::vector<std::string> removeFileArgs(std::vector<llvm::StringRef>& args) {
    std::vector<std::string> files;

    for (auto arg = args.begin(); arg != args.end();) {
        if (!arg->startswith("-")) {
            files.push_back(*arg);
            arg = args.erase(arg);
        } else {
            ++arg;
        }
    }

    return files;
}

static void printHelp() {
    llvm::outs() <<
        "OVERVIEW: Delta compiler\n"
        "\n"
        "USAGE: delta [options] <inputs>\n"
        "\n"
        "OPTIONS:\n"
        "  -c                    - Compile only, generating an .o file; don't link\n"
        "  -emit-assembly        - Emit assembly code\n"
        "  -fPIC                 - Emit position-independent code\n"
        "  -help                 - Display this help\n"
        "  -I<directory>         - Add a search path for module and C header import\n"
        "  -parse                - Perform parsing\n"
        "  -print-ast            - Print the abstract syntax tree to stdout\n"
        "  -print-ir             - Print the generated LLVM IR to stdout\n"
        "  -typecheck            - Perform parsing and type checking\n";
}

int main(int argc, const char** argv) {
    --argc;
    ++argv;

    if (argc == 0) {
        return replMain();
    }

    llvm::StringRef command = argv[0];

    try {
        if (command == "build") {
            std::vector<llvm::StringRef> args(argv + 1, argv + argc);
            return buildPackage(".", args, /* run */ false);
        } else if (command == "run") {
            std::vector<llvm::StringRef> args(argv + 1, argv + argc);
            auto files = removeFileArgs(args);

            if (files.empty()) {
                return buildPackage(".", args, /* run */ true);
            } else {
                return buildExecutable(files, args, /* run */ true);
            }
        } else {
            std::vector<llvm::StringRef> args(argv, argv + argc);

            if (checkFlag("-help", args) || checkFlag("--help", args) || checkFlag("-h", args)) {
                printHelp();
                return 0;
            }

            auto files = removeFileArgs(args);
            return buildExecutable(files, args, /* run */ false);
        }
    } catch (const CompileError& error) {
        error.print();
        return 1;
    }
}
