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

/// Handles `--leak-check <file> <count>`: runs <count> "check" analyses of
/// <file> through one LspSession (each with a unique trailing comment, so
/// every analysis misses the cache and resets the AST arena), drops the
/// session cache, then exits. Under LeakSanitizer the exit report must not
/// grow with <count>: growth means AST nodes own malloc'd memory that
/// resetAstArena never frees. Under AddressSanitizer the repeated analyses
/// and resets run as a memory error check instead. A failed analysis prints
/// a marker to stdout (otherwise empty); usage/IO errors go to stderr.
/// Returns 1 on any of those; analysis diagnostics are ignored.
int runLeakCheck(const char* filePath, int count);

} // namespace cx::lsp
