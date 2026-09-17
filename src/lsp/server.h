#pragma once

// Thin long-lived LSP server: stores open documents and speaks LSP JSON-RPC
// over stdio, but never runs the compiler frontend itself. Every operation
// spawns a fresh `cx-lsp --query` subprocess (see query.h) which compiles
// once and exits, so the OS always reclaims compiler memory.

#include <string>
#include <vector>

namespace cx::lsp {

struct ServerOptions {
    /// Path of the executable to respawn for `--query` subprocesses
    /// (normally the cx-lsp binary itself).
    std::string queryExecutable;
};

/// Runs the LSP event loop on stdin/stdout. Returns the process exit code.
int runServer(const ServerOptions& options);

} // namespace cx::lsp
