// WebAssembly API for the online playground.
//
// This file is compiled with Emscripten to a cx-wasm module that runs the
// cx frontend (parsing, type checking, IR generation, C code generation)
// entirely in the browser. It exposes two functions:
//
//   std::string cxCompileToC(std::string source, std::string importSearchPath)
//
// which compiles cx source code to C code and returns a JSON object:
//   {"status": 0, "cCode": "..."} on success,
//   {"status": 1} on failure (with an optional "internalError" message if the
//   failure happened inside the compiler rather than in the user program).
//
//   std::string cxComplete(std::string source, std::string importSearchPath,
//                          int line, int character)
//
// which returns the LSP completions at the 0-based cursor position as a JSON
// object: {"status": 0, "items": [{"label", "kind", "detail", "hasParams"}]}.
//
// Diagnostics (errors and warnings) are written to stderr, which the
// JavaScript glue code captures. The generated C code is then compiled to an
// executable WebAssembly module by the playground's in-browser C toolchain
// (see website/playground.js) and run.
//
// The standard library is provided via the virtual file system: the build
// preloads the repository's std/ directory at /cx/std (so the playground
// passes "/cx" as the import search path), and the playground writes the
// user's program to main.cx in the working directory so that diagnostics can
// show the surrounding source lines.
//
// This file is excluded from the native build.
#ifdef __EMSCRIPTEN__
#include <emscripten/bind.h>
#endif

#include "../driver/compile.h"
#include "../lsp/analyzer.h"
#include <cstdio>
#include <exception>
#include <string>

namespace {

void appendJsonEscaped(std::string& out, const std::string& value) {
    out += '"';
    for (char ch : value) {
        switch (ch) {
        case '"':
            out += "\\\"";
            break;
        case '\\':
            out += "\\\\";
            break;
        case '\n':
            out += "\\n";
            break;
        case '\r':
            out += "\\r";
            break;
        case '\t':
            out += "\\t";
            break;
        default:
            if (ch >= 0 && ch < 0x20) {
                char escaped[7] = {};
                std::snprintf(escaped, sizeof(escaped), "\\u%04x", ch);
                out += escaped;
            } else {
                out += ch;
            }
            break;
        }
    }
    out += '"';
}

} // namespace

std::string cxCompileToC(const std::string& source, const std::string& importSearchPath) {
    try {
        cx::CompileToCOptions options;
        options.importSearchPaths.push_back(importSearchPath);
        // The playground's in-browser C toolchain can't handle arbitrary
        // gotos, so always generate goto-free dispatch code.
        options.dispatchMode = true;
        auto result = cx::compileToC("main.cx", source.c_str(), options);

        std::string json = "{\"status\":";
        json += (result.status == 0 ? '0' : '1');
        if (result.status == 0) {
            json += ",\"cCode\":";
            appendJsonEscaped(json, result.cCode);
        }
        json += '}';
        return json;
    } catch (const std::exception& error) {
        std::string json = "{\"status\":1,\"internalError\":";
        appendJsonEscaped(json, error.what());
        json += '}';
        return json;
    } catch (...) {
        return "{\"status\":1,\"internalError\":\"unknown internal compiler error\"}";
    }
}

std::string cxComplete(const std::string& source, const std::string& importSearchPath, int line, int character) {
    try {
        cx::lsp::JsonObject query;
        query["method"] = "completion";
        // Relative, like the compile entry point's filename: the analyzer
        // reads the main file from the in-memory content below, so nothing
        // needs staging on the file system.
        query["file"] = "main.cx";
        query["content"] = source;
        cx::lsp::JsonArray searchPaths;
        searchPaths.push_back(importSearchPath);
        query["importSearchPaths"] = std::move(searchPaths);
        cx::lsp::JsonObject position;
        position["line"] = line < 0 ? 0 : line;
        position["character"] = character < 0 ? 0 : character;
        query["position"] = std::move(position);

        cx::lsp::JsonValue result = cx::lsp::handleQuery(cx::lsp::JsonValue(std::move(query)));
        std::string json = "{\"status\":0,\"items\":";
        if (const cx::lsp::JsonValue* items = cx::lsp::findJson(result, "items")) {
            json += cx::lsp::serializeJson(*items);
        } else {
            json += "[]";
        }
        json += '}';
        return json;
    } catch (const std::exception& error) {
        std::string json = "{\"status\":1,\"internalError\":";
        appendJsonEscaped(json, error.what());
        json += '}';
        return json;
    } catch (...) {
        return "{\"status\":1,\"internalError\":\"unknown internal compiler error\"}";
    }
}

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_BINDINGS(cx_wasm) {
    emscripten::function("cxCompileToC", &cxCompileToC);
    emscripten::function("cxComplete", &cxComplete);
}
#endif
