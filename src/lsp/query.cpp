#include "query.h"
#include "analyzer.h"
#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>

namespace cx::lsp {

std::string readAllStdin() {
    // The query is a single JSON document of unknown size.
    std::string input;
    char buffer[4096];
    size_t chunk = 0;
    while ((chunk = std::fread(buffer, 1, sizeof(buffer), stdin)) > 0) {
        input.append(buffer, chunk);
    }
    return input;
}

int runQueryProcess() {
    std::string input = readAllStdin();
    JsonObject envelope;
    try {
        JsonValue result = handleQuery(parseJson(input));
        envelope["ok"] = true;
        envelope["result"] = std::move(result);
    } catch (const JsonParseError& error) {
        envelope["ok"] = false;
        envelope["error"] = std::string(error.what());
    } catch (const std::exception& error) {
        // Never let a compiler crash take down the server: report it as a
        // failed query and let the server turn it into a diagnostic.
        envelope["ok"] = false;
        envelope["error"] = std::string("internal compiler error: ") + error.what();
    } catch (...) {
        envelope["ok"] = false;
        envelope["error"] = "internal compiler error";
    }
    std::string output = serializeJson(JsonValue(std::move(envelope)));
    std::fwrite(output.data(), 1, output.size(), stdout);
    return 0;
}

int runLeakCheck(const char* filePath, int count) {
// Without instrumentation the comparison is vacuous, so refuse loudly. The
// body below still compiles everywhere, keeping it free of bitrot. Under
// AddressSanitizer the repeated analyses and arena resets run as a memory
// error check instead (see test/asan/check_asan.py).
#ifdef __has_feature
#if __has_feature(leak_sanitizer) || __has_feature(address_sanitizer)
#define CX_SANITIZER_INSTRUMENTED 1
#endif
#endif
#ifndef CX_SANITIZER_INSTRUMENTED
    std::fputs("leak-check: rebuild with -fsanitize=address or -fsanitize=leak (Clang)\n", stdout);
    return 1;
#endif
#undef CX_SANITIZER_INSTRUMENTED
    if (!filePath || count < 1) {
        std::fputs("usage: cx-lsp --leak-check <file> <count>\n", stderr);
        return 1;
    }
    std::ifstream input(filePath);
    if (!input) {
        std::fputs("cx-lsp: cannot open leak-check file\n", stderr);
        return 1;
    }
    std::ostringstream content;
    content << input.rdbuf();
    std::string base = content.str();
    LspSession session;
    bool failed = false;
    auto runOnce = [&](const std::string& text) {
        LspQuery query;
        query.method = "check";
        query.filePath = filePath;
        query.content = text;
        // A failed analysis may leak through untaken cleanup paths; report
        // on stdout (stderr belongs to the sanitizer, whose exit code would
        // mask ours) so the gate fails loudly instead of measuring noise.
        if (!session.handle(std::move(query))) {
            std::fputs("leak-check: analysis failed\n", stdout);
            failed = true;
        }
    };
    // Unique trailing comment per iteration: the session cache keys on
    // content, so every analysis recompiles from reset globals.
    for (int i = 0; i < count; ++i) {
        runOnce(base + "\n// leak-check " + std::to_string(i) + "\n");
    }
    // Drop everything before exit: with no surviving cached frontend, the
    // LSan report holds one-time allocations plus whatever the resets
    // failed to free, independent of retained-analysis size noise.
    session.dropCache();
    return failed ? 1 : 0;
}

} // namespace cx::lsp
