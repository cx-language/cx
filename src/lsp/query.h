#pragma once

// One-shot LSP query subprocess: one query as JSON on stdin, one result as
// JSON on stdout, then exit. The server spawns a fresh process per operation
// so the frontend always runs exactly once in a pristine address space
// (see analyzer.h for the rationale).

#include <string>

namespace cx::lsp {

std::string readAllStdin();

/// Handles one `--query` invocation: read query JSON from stdin, write
/// `{"ok":true,"result":{...}}` (or `{"ok":false,"error":"..."}`) to stdout.
/// Returns the process exit code (always 0: even compiler crashes surface as
/// JSON diagnostics, so the server can stay alive).
int runQueryProcess();

} // namespace cx::lsp
