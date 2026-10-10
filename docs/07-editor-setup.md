# Editor setup

`cx-lsp` brings cx to any editor that speaks the [Language Server
Protocol](https://microsoft.github.io/language-server-protocol/) (VS Code,
Neovim, Emacs, Helix, Zed, ...). Start it with `cx-lsp` directly, or with `cx
lsp` (which forwards stdio to `cx-lsp` found next to `cx` or on `PATH`).

## VS Code

Install the bundled extension in `editors/vscode` (search for `cx` in the
extensions view, or build the `.vsix` from that directory). It contributes
syntax highlighting, snippets, and a language client that launches `cx-lsp`
from `PATH` (override with the `cx.languageServer.path` setting).

For manual configuration with another LSP client extension, the essentials are:

- command: `cx-lsp`
- filetypes: `cx` (files ending in `.cx`)
- root markers: `build.cx`, `.git`

## Neovim (nvim-lspconfig)

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

## Helix

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

## Zed / Emacs (eglot) / others

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

## Features

| Capability | LSP method |
| --- | --- |
| Diagnostics (parse + type errors and warnings, as you type) | `textDocument/publishDiagnostics` |
| Hover signatures plus `///` doc comments | `textDocument/hover` |
| Go to definition | `textDocument/definition` |
| Completions (keywords, locals, top-level/stdlib declarations, members after `.`; callables insert `(` or `()`; entries show `///` doc comments) | `textDocument/completion` |
| File outline (functions, types, fields, methods, enum cases) | `textDocument/documentSymbol` |
| Find references (same package + imports, including type names) | `textDocument/references` |
| Syntax highlighting (whole file + ranges) | `textDocument/semanticTokens/full`, `textDocument/semanticTokens/range` |

Hover and completion entries include the declaration's `///` doc comment
(consecutive `///` lines directly above it) as plain text, with single
newlines folded into spaces and blank `///` lines separating paragraphs.

Token types: `comment`, `string`, `number`, `keyword`, `macro` (`#if`/`#else`/`#endif`),
`type`, `struct`, `enum`, `interface`, `typeParameter`, `parameter`, `variable`,
`property` (fields), `enumMember`, `function`, `method`. Definitions carry the
`definition` modifier, and immutable variables and parameters carry `readonly`.
Keywords, strings, numbers, comments and macros highlight
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

- Unsaved changes in *imported* packages are not visible yet - save the
  imported file first.
