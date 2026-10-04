#pragma once

// Language-server queries.
//
// The server (see server.h) runs the compiler frontend in-process and keeps
// the latest compilation in an LspSession: repeated operations on unchanged
// inputs (hover, definition, completion, ...) answer from the cached module
// without recompiling. A cache miss resets all compiler globals and
// recompiles from scratch, exactly like a fresh `cx-lsp --query` process
// would, so cached and uncached answers never differ. `cx-lsp --query` (see
// query.h) keeps the one-shot behavior for tooling and tests.
//
// Memory stays bounded: evicting the cached entry deletes its modules (which
// frees the file buffers) and resets the AST arena, so the session only ever
// holds one compilation plus the arena's retained first slab.

#include <cstdint>
#include <optional>
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
    std::string kind; // "keyword", "function", "method", "variable", "type", "field", "enumMember" or "parameter".
    std::string detail;
    std::string documentation{}; // The declaration's /// doc comment, or "".
    bool hasParams = false; // True when a "function"/"method" takes parameters.
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
    bool readonly = false; // Immutable binding (maps to the "readonly" modifier).
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

/// A single language-server query, deserialized from query JSON (stdin for
/// `--query`, built directly by the server for its in-process cache).
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

/// A file the compilation read from disk, for cache validation.
struct FileStat {
    std::string path;
    int64_t mtimeNanos = 0;
    uint64_t size = 0;
    bool operator==(const FileStat&) const = default;
};

/// Which files form the open file's module. The session re-validates it per
/// query with stat-only probes (no parsing) to detect added or removed
/// siblings and build files.
struct ModuleLayout {
    std::optional<std::string> moduleDir;
    std::string buildDir;
    bool registerAsStd = false;
    std::vector<std::string> siblingPaths; // Sorted .cx siblings, excluding filePath.
    std::vector<std::string> checkedBuildFiles; // Every build.cx met walking up.
};

/// The result of one frontend run: the main module plus the structured
/// diagnostics collected via diagnosticCollector. The session owns mainModule
/// (deleted on evict); the one-shot query process leaks it and exits.
struct FrontendResult {
    Module* mainModule = nullptr;
    std::vector<CollectedDiagnostic> diagnostics;
    std::string content; // The analyzed text of filePath (for range mapping).
    std::string filePath;
    ModuleLayout layout; // For the session's cache entry (avoids re-discovery).
    std::vector<std::string> depBuildFiles; // build.cx files of resolved dependencies.
};

/// Runs the frontend (parse + typecheck) from clean compiler globals. The
/// session calls this only right after resetCompilerGlobals; the one-shot
/// query process relies on its fresh address space instead.
FrontendResult runFrontendOnce(const LspQuery& query);

/// Resets every compiler global a frontend run mutates (AST arena, type and
/// string interning, imported modules, diagnostics, synthesized names), so
/// the next runFrontendOnce behaves exactly like a fresh process. The caller
/// must have dropped all compiler-owned pointers first.
void resetCompilerGlobals();

/// Runs the frontend once and answers the query, returning the JSON "result"
/// object for the query subprocess to print (or "diagnostics" array for
/// "check"). Throws JsonParseError on malformed queries.
JsonValue handleQuery(const JsonValue& queryJson);

/// Parses stdin-style query JSON into an LspQuery. Throws JsonParseError.
LspQuery parseLspQuery(const JsonValue& queryJson);

/// One cached compilation plus everything that must still match to reuse it.
/// Position and method are excluded: they only select the answer from an
/// already-compiled module.
struct LspCachedFrontend {
    std::string filePath;
    std::vector<std::pair<std::string, std::string>> openDocs; // Relevant subset (siblings), sorted.
    std::vector<std::string> workspaceFolders;
    std::vector<std::string> importSearchPaths;
    std::vector<std::string> defines;
    ModuleLayout layout;
    std::vector<FileStat> depStats; // Every on-disk input (build files, main-module and import sources).
    FrontendResult frontend; // frontend.content is the post-placeholder text actually compiled.
};

/// The server's compilation cache: at most one live frontend, recompiled
/// only when request inputs or on-disk dependencies changed.
struct LspSession {
    std::optional<LspCachedFrontend> cached;
    /// Answers one query from the cache, recompiling on mismatch. Never
    /// throws: any failure comes back as nullopt and the server falls back
    /// (empty result or "analysis failed" diagnostic).
    std::optional<JsonValue> handle(LspQuery query);
};

/// AST query helpers operating on a single frontend run's module.
SymbolInfo findAt(Module* mainModule, const std::string& filePath, LspPosition pos);
std::string hoverAt(Module* mainModule, const std::string& filePath, LspPosition pos);
bool gotoDefinitionAt(Module* mainModule, const std::string& filePath, LspPosition pos, std::string& outFilePath, LspRange& outRange);
std::vector<CompletionItem> completeAt(Module* mainModule, const std::string& filePath, LspPosition pos, const std::string& content);
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
