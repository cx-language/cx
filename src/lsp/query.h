#pragma once

// One-shot LSP query: one query as JSON on stdin, one result as JSON on
// stdout, then exit. The frontend runs exactly once in a pristine address
// space. Kept for tooling authors and tests; the server answers from its own
// in-process cache instead (see analyzer.h).

#include <string>

namespace cx::lsp {

std::string readAllStdin();

/// Handles one `--query` invocation: read query JSON from stdin, write
/// `{"ok":true,"result":{...}}` (or `{"ok":false,"error":"..."}`) to stdout.
/// Returns the process exit code (always 0: even compiler crashes surface as
/// JSON diagnostics, so the caller can stay alive).
int runQueryProcess();

} // namespace cx::lsp
