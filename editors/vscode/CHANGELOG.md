# Change Log

All notable changes to the "cx-lang" extension will be documented in this file.

Check [Keep a Changelog](http://keepachangelog.com/) for recommendations on how to structure this file.

## 0.2.1
### Changed
- Fixed README screenshot and install instructions

## 0.2.0
### Added
- Language server integration: diagnostics, hover, go to definition, completions, document symbols, references, and semantic highlighting via `cx-lsp`, with `cx.languageServer.*` settings and a restart command

### Changed
- Renamed the extension to `cx-lang` (the `cx-language` name is taken on the Marketplace)
- Updated syntax highlighting to current cx syntax (current keywords, `{...}` string interpolation, builtin types, attributes, `#if`/`#else`/`#endif`, wrapping/saturating operators)
- Updated snippets to current syntax (no parentheses in `if`/`while`/`for`/`switch`, no semicolons); added `union` and `@test` snippets

## 0.1.0
### Changed
- Updated syntax highlighting to current cx syntax

### Added
- Bunch of new snippets
- Added a screenshot showcasing cx and the extension

## 0.0.1
- Initial release
