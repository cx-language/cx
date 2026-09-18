#include "query.h"
#include "analyzer.h"
#include "json.h"
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

int runQueryProcess(const char* argv0) {
    std::string input = readAllStdin();
    try {
        JsonValue query = parseJson(input);
        JsonValue result = handleQuery(query, argv0);
        JsonValue envelope = JsonValue::objectValue();
        envelope.set("ok", JsonValue::booleanValue(true));
        envelope.set("result", std::move(result));
        std::string output = serializeJson(envelope);
        std::fwrite(output.data(), 1, output.size(), stdout);
        return 0;
    } catch (const JsonParseError& error) {
        JsonValue envelope = JsonValue::objectValue();
        envelope.set("ok", JsonValue::booleanValue(false));
        envelope.set("error", JsonValue::stringValue(error.what()));
        std::string output = serializeJson(envelope);
        std::fwrite(output.data(), 1, output.size(), stdout);
        return 0;
    } catch (const std::exception& error) {
        // Never let a compiler crash take down the server: report it as a
        // failed query and let the server turn it into a diagnostic.
        JsonValue envelope = JsonValue::objectValue();
        envelope.set("ok", JsonValue::booleanValue(false));
        envelope.set("error", JsonValue::stringValue(std::string("internal compiler error: ") + error.what()));
        std::string output = serializeJson(envelope);
        std::fwrite(output.data(), 1, output.size(), stdout);
        return 0;
    } catch (...) {
        JsonValue envelope = JsonValue::objectValue();
        envelope.set("ok", JsonValue::booleanValue(false));
        envelope.set("error", JsonValue::stringValue("internal compiler error"));
        std::string output = serializeJson(envelope);
        std::fwrite(output.data(), 1, output.size(), stdout);
        return 0;
    }
}

} // namespace cx::lsp
