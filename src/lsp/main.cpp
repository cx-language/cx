// cx-lsp: the C* language server.
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
#include <cstring>
#pragma warning(push, 0)
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_ostream.h>
#pragma warning(pop)

int main(int argc, const char** argv) {
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--query") == 0) {
            return cx::lsp::runQueryProcess(argv[0]);
        }
        if (std::strcmp(argv[i], "--help") == 0 || std::strcmp(argv[i], "-h") == 0) {
            llvm::outs() << "Usage:\n"
                            "  cx-lsp            Run the C* language server (LSP over stdio)\n"
                            "  cx-lsp --query    Run one compiler query from stdin as JSON and exit\n";
            return 0;
        }
    }

    std::string queryExecutable = llvm::sys::fs::getMainExecutable(argv[0], reinterpret_cast<void*>(&cx::lsp::runServer));
    return cx::lsp::runServer(cx::lsp::ServerOptions{queryExecutable});
}
