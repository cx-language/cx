# Compiler internals

## cx-lsp

### Design: cached in-process compilation

The server runs the compiler frontend in-process and keeps the latest
compilation in a session cache (`LspSession` in `src/lsp/analyzer.h`).
Repeated operations on unchanged inputs - each hover, definition,
completion, references or semantic-tokens request - answer from the cached
module without recompiling, so navigation is instant. A cache miss resets
all compiler globals and recompiles from scratch, exactly like a fresh
`cx-lsp --query` run would, so cached and uncached answers never differ.

- A query compiles the target file plus the rest of its package from their
  unsaved buffers when open, and imports (including `std`) from disk:
  - files inside an importable package (such as `std` itself) are analyzed
    as that package: the import resolves to the open files rather than
    loading a second copy that would drown the file in duplicate
    "ambiguous reference" errors;
  - files under a `build.cx` file are analyzed as its target root
    (found by searching upwards, honoring `multitarget` layouts), so
    cross-file references work from nested subdirectories too - the build
    file is only used to locate the root, dependencies are never fetched;
  - anything else is standalone, like `cx file.cx`.
- The cache key covers request inputs (file, content, the open-documents
  overlay for siblings, workspace folders, search paths, defines) plus
  on-disk state: the module layout is re-discovered per query (added or
  removed siblings and build files invalidate), and every source read from
  disk - main-module files, imported packages, build files, the directly
  imported C header - is validated by modification time and size. Cursor
  position and method are excluded: they only select the answer from an
  already-compiled module. Inputs are re-verified after compiling, so a
  file changed mid-compile stays uncacheable instead of sticking. A
  compilation that throws partway (a malformed build file) also never
  caches: every query recompiles until it is fixed. Gaps that remain: a
  same-size rewrite inside mtime granularity is missed; newly added files
  inside an imported package and transitive C-header includes need an edit
  in the open file to refresh; pkg-config results are pinned per process
  (failures retry, successes stick), so .pc edits need a restart.
- Memory stays bounded: evicting the cached entry deletes its modules
  (freeing the file buffers), destroys every arena node (freeing their
  vectors and strings) and resets the AST arena, so the session only ever
  holds one compilation. Steady state is ~75 MB for a small file and
  ~145 MB for the SDL example on macOS.
- Compiler errors and C++ exceptions are still contained per query and
  surface as diagnostics, but a frontend hard crash (segfault, failed
  assert) now takes down the server instead of a single query subprocess,
  and there is no hang timeout anymore. If that ever bites, the principled
  fix is a warm worker process the server can restart, not per-operation
  subprocesses again.
- Diagnostics are owned by the file being checked: an error in a sibling file
  surfaces when that sibling (or an importer in the same module) is checked.
  Fixing file B clears an error that file A's check attributed to B the next
  time either file is checked.
- The server handles one query at a time: a slow compilation briefly delays
  hover/completion/diagnostics for all files.
- Header search paths mirror `cx build` (`CPATH`/`C_INCLUDE_PATH`/`INCLUDE`,
  `/usr/include`, ...), except the server doesn't probe the external C
  compiler for its builtin header paths.
- Process-constant lookups (platform SDK options, pkg-config output,
  compiler search paths) are cached instead of re-queried per compilation.

Measured costs on Apple Silicon in release builds: a cache hit answers in
well under a millisecond of server time (~2 ms including JSON framing);
a miss pays one full compilation (~20 ms for a small file, ~170 ms for
the SDL example, most of it lazily checking the used standard-library
and SDL declarations).

### Tests

`test/lsp/test_lsp.py` drives both layers end to end (one-shot `--query`
plus live server sessions over stdio, including via `cx lsp`, covering
cache hits and every invalidation path). It runs as the `lsp` CTest test:

```sh
cmake --build build --target check   # includes the lsp test
# or just:
ctest --test-dir build -R lsp --output-on-failure
```

### Query protocol

`cx-lsp --query` reads one JSON object from stdin and writes
`{"ok": true, "result": {...}}` to stdout (never exits non-zero for compiler
errors; malformed input yields `{"ok": false, "error": "..."}`).

Request:

```jsonc
{
    "method": "check",        // or "hover" | "definition" | "completion"
                              // | "documentSymbol" | "references" | "semanticTokens"
    "file": "/abs/path/main.cx",
    "content": "<full unsaved text>",
    "openDocs": {"/abs/path/other.cx": "<text>"},  // sibling overlay
    "workspaceFolders": [],
    "importSearchPaths": [],
    "defines": [],
    "position": {"line": 5, "character": 18}  // 0-based; needed for all
                                              // methods except check/documentSymbol
}
```

`check` returns `{"diagnostics": [{"file", "range", "severity" (1=error,
2=warning), "message", "relatedInformation"}]}`. Every other method returns
the same `diagnostics` array plus its own payload (`hover`, `found`/`file`/
`range`, `items`, `symbols`, `references`, `tokens`). All ranges are 0-based.
`semanticTokens` returns `tokens` sorted by position as
`[{"line", "start", "length", "type", "modifiers"}]` (absolute positions; the
server delta-encodes them for LSP).

Example:

```sh
echo '{"method":"check","file":"/tmp/main.cx","content":"void main() {\n nosuchvar;\n}\n","openDocs":{},"workspaceFolders":[],"importSearchPaths":[],"defines":[]}' \
  | cx-lsp --query
```
