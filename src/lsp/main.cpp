// cx-lsp: the cx language server.
//
// Two modes in one binary:
//
//   cx-lsp            Run the LSP server (JSON-RPC over stdio). Answers each
//                     operation from an in-process compilation cache,
//                     recompiling from reset compiler globals only when
//                     inputs or on-disk dependencies changed.
//
//   cx-lsp --query    Read one query as JSON from stdin, run the compiler
//                     frontend exactly once, print the result JSON to stdout,
//                     and exit. All compiler memory is reclaimed by the OS on
//                     exit, just like a normal `cx` invocation. Kept for
//                     tooling authors and tests.
//
//   cx-lsp --leak-check <file> <count>
//                     Analyze <file> <count> times through one session and
//                     exit. Test-only hook for the LeakSanitizer gate over
//                     repeated arena resets (see test/lsp/check_lsan.py).
//
// `cx lsp` forwards to this binary.
#include "query.h"
#include "server.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif
#pragma warning(push, 0)
#include <llvm/Support/raw_ostream.h>
#pragma warning(pop)

int main(int argc, const char** argv) {
#ifdef _WIN32
    // LSP framing counts bytes, but stdio defaults to text mode on Windows,
    // which expands \n to \r\n on stdout and breaks Content-Length framing.
    _setmode(_fileno(stdin), _O_BINARY);
    _setmode(_fileno(stdout), _O_BINARY);
#endif

    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--query") == 0) {
            return cx::lsp::runQueryProcess();
        }
        if (std::strcmp(argv[i], "--leak-check") == 0) {
            const char* filePath = i + 1 < argc ? argv[i + 1] : nullptr;
            int count = i + 2 < argc ? std::atoi(argv[i + 2]) : 0;
            return cx::lsp::runLeakCheck(filePath, count);
        }
        if (std::strcmp(argv[i], "--help") == 0 || std::strcmp(argv[i], "-h") == 0) {
            llvm::outs() << "Usage:\n"
                            "  cx-lsp            Run the cx language server (LSP over stdio)\n"
                            "  cx-lsp --query    Run one compiler query from stdin as JSON and exit\n"
                            "  cx-lsp --leak-check <file> <count>\n"
                            "                    Analyze <file> <count> times in one session and exit\n";
            return 0;
        }
    }

    (void)argv;
    return cx::lsp::runServer();
}
