// cx-lsp: the cx language server.
//
// Two modes in one binary so queries always run in a pristine address space:
//
//   cx-lsp            Run the LSP server (JSON-RPC over stdio). This process
//                     never runs the compiler frontend; it only stores open
//                     documents and answers each operation by spawning a fresh
//                     `cx-lsp --query` subprocess (see below).
//
//   cx-lsp --query    Read one query as JSON from stdin, run the compiler
//                     frontend exactly once, print the result JSON to stdout,
//                     and exit. All compiler memory is reclaimed by the OS on
//                     exit, just like a normal `cx` invocation.
//
// `cx lsp` forwards to this binary.
#include "query.h"
#include "server.h"
#include <cstdio>
#include <cstring>
#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif
#pragma warning(push, 0)
#include <llvm/Support/FileSystem.h>
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
        if (std::strcmp(argv[i], "--help") == 0 || std::strcmp(argv[i], "-h") == 0) {
            llvm::outs() << "Usage:\n"
                            "  cx-lsp            Run the cx language server (LSP over stdio)\n"
                            "  cx-lsp --query    Run one compiler query from stdin as JSON and exit\n";
            return 0;
        }
    }

    std::string queryExecutable = llvm::sys::fs::getMainExecutable(argv[0], reinterpret_cast<void*>(&cx::lsp::runServer));
    return cx::lsp::runServer(cx::lsp::ServerOptions{queryExecutable});
}
