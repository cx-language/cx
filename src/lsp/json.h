#pragma once

#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#pragma warning(push, 0)
#include <llvm/ADT/StringRef.h>
#pragma warning(pop)

namespace cx::lsp {

struct JsonParseError : std::runtime_error {
    using std::runtime_error::runtime_error;
};

/// Minimal JSON value implementation sufficient for LSP (no external deps).
struct JsonValue {
    enum class Type { Null, Bool, Number, String, Array, Object };

    Type type = Type::Null;
    bool boolean = false;
    std::string number; // Preserved verbatim (integer or floating point).
    std::string str;
    std::vector<JsonValue> array;
    // Objects preserve insertion order for stable output.
    std::vector<std::pair<std::string, JsonValue>> object;

    static JsonValue null() { return JsonValue(); }
    static JsonValue booleanValue(bool b) {
        JsonValue v;
        v.type = Type::Bool;
        v.boolean = b;
        return v;
    }
    static JsonValue numberValue(std::string n) {
        JsonValue v;
        v.type = Type::Number;
        v.number = std::move(n);
        return v;
    }
    static JsonValue numberValue(long long n) { return numberValue(std::to_string(n)); }
    static JsonValue numberValue(int n) { return numberValue(static_cast<long long>(n)); }
    static JsonValue stringValue(std::string s) {
        JsonValue v;
        v.type = Type::String;
        v.str = std::move(s);
        return v;
    }
    static JsonValue arrayValue(std::vector<JsonValue>&& items = {}) {
        JsonValue v;
        v.type = Type::Array;
        v.array = std::move(items);
        return v;
    }
    static JsonValue objectValue(std::vector<std::pair<std::string, JsonValue>>&& members = {}) {
        JsonValue v;
        v.type = Type::Object;
        v.object = std::move(members);
        return v;
    }

    bool isNull() const { return type == Type::Null; }
    bool isBool() const { return type == Type::Bool; }
    bool isNumber() const { return type == Type::Number; }
    bool isString() const { return type == Type::String; }
    bool isArray() const { return type == Type::Array; }
    bool isObject() const { return type == Type::Object; }

    const JsonValue* find(const std::string& key) const {
        if (type != Type::Object) return nullptr;
        for (auto& member : object) {
            if (member.first == key) return &member.second;
        }
        return nullptr;
    }
    JsonValue* find(const std::string& key) {
        if (type != Type::Object) return nullptr;
        for (auto& member : object) {
            if (member.first == key) return &member.second;
        }
        return nullptr;
    }
    bool has(const std::string& key) const { return find(key) != nullptr; }

    std::string getString(const std::string& key, const std::string& fallback = "") const {
        if (auto* v = find(key)) {
            if (v->type == Type::String) return v->str;
        }
        return fallback;
    }
    long long getInt(const std::string& key, long long fallback = 0) const {
        if (auto* v = find(key)) {
            if (v->type == Type::Number) {
                long long result = 0;
                if (!llvm::StringRef(v->number).getAsInteger(10, result)) return result;
                return fallback;
            }
            if (v->type == Type::Bool) return v->boolean ? 1 : 0;
        }
        return fallback;
    }
    bool getBool(const std::string& key, bool fallback = false) const {
        if (auto* v = find(key)) {
            if (v->type == Type::Bool) return v->boolean;
        }
        return fallback;
    }

    std::string asString() const { return type == Type::String ? str : ""; }
    long long asInt(long long fallback = 0) const {
        if (type != Type::Number) return fallback;
        long long result = 0;
        if (!llvm::StringRef(number).getAsInteger(10, result)) return result;
        return fallback;
    }
    bool asBool(bool fallback = false) const { return type == Type::Bool ? boolean : fallback; }

    void set(std::string key, JsonValue value) {
        if (type != Type::Object) {
            type = Type::Object;
            object.clear();
        }
        for (auto& member : object) {
            if (member.first == key) {
                member.second = std::move(value);
                return;
            }
        }
        object.emplace_back(std::move(key), std::move(value));
    }
};

/// Parses a complete JSON document. Throws JsonParseError on failure.
JsonValue parseJson(const std::string& text);

/// Serializes a JSON value with minimal escaping.
std::string serializeJson(const JsonValue& value);

/// Escapes a string for inclusion in JSON output (without surrounding quotes).
std::string escapeJsonString(const std::string& value);

} // namespace cx::lsp
