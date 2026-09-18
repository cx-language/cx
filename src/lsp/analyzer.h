#pragma once

// Single-shot language-server queries.
//
// Design note: this compiler is intentionally not a long-running process - it
// never frees memory and is built to start fast, compile fast, and exit. The
// LSP honors that: the long-lived server process (see server.h) never runs
// the compiler frontend itself. Every user-visible operation (diagnostics,
// hover, definition, completion, symbols, references) spawns a fresh one-shot
// query process (`cx-lsp --query`, see query.h) which calls runFrontendOnce()
// exactly once in a pristine address space and then exits. Nothing here is
// ever reused across compilations, so there are intentionally no "reset"
// helpers for compiler globals.
//
// Memory is deliberately leaked (modules, AST nodes, interned types): the OS
// reclaims it on process exit, just like a normal `cx` invocation.

#include <string>
#include <utility>
#include <vector>
#pragma warning(push, 0)
#include <llvm/ADT/StringMap.h>
#pragma warning(pop)
#include "../ast/location.h"
#include "../support/utility.h"
#include "json.h"

namespace cx {
struct Decl;
struct Expr;
struct Module;
} // namespace cx

namespace cx::lsp {

struct LspPosition {
    int line = 0; // 0-based.
    int character = 0; // 0-based column; positions are byte offsets (source is treated as ASCII).
};

struct LspRange {
    LspPosition start;
    LspPosition end;
};

struct LspDiagnostic {
    LspRange range;
    int severity = 1; // 1 = error, 2 = warning.
    std::string message;
    std::string filePath; // Absolute path.
    std::vector<Note> relatedNotes;
};

struct SymbolInfo {
    Decl* decl = nullptr; // Resolved declaration (or definition under cursor).
    Expr* expr = nullptr; // Reference expression under cursor, if any.
    Location refLocation; // Location of the reference/name under cursor.
    bool isDefinition = false;
};

struct CompletionItem {
    std::string label;
    std::string kind; // "keyword", "function", "variable", "type", "field" or "parameter".
    std::string detail;
};

struct DocumentSymbol {
    std::string name;
    std::string kind; // "function", "struct", "enum", "method", "field", "enumMember" or "variable".
    LspRange range;
    LspRange selectionRange;
};

/// Converts a file:// URI to a filesystem path. Returns the input unchanged
/// if it doesn't look like a file URI. Handles percent-encoding.
std::string uriToPath(const std::string& uri);
/// Converts a filesystem path to a file:// URI.
std::string pathToUri(const std::string& path);

/// A single language-server query, deserialized from the JSON the server
/// pipes to the query process's stdin.
struct LspQuery {
    std::string method; // "check", "hover", "definition", "completion", "documentSymbol" or "references".
    std::string filePath;
    std::string content; // Unsaved (or on-disk) text of filePath.
    llvm::StringMap<std::string> openDocs; // Unsaved-text overlay for sibling files.
    std::vector<std::string> workspaceFolders;
    std::vector<std::string> importSearchPaths;
    std::vector<std::string> defines;
    LspPosition position;
};

/// The result of one frontend run: the (intentionally leaked, process-lifetime)
/// main module plus the structured diagnostics collected via diagnosticCollector.
struct FrontendResult {
    Module* mainModule = nullptr; // Leaked on purpose; the query process exits right after.
    std::vector<CollectedDiagnostic> diagnostics;
    std::string content; // The analyzed text of filePath (for range mapping).
    std::string filePath;
};

/// Parses the frontend (parse + typecheck) exactly once. Must be called at
/// most once per process. `argv0` locates the standard library at runtime
/// (see getCxRootDir); pass the query process's argv[0].
FrontendResult runFrontendOnce(const LspQuery& query, const char* argv0);

/// Runs the frontend once and answers the query, returning the JSON "result"
/// object for the query subprocess to print (or "diagnostics" array for
/// "check"). Throws JsonParseError on malformed queries.
JsonValue handleQuery(const JsonValue& queryJson, const char* argv0);

/// Parses stdin-style query JSON into an LspQuery. Throws JsonParseError.
LspQuery parseLspQuery(const JsonValue& queryJson);

/// AST query helpers operating on a single frontend run's module.
SymbolInfo findAt(Module* mainModule, const std::string& filePath, LspPosition pos);
std::string hoverAt(Module* mainModule, const std::string& filePath, LspPosition pos);
bool gotoDefinitionAt(Module* mainModule, const std::string& filePath, LspPosition pos, std::string& outFilePath, LspRange& outRange);
std::vector<CompletionItem> completeAt(Module* mainModule, const std::string& filePath, LspPosition pos);
std::vector<DocumentSymbol> documentSymbolsIn(Module* mainModule, const std::string& filePath);
std::vector<std::pair<std::string, LspRange>> referencesTo(Module* mainModule, const std::string& filePath, LspPosition pos);
std::vector<LspDiagnostic> toLspDiagnostics(const std::vector<CollectedDiagnostic>& collected, const std::string& filePath, const std::string& content);

LspRange locationToRange(const Location& loc, const std::string& lineText);
LspRange nameRange(const Location& loc, size_t nameLength);

} // namespace cx::lsp
