#include "server.h"
#include "analyzer.h"
#include <algorithm>
#include <cstdio>
#include <new>
#include <optional>
#include <string>
#include <vector>
#pragma warning(push, 0)
#include <llvm/ADT/ScopeExit.h>
#include <llvm/ADT/StringMap.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Program.h>
#include <llvm/Support/raw_ostream.h>
#pragma warning(pop)

namespace cx::lsp {

namespace {

void logMessage(const std::string& message) {
    llvm::errs() << "[cx-lsp] " << message << '\n';
}

struct OpenDocument {
    std::string uri;
    std::string path;
    std::string text;
    long long version = 0;
};

/// A diagnostic as published to the editor. Unlike analyzer.h's LspDiagnostic
/// (whose notes borrow compiler-owned strings), relatedInformation is carried
/// as plain JSON so the cache owns everything it stores.
struct ServerDiagnostic {
    LspRange range;
    int severity = 1;
    std::string message;
    std::string filePath;
    JsonValue relatedInformation = JsonArray{};
};

struct ServerState {
    ServerOptions options;
    llvm::StringMap<OpenDocument> openDocs; // Keyed by path.
    // Last published diagnostics per file, so checking one file never wipes
    // the errors shown for another.
    llvm::StringMap<std::vector<ServerDiagnostic>> diagCache; // Keyed by path.
    std::vector<std::string> workspaceFolders;
    std::vector<std::string> importSearchPaths;
    std::vector<std::string> defines;
    bool shutdownRequested = false;
};

/// LSP transport framing: every message is a JSON document prefixed with a
/// Content-Length header on stdin/stdout. Byte-exact C stdio is used for both
/// directions because the framing counts bytes, not formatted output.
enum class ReadResult { Message, CleanEof, Malformed };

/// Reads one LSP message. Malformed input is reported (and skipped) without
/// killing the session; only a clean stdin EOF ends the server loop.
ReadResult readLspMessage(std::string& out) {
    // Cap single messages well above anything an editor sends; without this a
    // bogus Content-Length would exhaust memory in resize() below.
    constexpr size_t maxMessageBytes = 64 * 1024 * 1024;
    // Kept wide until the cap check below: narrowing first would truncate a
    // bogus length on 32-bit and skip the oversize drain.
    unsigned long long contentLength = 0;
    bool hasLength = false;
    bool lengthValid = true;
    std::string line;
    // Read headers.
    while (true) {
        line.clear();
        int ch = 0;
        // Read one line terminated by \r\n (tolerate bare \n).
        while ((ch = std::getchar()) != EOF && ch != '\n') {
            line += static_cast<char>(ch);
        }
        if (ch == EOF && line.empty()) return ReadResult::CleanEof;
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) break; // End of headers.
        const std::string prefix = "Content-Length:";
        if (line.size() >= prefix.size() && line.compare(0, prefix.size(), prefix) == 0) {
            std::string value = line.substr(prefix.size());
            size_t start = value.find_first_not_of(" \t");
            if (start == std::string::npos) {
                lengthValid = false;
            } else {
                unsigned long long length = 0;
                if (llvm::StringRef(value.substr(start)).getAsInteger(10, length)) {
                    lengthValid = false;
                } else {
                    contentLength = length;
                    hasLength = true;
                }
            }
        }
    }
    if (!hasLength || !lengthValid) {
        logMessage("dropping message with missing or invalid Content-Length");
        return ReadResult::Malformed;
    }
    if (contentLength == 0 || contentLength > maxMessageBytes) {
        // Drain oversized bodies so the stream stays framed, then skip.
        char discard[4096];
        unsigned long long remaining = contentLength;
        while (remaining > 0) {
            size_t chunk = std::fread(discard, 1, static_cast<size_t>(std::min<unsigned long long>(remaining, sizeof(discard))), stdin);
            if (chunk == 0) return ReadResult::CleanEof;
            remaining -= chunk;
        }
        logMessage("dropping message with invalid size");
        return ReadResult::Malformed;
    }
    // Capped above, so this fits in size_t on every platform.
    size_t bodySize = static_cast<size_t>(contentLength);
    try {
        out.resize(bodySize);
    } catch (const std::bad_alloc&) {
        logMessage("dropping message: out of memory");
        return ReadResult::Malformed;
    }
    size_t read = 0;
    while (read < bodySize) {
        size_t chunk = std::fread(&out[read], 1, bodySize - read, stdin);
        if (chunk == 0) {
            if (std::feof(stdin)) return ReadResult::CleanEof;
            // Error, or no progress for any other reason: bail out instead of
            // spinning forever on `read += 0`.
            return ReadResult::Malformed;
        }
        read += chunk;
    }
    return ReadResult::Message;
}

void writeLspMessage(const std::string& body) {
    std::string header = "Content-Length: " + std::to_string(body.size()) + "\r\n\r\n";
    std::fwrite(header.data(), 1, header.size(), stdout);
    std::fwrite(body.data(), 1, body.size(), stdout);
    std::fflush(stdout);
}

JsonValue makeResponse(const JsonValue& id, JsonValue result) {
    JsonObject response;
    response["jsonrpc"] = "2.0";
    response["id"] = id;
    response["result"] = std::move(result);
    return JsonValue(std::move(response));
}

JsonValue makeErrorResponse(const JsonValue& id, int code, const std::string& message) {
    JsonObject error;
    error["code"] = code;
    error["message"] = message;
    JsonObject response;
    response["jsonrpc"] = "2.0";
    response["id"] = id;
    response["error"] = std::move(error);
    return JsonValue(std::move(response));
}

JsonValue makeNotification(const std::string& method, JsonValue params) {
    JsonObject notification;
    notification["jsonrpc"] = "2.0";
    notification["method"] = method;
    notification["params"] = std::move(params);
    return JsonValue(std::move(notification));
}

JsonValue serverPositionToJson(const LspPosition& pos) {
    JsonObject out;
    out["line"] = pos.line;
    out["character"] = pos.character;
    return JsonValue(std::move(out));
}

JsonValue serverRangeToJson(const LspRange& range) {
    JsonObject out;
    out["start"] = serverPositionToJson(range.start);
    out["end"] = serverPositionToJson(range.end);
    return JsonValue(std::move(out));
}

LspPosition positionFromJson(const JsonValue* value) {
    LspPosition pos;
    if (value) {
        pos.line = static_cast<int>(getJsonInt(*value, "line"));
        pos.character = static_cast<int>(getJsonInt(*value, "character"));
    }
    return pos;
}

/// Applies one incremental edit. Only used when a client sends ranged edits
/// despite the server advertising full sync; positions are byte offsets
/// (see the ASCII note in analyzer.h).
std::string applyIncrementalChange(const std::string& text, LspPosition start, LspPosition end, const std::string& replacement) {
    std::vector<std::string> lines;
    std::string current;
    for (char ch : text) {
        if (ch == '\n') {
            lines.push_back(current);
            current.clear();
        } else if (ch != '\r') {
            current += ch;
        }
    }
    lines.push_back(current);

    auto clamp = [&](LspPosition pos) {
        if (pos.line < 0) pos.line = 0;
        if (pos.line >= static_cast<int>(lines.size())) pos = {static_cast<int>(lines.size()) - 1, static_cast<int>(lines.back().size())};
        if (pos.character < 0) pos.character = 0;
        if (pos.character > static_cast<int>(lines[pos.line].size())) pos.character = static_cast<int>(lines[pos.line].size());
        return pos;
    };
    start = clamp(start);
    end = clamp(end);

    std::string out;
    for (int i = 0; i < start.line; ++i)
        out += lines[i] + "\n";
    out += lines[start.line].substr(0, start.character);
    out += replacement;
    out += lines[end.line].substr(end.character);
    if (end.line + 1 < static_cast<int>(lines.size())) out += "\n";
    for (size_t i = end.line + 1; i < lines.size(); ++i) {
        out += lines[i];
        if (i + 1 < lines.size()) out += "\n";
    }
    return out;
}

int completionKindToLsp(const std::string& kind) {
    if (kind == "function") return 3;
    if (kind == "method") return 2;
    if (kind == "field") return 5;
    if (kind == "variable" || kind == "parameter") return 6;
    if (kind == "type") return 7;
    if (kind == "keyword") return 14;
    return 1; // Text
}

int symbolKindToLsp(const std::string& kind) {
    if (kind == "function") return 12;
    if (kind == "method") return 6;
    if (kind == "struct") return 23;
    if (kind == "enum") return 10;
    if (kind == "enumMember") return 22;
    if (kind == "variable") return 13;
    if (kind == "field") return 8;
    return 13; // Variable
}

struct DecodedToken {
    int line = 0;
    int start = 0;
    int length = 0;
    int type = 0;
    int modifiers = 0;
};

/// Maps query token names to legend indices, dropping unknown types and empty spans.
std::vector<DecodedToken> decodeSemanticTokens(const JsonValue* entries) {
    std::vector<DecodedToken> tokens;
    auto* array = entries ? entries->getAsArray() : nullptr;
    if (!array) return tokens;
    const auto& types = semanticTokenTypes();
    const auto& modifiers = semanticTokenModifiers();
    for (auto& entry : *array) {
        std::string typeName = getJsonString(entry, "type");
        int typeIndex = -1;
        for (size_t i = 0; i < types.size(); ++i) {
            if (types[i] == typeName) {
                typeIndex = static_cast<int>(i);
                break;
            }
        }
        if (typeIndex < 0) continue;
        int mask = 0;
        if (auto* mods = findJsonArray(entry, "modifiers")) {
            for (auto& mod : *mods) {
                auto modName = mod.getAsString();
                if (!modName) continue;
                for (size_t i = 0; i < modifiers.size(); ++i) {
                    if (*modName == modifiers[i]) mask |= 1 << static_cast<int>(i);
                }
            }
        }
        DecodedToken token;
        token.line = static_cast<int>(getJsonInt(entry, "line"));
        token.start = static_cast<int>(getJsonInt(entry, "start"));
        token.length = static_cast<int>(getJsonInt(entry, "length"));
        token.type = typeIndex;
        token.modifiers = mask;
        if (token.length <= 0) continue;
        tokens.push_back(token);
    }
    return tokens;
}

bool tokenOverlapsRange(const DecodedToken& token, LspPosition rangeStart, LspPosition rangeEnd) {
    if (token.line < rangeStart.line || token.line > rangeEnd.line) return false;
    if (token.line == rangeStart.line && token.start + token.length <= rangeStart.character) return false;
    if (token.line == rangeEnd.line && token.start >= rangeEnd.character) return false;
    return true;
}

/// Delta-encodes absolute tokens into the LSP `{"data": [...]}` response.
JsonValue encodeSemanticTokens(std::vector<DecodedToken> tokens) {
    std::sort(tokens.begin(), tokens.end(), [](const DecodedToken& a, const DecodedToken& b) {
        if (a.line != b.line) return a.line < b.line;
        return a.start < b.start;
    });
    JsonArray data;
    int prevLine = 0;
    int prevStart = 0;
    bool first = true;
    for (auto& token : tokens) {
        int deltaLine = first ? token.line : token.line - prevLine;
        int deltaStart = (first || token.line != prevLine) ? token.start : token.start - prevStart;
        data.push_back(deltaLine);
        data.push_back(deltaStart);
        data.push_back(token.length);
        data.push_back(token.type);
        data.push_back(token.modifiers);
        prevLine = token.line;
        prevStart = token.start;
        first = false;
    }
    JsonObject response;
    response["data"] = std::move(data);
    return JsonValue(std::move(response));
}

/// True for JSON-RPC requests (which demand a response), false for
/// notifications - including notifications with an explicit null id, which
/// must never be answered per spec.
bool isRequestMessage(const JsonValue* id) {
    return id && id->kind() != JsonValue::Null;
}

/// Runs one frontend compilation in a fresh `queryExecutable --query`
/// process. Returns the parsed "result" object on success, nullopt on any
/// failure (already logged).
std::optional<JsonValue> runQuerySubprocess(ServerState& state, JsonObject queryJson) {
    std::string queryText = serializeJson(JsonValue(std::move(queryJson)));

    llvm::SmallString<128> queryPath;
    llvm::SmallString<128> resultPath;
    int queryFd = 0, resultFd = 0;
    if (auto error = llvm::sys::fs::createTemporaryFile("cx-lsp-query", "json", queryFd, queryPath)) {
        logMessage("couldn't create query temp file: " + error.message());
        return std::nullopt;
    }
    // queryFd must be closed on every path below; ownership moves to the
    // raw_fd_ostream once writing starts.
    auto queryFdGuard = llvm::scope_exit([queryFd] {
        // A short-lived stream just to close the fd (same idiom as below).
        llvm::raw_fd_ostream(queryFd, /*shouldClose=*/true);
    });
    if (auto error = llvm::sys::fs::createTemporaryFile("cx-lsp-result", "json", resultFd, resultPath)) {
        logMessage("couldn't create result temp file: " + error.message());
        llvm::sys::fs::remove(queryPath);
        return std::nullopt;
    }
    queryFdGuard.release();
    {
        llvm::raw_fd_ostream queryFile(queryFd, /*shouldClose=*/true);
        queryFile << queryText;
    }
    // Close the reserved result fd; the child reopens the path for writing
    // via stdout redirection below.
    {
        llvm::raw_fd_ostream resultFile(resultFd, /*shouldClose=*/true);
    }

    std::string program = state.options.queryExecutable;
    std::vector<std::string> argsStorage = {program, "--query"};
    std::vector<llvm::StringRef> args;
    for (auto& arg : argsStorage)
        args.push_back(arg);

    std::vector<std::optional<llvm::StringRef>> redirects;
    redirects.push_back(llvm::StringRef(queryPath.str()));
    redirects.push_back(llvm::StringRef(resultPath.str()));
    redirects.push_back(std::nullopt);

    std::string errorMessage;
    bool executionFailed = false;
    int status = llvm::sys::ExecuteAndWait(program, args, std::nullopt, redirects, 30, 0, &errorMessage, &executionFailed);
    if (executionFailed || status != 0) {
        logMessage("query process failed (status " + std::to_string(status) + "): " + errorMessage);
        llvm::sys::fs::remove(queryPath);
        llvm::sys::fs::remove(resultPath);
        return std::nullopt;
    }

    auto buffer = llvm::MemoryBuffer::getFile(resultPath);
    llvm::sys::fs::remove(queryPath);
    llvm::sys::fs::remove(resultPath);
    if (!buffer) {
        logMessage("couldn't read query result");
        return std::nullopt;
    }
    try {
        JsonValue envelope = parseJson((*buffer)->getBuffer().str());
        if (!getJsonBool(envelope, "ok")) {
            logMessage("query failed: " + getJsonString(envelope, "error", "unknown error"));
            return std::nullopt;
        }
        if (auto* result = findJson(envelope, "result")) return *result;
        logMessage("query response is missing \"result\"");
        return std::nullopt;
    } catch (const JsonParseError& error) {
        logMessage(std::string("couldn't parse query result: ") + error.what());
        return std::nullopt;
    }
}

JsonObject buildBaseQuery(ServerState& state, const std::string& method, const OpenDocument& doc) {
    JsonObject query;
    query["method"] = method;
    query["file"] = doc.path;
    query["content"] = doc.text;
    JsonObject openDocs;
    for (auto& entry : state.openDocs) {
        openDocs[entry.getKey().str()] = entry.getValue().text;
    }
    query["openDocs"] = std::move(openDocs);
    JsonArray folders;
    for (auto& folder : state.workspaceFolders)
        folders.push_back(folder);
    query["workspaceFolders"] = std::move(folders);
    JsonArray paths;
    for (auto& path : state.importSearchPaths)
        paths.push_back(path);
    query["importSearchPaths"] = std::move(paths);
    JsonArray defines;
    for (auto& define : state.defines)
        defines.push_back(define);
    query["defines"] = std::move(defines);
    return query;
}

std::vector<ServerDiagnostic> checkDocument(ServerState& state, const OpenDocument& doc) {
    JsonObject query = buildBaseQuery(state, "check", doc);
    auto result = runQuerySubprocess(state, std::move(query));
    std::vector<ServerDiagnostic> diagnostics;
    if (!result) {
        // Keep the editor honest instead of silently going dark.
        ServerDiagnostic fallback;
        fallback.filePath = doc.path;
        fallback.range.start = {0, 0};
        fallback.range.end = {0, 1};
        fallback.severity = 1;
        fallback.message = "C* language server: analysis failed (see server log)";
        diagnostics.push_back(std::move(fallback));
        return diagnostics;
    }
    if (auto* items = findJsonArray(*result, "diagnostics")) {
        for (auto& item : *items) {
            ServerDiagnostic diagnostic;
            diagnostic.filePath = getJsonString(item, "file", doc.path);
            if (auto* range = findJson(item, "range")) {
                diagnostic.range.start = positionFromJson(findJson(*range, "start"));
                diagnostic.range.end = positionFromJson(findJson(*range, "end"));
            }
            diagnostic.severity = static_cast<int>(getJsonInt(item, "severity", 1));
            diagnostic.message = getJsonString(item, "message");
            if (auto* notes = findJson(item, "relatedInformation")) {
                if (notes->getAsArray()) diagnostic.relatedInformation = *notes;
            }
            diagnostics.push_back(std::move(diagnostic));
        }
    }
    return diagnostics;
}

void publishDiagnosticsFor(ServerState& state, const std::string& path) {
    auto it = state.openDocs.find(path);
    if (it == state.openDocs.end()) return;
    const OpenDocument& doc = it->second;
    std::vector<ServerDiagnostic> all = checkDocument(state, doc);

    // Merge into the per-file cache: the checked file is always refreshed
    // (cleared when clean); other files mentioned in this result are updated;
    // files not mentioned keep their previous diagnostics instead of being
    // wiped by an unrelated file's check.
    llvm::StringMap<std::vector<ServerDiagnostic>> byFile;
    for (auto& diagnostic : all)
        byFile[diagnostic.filePath].push_back(diagnostic);
    state.diagCache[path] = byFile.count(path) ? byFile[path] : std::vector<ServerDiagnostic>();
    for (auto& entry : byFile) {
        if (entry.getKey() != llvm::StringRef(path)) state.diagCache[entry.getKey()] = entry.getValue();
    }

    auto publish = [&](const std::string& openPath, const OpenDocument& open) {
        JsonObject params;
        params["uri"] = open.uri;
        params["version"] = open.version;
        JsonArray items;
        auto found = state.diagCache.find(openPath);
        if (found != state.diagCache.end()) {
            for (auto& diagnostic : found->second) {
                JsonObject entry;
                entry["range"] = serverRangeToJson(diagnostic.range);
                entry["severity"] = diagnostic.severity;
                entry["source"] = "cx";
                entry["message"] = diagnostic.message;
                entry["relatedInformation"] = diagnostic.relatedInformation;
                items.push_back(std::move(entry));
            }
        }
        params["diagnostics"] = std::move(items);
        writeLspMessage(serializeJson(makeNotification("textDocument/publishDiagnostics", JsonValue(std::move(params)))));
    };

    // Notify the checked file plus every other open file whose diagnostics
    // changed in this result; untouched files keep what was last published.
    publish(path, doc);
    for (auto& entry : byFile) {
        if (entry.getKey() == llvm::StringRef(path)) continue;
        auto open = state.openDocs.find(entry.getKey());
        if (open != state.openDocs.end()) publish(open->second.path, open->second);
    }
}

} // namespace

int runServer(const ServerOptions& options) {
    ServerState state;
    state.options = options;

    std::string messageText;
    while (true) {
        ReadResult read = readLspMessage(messageText);
        if (read == ReadResult::CleanEof) break;
        if (read == ReadResult::Malformed) continue;
        JsonValue message = nullptr;
        try {
            message = parseJson(messageText);
        } catch (const JsonParseError& error) {
            logMessage(std::string("dropping malformed message: ") + error.what());
            continue;
        }

        const JsonValue* id = findJson(message, "id");
        bool wantResponse = isRequestMessage(id);
        std::string method = getJsonString(message, "method");
        const JsonValue* params = findJson(message, "params");
        JsonValue emptyParams = JsonObject{};
        if (!params) params = &emptyParams;

        if (method != "exit" && state.shutdownRequested) {
            // Per spec, only `exit` is valid after `shutdown`.
            if (wantResponse) {
                writeLspMessage(serializeJson(makeErrorResponse(*id, -32602, "Server is shutting down")));
            }
            continue;
        }

        if (method == "initialize") {
            // Workspace folders + initializationOptions.
            if (auto* folders = findJsonArray(*params, "workspaceFolders")) {
                for (auto& folder : *folders) {
                    std::string uri = getJsonString(folder, "uri");
                    if (!uri.empty()) state.workspaceFolders.push_back(uriToPath(uri));
                }
            }
            if (auto* rootUri = findJson(*params, "rootUri")) {
                if (auto uri = rootUri->getAsString(); uri && !uri->empty() && state.workspaceFolders.empty()) {
                    state.workspaceFolders.push_back(uriToPath(uri->str()));
                }
            }
            if (state.workspaceFolders.empty()) {
                if (auto* rootPath = findJson(*params, "rootPath")) {
                    if (auto path = rootPath->getAsString(); path && !path->empty()) state.workspaceFolders.push_back(path->str());
                }
            }
            if (auto* initOptions = findJson(*params, "initializationOptions")) {
                if (auto* paths = findJsonArray(*initOptions, "importSearchPaths")) {
                    for (auto& path : *paths) {
                        if (auto str = path.getAsString()) state.importSearchPaths.push_back(str->str());
                    }
                }
                if (auto* defines = findJsonArray(*initOptions, "defines")) {
                    for (auto& define : *defines) {
                        if (auto str = define.getAsString()) state.defines.push_back(str->str());
                    }
                }
            }

            JsonObject capabilities;
            JsonObject sync;
            sync["openClose"] = true;
            sync["change"] = 1; // Full
            capabilities["textDocumentSync"] = std::move(sync);
            capabilities["hoverProvider"] = true;
            capabilities["definitionProvider"] = true;
            JsonObject completion;
            completion["resolveProvider"] = false;
            capabilities["completionProvider"] = std::move(completion);
            capabilities["documentSymbolProvider"] = true;
            capabilities["referencesProvider"] = true;
            JsonObject legend;
            JsonArray tokenTypes;
            for (auto& type : semanticTokenTypes())
                tokenTypes.push_back(type);
            legend["tokenTypes"] = std::move(tokenTypes);
            JsonArray tokenModifiers;
            for (auto& modifier : semanticTokenModifiers())
                tokenModifiers.push_back(modifier);
            legend["tokenModifiers"] = std::move(tokenModifiers);
            JsonObject semanticTokens;
            semanticTokens["legend"] = std::move(legend);
            semanticTokens["full"] = true;
            semanticTokens["range"] = true;
            capabilities["semanticTokensProvider"] = std::move(semanticTokens);

            JsonObject serverInfo;
            serverInfo["name"] = "cx-lsp";
            serverInfo["version"] = "0.1.0";

            JsonObject result;
            result["capabilities"] = std::move(capabilities);
            result["serverInfo"] = std::move(serverInfo);
            if (wantResponse) writeLspMessage(serializeJson(makeResponse(*id, JsonValue(std::move(result)))));
        } else if (method == "initialized") {
            // No-op.
        } else if (method == "shutdown") {
            state.shutdownRequested = true;
            if (wantResponse) {
                writeLspMessage(serializeJson(makeResponse(*id, JsonValue(nullptr))));
            }
        } else if (method == "exit") {
            return state.shutdownRequested ? 0 : 1;
        } else if (method == "textDocument/didOpen") {
            auto* doc = findJson(*params, "textDocument");
            if (!doc) continue;
            OpenDocument open;
            open.uri = getJsonString(*doc, "uri");
            open.path = uriToPath(open.uri);
            open.text = getJsonString(*doc, "text");
            open.version = getJsonInt(*doc, "version");
            state.openDocs[open.path] = open;
            publishDiagnosticsFor(state, open.path);
        } else if (method == "textDocument/didChange") {
            auto* docId = findJson(*params, "textDocument");
            auto* changes = findJsonArray(*params, "contentChanges");
            if (!docId || !changes || changes->empty()) continue;
            std::string uri = getJsonString(*docId, "uri");
            std::string path = uriToPath(uri);
            auto it = state.openDocs.find(path);
            if (it == state.openDocs.end()) continue;
            // Full sync: a change without "range" holds the whole document.
            // Ranged changes are only a fallback for clients that send them
            // anyway; they are applied in order.
            for (auto& change : *changes) {
                if (auto* range = findJson(change, "range")) {
                    it->second.text = applyIncrementalChange(it->second.text, positionFromJson(findJson(*range, "start")),
                                                             positionFromJson(findJson(*range, "end")), getJsonString(change, "text"));
                } else {
                    it->second.text = getJsonString(change, "text", it->second.text);
                }
            }
            it->second.version = getJsonInt(*docId, "version", it->second.version);
            publishDiagnosticsFor(state, path);
        } else if (method == "textDocument/didClose") {
            auto* docId = findJson(*params, "textDocument");
            if (!docId) continue;
            std::string path = uriToPath(getJsonString(*docId, "uri"));
            state.openDocs.erase(path);
            state.diagCache.erase(path);
        } else if (method == "textDocument/hover") {
            if (!wantResponse) continue;
            auto* docId = findJson(*params, "textDocument");
            auto* posJson = findJson(*params, "position");
            if (!docId) {
                writeLspMessage(serializeJson(makeResponse(*id, JsonValue(nullptr))));
                continue;
            }
            std::string path = uriToPath(getJsonString(*docId, "uri"));
            auto it = state.openDocs.find(path);
            if (it == state.openDocs.end()) {
                writeLspMessage(serializeJson(makeResponse(*id, JsonValue(nullptr))));
                continue;
            }
            JsonObject query = buildBaseQuery(state, "hover", it->second);
            JsonObject pos;
            LspPosition posValue = positionFromJson(posJson);
            pos["line"] = posValue.line;
            pos["character"] = posValue.character;
            query["position"] = std::move(pos);
            auto result = runQuerySubprocess(state, std::move(query));
            std::string text = result ? getJsonString(*result, "hover") : "";
            if (text.empty()) {
                writeLspMessage(serializeJson(makeResponse(*id, JsonValue(nullptr))));
            } else {
                JsonObject contents;
                contents["kind"] = "markdown";
                contents["value"] = text;
                JsonObject response;
                response["contents"] = std::move(contents);
                writeLspMessage(serializeJson(makeResponse(*id, JsonValue(std::move(response)))));
            }
        } else if (method == "textDocument/definition") {
            if (!wantResponse) continue;
            auto* docId = findJson(*params, "textDocument");
            auto* posJson = findJson(*params, "position");
            if (!docId) {
                writeLspMessage(serializeJson(makeResponse(*id, JsonValue(nullptr))));
                continue;
            }
            std::string path = uriToPath(getJsonString(*docId, "uri"));
            auto it = state.openDocs.find(path);
            if (it == state.openDocs.end()) {
                writeLspMessage(serializeJson(makeResponse(*id, JsonValue(nullptr))));
                continue;
            }
            JsonObject query = buildBaseQuery(state, "definition", it->second);
            JsonObject pos;
            LspPosition posValue = positionFromJson(posJson);
            pos["line"] = posValue.line;
            pos["character"] = posValue.character;
            query["position"] = std::move(pos);
            auto result = runQuerySubprocess(state, std::move(query));
            if (!result || !getJsonBool(*result, "found")) {
                writeLspMessage(serializeJson(makeResponse(*id, JsonValue(nullptr))));
            } else {
                JsonObject response;
                response["uri"] = pathToUri(getJsonString(*result, "file"));
                if (auto* range = findJson(*result, "range")) response["range"] = *range;
                writeLspMessage(serializeJson(makeResponse(*id, JsonValue(std::move(response)))));
            }
        } else if (method == "textDocument/completion") {
            if (!wantResponse) continue;
            auto* docId = findJson(*params, "textDocument");
            auto* posJson = findJson(*params, "position");
            if (!docId) {
                writeLspMessage(serializeJson(makeResponse(*id, JsonValue(JsonArray{}))));
                continue;
            }
            std::string path = uriToPath(getJsonString(*docId, "uri"));
            auto it = state.openDocs.find(path);
            if (it == state.openDocs.end()) {
                writeLspMessage(serializeJson(makeResponse(*id, JsonValue(JsonArray{}))));
                continue;
            }
            JsonObject query = buildBaseQuery(state, "completion", it->second);
            JsonObject pos;
            LspPosition posValue = positionFromJson(posJson);
            pos["line"] = posValue.line;
            pos["character"] = posValue.character;
            query["position"] = std::move(pos);
            auto result = runQuerySubprocess(state, std::move(query));
            JsonArray items;
            if (result) {
                if (auto* entries = findJsonArray(*result, "items")) {
                    for (auto& entry : *entries) {
                        JsonObject item;
                        item["label"] = getJsonString(entry, "label");
                        item["kind"] = completionKindToLsp(getJsonString(entry, "kind"));
                        item["detail"] = getJsonString(entry, "detail");
                        items.push_back(std::move(item));
                    }
                }
            }
            writeLspMessage(serializeJson(makeResponse(*id, JsonValue(std::move(items)))));
        } else if (method == "textDocument/documentSymbol") {
            if (!wantResponse) continue;
            auto* docId = findJson(*params, "textDocument");
            if (!docId) {
                writeLspMessage(serializeJson(makeResponse(*id, JsonValue(JsonArray{}))));
                continue;
            }
            std::string path = uriToPath(getJsonString(*docId, "uri"));
            auto it = state.openDocs.find(path);
            if (it == state.openDocs.end()) {
                writeLspMessage(serializeJson(makeResponse(*id, JsonValue(JsonArray{}))));
                continue;
            }
            JsonObject query = buildBaseQuery(state, "documentSymbol", it->second);
            auto result = runQuerySubprocess(state, std::move(query));
            JsonArray items;
            if (result) {
                if (auto* entries = findJsonArray(*result, "symbols")) {
                    for (auto& entry : *entries) {
                        JsonObject item;
                        item["name"] = getJsonString(entry, "name");
                        item["kind"] = symbolKindToLsp(getJsonString(entry, "kind"));
                        if (auto* range = findJson(entry, "range")) item["range"] = *range;
                        if (auto* selection = findJson(entry, "selectionRange")) item["selectionRange"] = *selection;
                        items.push_back(std::move(item));
                    }
                }
            }
            writeLspMessage(serializeJson(makeResponse(*id, JsonValue(std::move(items)))));
        } else if (method == "textDocument/references") {
            if (!wantResponse) continue;
            auto* docId = findJson(*params, "textDocument");
            auto* posJson = findJson(*params, "position");
            if (!docId) {
                writeLspMessage(serializeJson(makeResponse(*id, JsonValue(JsonArray{}))));
                continue;
            }
            std::string path = uriToPath(getJsonString(*docId, "uri"));
            auto it = state.openDocs.find(path);
            if (it == state.openDocs.end()) {
                writeLspMessage(serializeJson(makeResponse(*id, JsonValue(JsonArray{}))));
                continue;
            }
            JsonObject query = buildBaseQuery(state, "references", it->second);
            JsonObject pos;
            LspPosition posValue = positionFromJson(posJson);
            pos["line"] = posValue.line;
            pos["character"] = posValue.character;
            query["position"] = std::move(pos);
            auto result = runQuerySubprocess(state, std::move(query));
            JsonArray items;
            if (result) {
                if (auto* entries = findJsonArray(*result, "references")) {
                    for (auto& entry : *entries) {
                        JsonObject item;
                        item["uri"] = pathToUri(getJsonString(entry, "file"));
                        if (auto* range = findJson(entry, "range")) item["range"] = *range;
                        items.push_back(std::move(item));
                    }
                }
            }
            writeLspMessage(serializeJson(makeResponse(*id, JsonValue(std::move(items)))));
        } else if (method == "textDocument/semanticTokens/full" || method == "textDocument/semanticTokens/range") {
            if (!wantResponse) continue;
            auto* docId = findJson(*params, "textDocument");
            if (!docId) {
                writeLspMessage(serializeJson(makeResponse(*id, JsonValue(nullptr))));
                continue;
            }
            std::string path = uriToPath(getJsonString(*docId, "uri"));
            auto it = state.openDocs.find(path);
            if (it == state.openDocs.end()) {
                writeLspMessage(serializeJson(makeResponse(*id, JsonValue(nullptr))));
                continue;
            }
            JsonObject query = buildBaseQuery(state, "semanticTokens", it->second);
            auto result = runQuerySubprocess(state, std::move(query));
            if (!result) {
                // Null keeps the client's current tokens; empty data would wipe them.
                writeLspMessage(serializeJson(makeResponse(*id, JsonValue(nullptr))));
                continue;
            }
            std::vector<DecodedToken> tokens = decodeSemanticTokens(findJson(*result, "tokens"));
            if (method == "textDocument/semanticTokens/range") {
                LspPosition rangeStart;
                LspPosition rangeEnd;
                if (auto* range = findJson(*params, "range")) {
                    rangeStart = positionFromJson(findJson(*range, "start"));
                    rangeEnd = positionFromJson(findJson(*range, "end"));
                }
                std::vector<DecodedToken> filtered;
                for (auto& token : tokens) {
                    if (tokenOverlapsRange(token, rangeStart, rangeEnd)) filtered.push_back(token);
                }
                tokens = std::move(filtered);
            }
            writeLspMessage(serializeJson(makeResponse(*id, encodeSemanticTokens(std::move(tokens)))));
        } else {
            // Unknown method: per JSON-RPC, only requests get error responses.
            if (wantResponse) {
                writeLspMessage(serializeJson(makeErrorResponse(*id, -32601, "Method not found: " + method)));
            }
        }
    }
    return 0;
}

} // namespace cx::lsp
