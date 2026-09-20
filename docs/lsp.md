# cx Language Server (`cx-lsp`)

`cx-lsp` brings cx to any editor that speaks the [Language Server
Protocol](https://microsoft.github.io/language-server-protocol/) (VS Code,
Neovim, Emacs, Helix, Zed, ...). Start it with `cx-lsp` directly, or with `cx
lsp` (which forwards stdio to `cx-lsp` found next to `cx` or on `PATH`).

## Features

| Capability | LSP method |
| --- | --- |
| Diagnostics (parse + type errors, as you type) | `textDocument/publishDiagnostics` |
| Hover signatures | `textDocument/hover` |
| Go to definition | `textDocument/definition` |
| Completions (keywords, locals, top-level and stdlib declarations) | `textDocument/completion` |
| File outline (functions, types, fields, methods, enum cases) | `textDocument/documentSymbol` |
| Find references (same package + imports, including type names) | `textDocument/references` |
| Syntax highlighting (whole file + ranges) | `textDocument/semanticTokens/full`, `textDocument/semanticTokens/range` |

Token types: `comment`, `string`, `number`, `keyword`, `macro` (`#if`/`#else`/`#endif`),
`type`, `struct`, `enum`, `interface`, `typeParameter`, `parameter`, `variable`,
`property` (fields), `enumMember`, `function`, `method`. Definitions carry the
`definition` modifier. Keywords, strings, numbers, comments and macros highlight
even when the file doesn't compile.

Positions assume UTF-8/ASCII source (one byte per character).

## Multi-file projects

A multi-file program needs a `build.cx` file for cross-file
references to work; without one, each file is checked standalone.
Conversely, everything under the same `build.cx` target root is
analyzed as one module, so independent programs sharing a target
root report redefinition errors. To keep several programs under
one build file, set `multitarget = true` in `build.cx` and give each
program its own subdirectory (directly under the package root, or
under `src/`).

## Limitations

- Unsaved changes in *imported* packages are not visible yet — save the
  imported file first.

## Editor setup

### VS Code

Use any LSP client extension that supports custom servers, configured with:

```jsonc
{
    "languageServerExample.serverCommand": "cx-lsp",
    "languageServerExample.filetypes": ["cx"]
}
```

For manual configuration, the essentials are:

- command: `cx-lsp`
- filetypes: `cx` (files ending in `.cx`)
- root markers: `build.cx`, `.git`

### Neovim (nvim-lspconfig)

```lua
local lspconfig = require('lspconfig')
local configs = require('lspconfig.configs')

configs.cx_lsp = {
  default_config = {
    cmd = { 'cx-lsp' },
    filetypes = { 'cx' },
    root_dir = lspconfig.util.root_pattern('build.cx', '.git'),
  },
}

lspconfig.cx_lsp.setup {}
```

### Helix

```toml
# ~/.config/helix/languages.toml
[[language]]
name = "cx"
scope = "source.cx"
file-types = ["cx"]
language-servers = ["cx-lsp"]

[language-server.cx-lsp]
command = "cx-lsp"
```

### Zed / Emacs (eglot) / others

Point your client at `cx-lsp` for the `cx` language; no extra flags needed.
The server speaks full-sync `textDocumentSync`, so any conforming client
works.

## Initialization options

`initializationOptions` supports two optional keys:

```jsonc
{
    "importSearchPaths": ["/path/to/my/libs"],
    "defines": ["MyFeature"]
}
```

Extra search paths are searched after the file's directory and the workspace
folders, but before the bundled `std/`. `defines` sets `#if` flags. If a
C-header import only resolves under `cx build`, add its directory to
`importSearchPaths`.

## The `--query` protocol (for tooling authors)

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

## Contributor notes

### Design: one compilation per process

This compiler is deliberately not a long-running process: it never frees
memory and is built to start fast, compile fast, and exit. The language
server honors that instead of working around it:

- The long-lived server process **never runs the compiler frontend**. It only
  stores open-document text and speaks LSP JSON-RPC over stdio.
- Every operation — each keystroke's diagnostics, each hover, each completion
  — spawns a fresh **`cx-lsp --query` subprocess**, pipes it one JSON query
  (file path, unsaved content, open documents, cursor position), reads back
  one JSON result, and lets the process exit. The OS reclaims all compiler
  memory, exactly like a normal `cx` invocation.
- A query compiles the target file plus the rest of its package from their
  unsaved buffers when open, and imports (including `std`) from disk:
  - files inside an importable package (such as `std` itself) are analyzed
    as that package: the import resolves to the open files rather than
    loading a second copy that would drown the file in duplicate
    "ambiguous reference" errors;
  - files under a `build.cx` file are analyzed as its target root
    (found by searching upwards, honoring `multitarget` layouts), so
    cross-file references work from nested subdirectories too — the build
    file is only used to locate the root, dependencies are never fetched;
  - anything else is standalone, like `cx file.cx`.
- Diagnostics are owned by the file being checked: an error in a sibling file
  surfaces when that sibling (or an importer in the same module) is checked.
  Fixing file B clears an error that file A's check attributed to B the next
  time either file is checked.
- The server handles one query at a time: a slow compilation briefly delays
  hover/completion/diagnostics for all files. Queries carry a 30 s timeout, so
  a hung compiler can't wedge the session forever.
- Header search paths mirror `cx build` (`CPATH`/`C_INCLUDE_PATH`/`INCLUDE`,
  `/usr/include`, ...), except the server doesn't probe the external C
  compiler for its builtin header paths.
- There are intentionally no "reset compiler globals" helpers anywhere in the
  LSP code. If per-keystroke compiles ever feel slow, that is a signal to
  make the compiler faster (good dogfooding), not to add caches to the
  server.

Measured cost of one query (parse + typecheck including the standard
library): ~40 ms on Apple Silicon in release builds.

### Tests

`test/lsp/test_lsp.py` drives both layers end to end (query subprocess +
live server session over stdio, including via `cx lsp`). It runs as the
`lsp` CTest test:

```sh
cmake --build build --target check   # includes the lsp test
# or just:
ctest --test-dir build -R lsp --output-on-failure
```
