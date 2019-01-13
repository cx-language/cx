#include <csignal>
#include <cstdlib>
#include <string>
#include <vector>
#ifndef _WIN32
#include <cstring>
#include <execinfo.h>
#include <unistd.h>
#endif
#pragma warning(push, 0)
#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_ostream.h>
#pragma warning(pop)
#include "../driver/driver.h"
#include "../driver/repl.h"
#include "../support/utility.h"

using namespace delta;

static void printHelp() {
    llvm::outs() << "OVERVIEW: Delta compiler\n"
                    "\n"
                    "USAGE: delta [options] <inputs>\n"
                    "\n"
                    "OPTIONS:\n"
                    "  -c                    - Compile only, generating an .o file; don't link\n"
                    "  -emit-assembly        - Emit assembly code\n"
                    "  -emit-llvm-bitcode    - Emit LLVM bitcode\n"
                    "  -F<directory>         - Add a search path for C framework header import\n"
                    "  -fPIC                 - Emit position-independent code\n"
                    "  -help                 - Display this help\n"
                    "  -I<directory>         - Add a search path for module and C header import\n"
                    "  -o<filename>          - Use the given filename for the output executable\n"
                    "  -parse                - Perform parsing\n"
                    "  -print-ast            - Print the abstract syntax tree to stdout\n"
                    "  -print-ir             - Print the generated LLVM IR to stdout\n"
                    "  -typecheck            - Perform parsing and type checking\n"
                    "  -Werror               - Treat warnings as errors\n";
}

static void segfaultHandler(int signal) {
#ifndef _WIN32
    void* stacktrace[128];
    int size = backtrace(stacktrace, 128);
    llvm::errs() << strsignal(signal) << '\n';
    backtrace_symbols_fd(stacktrace, size, STDERR_FILENO);
#endif
    std::exit(signal);
}

int main(int argc, const char** argv) {
    std::signal(SIGSEGV, segfaultHandler);
    const char* argv0 = argv[0];

    --argc;
    ++argv;

    if (argc == 0) {
        return replMain();
    }

    llvm::StringRef command = argv[0];
    bool build = command == "build";
    bool run = command == "run";

    if (build || run) {
        --argc;
        ++argv;
    }

    std::vector<std::string> inputs;
    std::vector<llvm::StringRef> args;

    for (int i = 0; i < argc; ++i) {
        llvm::StringRef arg = argv[i];

        if (arg == "help" || arg == "-help" || arg == "--help" || arg == "-h") {
            printHelp();
            return 0;
        } else if (arg.startswith("-")) {
            args.push_back(arg);
        } else {
            inputs.push_back(arg);
        }
    }

    try {
        if (inputs.empty()) {
            llvm::SmallString<128> currentPath;
            if (auto error = llvm::sys::fs::current_path(currentPath)) {
                printErrorAndExit(error.message());
            }
            return buildPackage(currentPath, argv0, args, run);
        } else {
            return buildExecutable(inputs, nullptr, argv0, args, ".", "", run);
        }
    } catch (const CompileError& error) {
        error.print();
        return 1;
    }
}
