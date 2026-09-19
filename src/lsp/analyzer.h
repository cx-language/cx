#pragma once

// Single-shot language-server queries.
//
// Design note: this compiler is intentionally not a long-running process - it
// never frees memory and is built to start fast, compile fast, and exit. The
// LSP honors that: the long-lived server process (see server.h) never runs
// the compiler frontend itself. Every user-visible operation (diagnostics,
// hover, definition, completion, symbols, references, semantic tokens) spawns
// a fresh one-shot
// query process (`cx-lsp --query`, see query.h) which calls runFrontendOnce()
// exactly once in a pristine address space and then exits. Nothing here is
// ever reused across compilations, so there are intentionally no "reset"
// helpers for compiler globals.
//
// Memory is deliberately leaked (modules, AST nodes, interned types): the OS
// reclaims it on process exit, just like a normal `cx` invocation.

#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#pragma warning(push, 0)
#include <llvm/ADT/StringMap.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/JSON.h>
#include <llvm/Support/raw_ostream.h>
#pragma warning(pop)
#include "../ast/location.h"
#include "../support/utility.h"

namespace cx {
struct Decl;
struct Expr;
struct Module;
} // namespace cx

namespace cx::lsp {

// JSON plumbing for queries and LSP messages, on top of LLVM's JSON parser.
// Values are built with JsonObject/JsonArray and read back through the
// null-safe findJson/getJson* helpers, which yield nullptr/fallbacks for
// missing keys and mistyped values instead of crashing on malformed input.
using JsonValue = llvm::json::Value;
using JsonObject = llvm::json::Object;
using JsonArray = llvm::json::Array;

struct JsonParseError : std::runtime_error {
    using std::runtime_error::runtime_error;
};

/// Parses one JSON document. Throws JsonParseError on invalid input.
inline JsonValue parseJson(llvm::StringRef text) {
    auto parsed = llvm::json::parse(text);
    if (parsed) return std::move(*parsed);
    throw JsonParseError("JSON parse error: " + llvm::toString(parsed.takeError()));
}

/// Serializes a JSON value in compact form.
inline std::string serializeJson(const JsonValue& value) {
    std::string out;
    llvm::raw_string_ostream stream(out);
    stream << value;
    stream.flush();
    return out;
}

/// Returns the named member, or nullptr if `object` isn't an object or has no such member.
inline const JsonValue* findJson(const JsonValue& object, llvm::StringRef key) {
    if (auto* obj = object.getAsObject()) return obj->get(key);
    return nullptr;
}

/// Returns the named member if it's an array, nullptr otherwise.
inline const JsonArray* findJsonArray(const JsonValue& object, llvm::StringRef key) {
    if (auto* obj = object.getAsObject()) return obj->getArray(key);
    return nullptr;
}

inline std::string getJsonString(const JsonValue& object, llvm::StringRef key, const std::string& fallback = "") {
    if (auto* obj = object.getAsObject()) {
        if (auto value = obj->getString(key)) return value->str();
    }
    return fallback;
}

inline long long getJsonInt(const JsonValue& object, llvm::StringRef key, long long fallback = 0) {
    if (auto* obj = object.getAsObject()) {
        if (auto value = obj->getInteger(key)) return *value;
    }
    return fallback;
}

inline bool getJsonBool(const JsonValue& object, llvm::StringRef key, bool fallback = false) {
    if (auto* obj = object.getAsObject()) {
        if (auto value = obj->getBoolean(key)) return *value;
    }
    return fallback;
}

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

struct SemanticToken {
    int line = 0; // 0-based.
    int start = 0; // 0-based byte column; single-line span.
    int length = 0;
    std::string type; // LSP token type name, e.g. "keyword" (see semanticTokenTypes()).
    bool definition = false; // Definition site (maps to the "definition" modifier).
};

/// The semantic-tokens legend: token type and modifier names indexed by the
/// LSP `data` encoding. The server advertises these verbatim.
const std::vector<std::string>& semanticTokenTypes();
const std::vector<std::string>& semanticTokenModifiers();

/// Converts a file:// URI to a filesystem path. Returns the input unchanged
/// if it doesn't look like a file URI. Handles percent-encoding.
std::string uriToPath(const std::string& uri);
/// Converts a filesystem path to a file:// URI.
std::string pathToUri(const std::string& path);

/// A single language-server query, deserialized from the JSON the server
/// pipes to the query process's stdin.
struct LspQuery {
    std::string method; // "check", "hover", "definition", "completion", "documentSymbol", "references" or "semanticTokens".
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
/// most once per process.
FrontendResult runFrontendOnce(const LspQuery& query);

/// Runs the frontend once and answers the query, returning the JSON "result"
/// object for the query subprocess to print (or "diagnostics" array for
/// "check"). Throws JsonParseError on malformed queries.
JsonValue handleQuery(const JsonValue& queryJson);

/// Parses stdin-style query JSON into an LspQuery. Throws JsonParseError.
LspQuery parseLspQuery(const JsonValue& queryJson);

/// AST query helpers operating on a single frontend run's module.
SymbolInfo findAt(Module* mainModule, const std::string& filePath, LspPosition pos);
std::string hoverAt(Module* mainModule, const std::string& filePath, LspPosition pos);
bool gotoDefinitionAt(Module* mainModule, const std::string& filePath, LspPosition pos, std::string& outFilePath, LspRange& outRange);
std::vector<CompletionItem> completeAt(Module* mainModule, const std::string& filePath, LspPosition pos);
std::vector<DocumentSymbol> documentSymbolsIn(Module* mainModule, const std::string& filePath);
std::vector<std::pair<std::string, LspRange>> referencesTo(Module* mainModule, const std::string& filePath, LspPosition pos);
/// Highlight tokens for one file, sorted by (line, start). Works on raw text
/// alone when the frontend failed (mainModule null), so broken code still
/// highlights keywords, strings, numbers and comments.
std::vector<SemanticToken> semanticTokensIn(Module* mainModule, const std::string& filePath, const std::string& content);
std::vector<LspDiagnostic> toLspDiagnostics(const std::vector<CollectedDiagnostic>& collected, const std::string& filePath, const std::string& content);

LspRange locationToRange(const Location& loc, const std::string& lineText);
LspRange nameRange(const Location& loc, size_t nameLength);

} // namespace cx::lsp
