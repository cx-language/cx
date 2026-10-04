#pragma once

// Long-lived LSP server: stores open documents, speaks LSP JSON-RPC over
// stdio, and answers every operation from an in-process compilation cache
// (see analyzer.h's LspSession). Repeated operations on unchanged inputs
// reuse the cached frontend without recompiling; a cache miss resets all
// compiler globals and recompiles from scratch, exactly like a fresh
// `cx-lsp --query` run would.

namespace cx::lsp {

/// Runs the LSP event loop on stdin/stdout. Returns the process exit code.
int runServer();

} // namespace cx::lsp
