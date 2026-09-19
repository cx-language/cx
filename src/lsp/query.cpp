#include "query.h"
#include "analyzer.h"
#include <cstdio>
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

} // namespace cx::lsp
