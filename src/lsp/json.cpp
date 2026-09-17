#include "json.h"
#include <cctype>
#include <cstdio>

namespace cx::lsp {

std::string escapeJsonString(const std::string& value) {
    std::string out;
    out.reserve(value.size() + 2);
    for (unsigned char ch : value) {
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
        case '\b':
            out += "\\b";
            break;
        case '\f':
            out += "\\f";
            break;
        default:
            if (ch < 0x20) {
                char escaped[7] = {};
                std::snprintf(escaped, sizeof(escaped), "\\u%04x", ch);
                out += escaped;
            } else {
                out += static_cast<char>(ch);
            }
            break;
        }
    }
    return out;
}

std::string serializeJson(const JsonValue& value) {
    switch (value.type) {
    case JsonValue::Type::Null:
        return "null";
    case JsonValue::Type::Bool:
        return value.boolean ? "true" : "false";
    case JsonValue::Type::Number:
        return value.number.empty() ? "0" : value.number;
    case JsonValue::Type::String:
        return "\"" + escapeJsonString(value.str) + "\"";
    case JsonValue::Type::Array: {
        std::string out = "[";
        for (size_t i = 0; i < value.array.size(); ++i) {
            if (i != 0) out += ",";
            out += serializeJson(value.array[i]);
        }
        out += "]";
        return out;
    }
    case JsonValue::Type::Object: {
        std::string out = "{";
        for (size_t i = 0; i < value.object.size(); ++i) {
            if (i != 0) out += ",";
            out += "\"" + escapeJsonString(value.object[i].first) + "\":";
            out += serializeJson(value.object[i].second);
        }
        out += "}";
        return out;
    }
    }
    return "null";
}

namespace {

struct JsonParser {
    const char* pos;
    const char* end;
    int depth = 0;
    static constexpr int maxDepth = 100;

    [[noreturn]] void fail(const std::string& message) { throw JsonParseError("JSON parse error: " + message); }

    void skipWhitespace() {
        while (pos != end && (*pos == ' ' || *pos == '\t' || *pos == '\n' || *pos == '\r'))
            ++pos;
    }

    char peek() {
        if (pos == end) fail("unexpected end of input");
        return *pos;
    }

    char take() {
        if (pos == end) fail("unexpected end of input");
        return *pos++;
    }

    void expectLiteral(const char* literal) {
        for (const char* p = literal; *p; ++p) {
            if (pos == end || *pos != *p) fail(std::string("expected '") + literal + "'");
            ++pos;
        }
    }

    JsonValue parseValue() {
        if (++depth > maxDepth) fail("nesting too deep");
        skipWhitespace();
        char ch = peek();
        JsonValue value;
        switch (ch) {
        case '{':
            value = parseObject();
            break;
        case '[':
            value = parseArray();
            break;
        case '"':
            value = JsonValue::stringValue(parseString());
            break;
        case 't':
            expectLiteral("true");
            value = JsonValue::booleanValue(true);
            break;
        case 'f':
            expectLiteral("false");
            value = JsonValue::booleanValue(false);
            break;
        case 'n':
            expectLiteral("null");
            value = JsonValue::null();
            break;
        default:
            if (ch == '-' || (ch >= '0' && ch <= '9')) {
                value = parseNumber();
            } else {
                fail(std::string("unexpected character '") + ch + "'");
            }
            break;
        }
        --depth;
        return value;
    }

    JsonValue parseObject() {
        take(); // '{'
        std::vector<std::pair<std::string, JsonValue>> members;
        skipWhitespace();
        if (peek() == '}') {
            take();
            return JsonValue::objectValue(std::move(members));
        }
        while (true) {
            skipWhitespace();
            if (peek() != '"') fail("expected string key in object");
            std::string key = parseString();
            skipWhitespace();
            if (take() != ':') fail("expected ':' in object");
            JsonValue value = parseValue();
            members.emplace_back(std::move(key), std::move(value));
            skipWhitespace();
            char ch = take();
            if (ch == '}') break;
            if (ch != ',') fail("expected ',' or '}' in object");
        }
        return JsonValue::objectValue(std::move(members));
    }

    JsonValue parseArray() {
        take(); // '['
        std::vector<JsonValue> items;
        skipWhitespace();
        if (peek() == ']') {
            take();
            return JsonValue::arrayValue(std::move(items));
        }
        while (true) {
            items.push_back(parseValue());
            skipWhitespace();
            char ch = take();
            if (ch == ']') break;
            if (ch != ',') fail("expected ',' or ']' in array");
        }
        return JsonValue::arrayValue(std::move(items));
    }

    std::string parseString() {
        if (take() != '"') fail("expected string");
        std::string out;
        while (true) {
            if (pos == end) fail("unterminated string");
            char ch = *pos++;
            if (ch == '"') break;
            if (ch == '\\') {
                if (pos == end) fail("unterminated escape");
                char esc = *pos++;
                switch (esc) {
                case '"':
                    out += '"';
                    break;
                case '\\':
                    out += '\\';
                    break;
                case '/':
                    out += '/';
                    break;
                case 'b':
                    out += '\b';
                    break;
                case 'f':
                    out += '\f';
                    break;
                case 'n':
                    out += '\n';
                    break;
                case 'r':
                    out += '\r';
                    break;
                case 't':
                    out += '\t';
                    break;
                case 'u': {
                    if (end - pos < 4) fail("invalid \\u escape");
                    unsigned code = 0;
                    for (int i = 0; i < 4; ++i) {
                        char h = *pos++;
                        code <<= 4;
                        if (h >= '0' && h <= '9')
                            code |= static_cast<unsigned>(h - '0');
                        else if (h >= 'a' && h <= 'f')
                            code |= static_cast<unsigned>(h - 'a' + 10);
                        else if (h >= 'A' && h <= 'F')
                            code |= static_cast<unsigned>(h - 'A' + 10);
                        else
                            fail("invalid hex digit in \\u escape");
                    }
                    // Combine UTF-16 surrogate pairs into a single code point.
                    if (code >= 0xD800 && code <= 0xDBFF && end - pos >= 6 && pos[0] == '\\' && pos[1] == 'u') {
                        unsigned low = 0;
                        bool valid = true;
                        for (int i = 2; i < 6; ++i) {
                            char h = pos[i];
                            low <<= 4;
                            if (h >= '0' && h <= '9')
                                low |= static_cast<unsigned>(h - '0');
                            else if (h >= 'a' && h <= 'f')
                                low |= static_cast<unsigned>(h - 'a' + 10);
                            else if (h >= 'A' && h <= 'F')
                                low |= static_cast<unsigned>(h - 'A' + 10);
                            else
                                valid = false;
                        }
                        if (valid && low >= 0xDC00 && low <= 0xDFFF) {
                            pos += 6;
                            code = 0x10000 + ((code - 0xD800) << 10) + (low - 0xDC00);
                        }
                    }
                    // Encode as UTF-8.
                    if (code < 0x80) {
                        out += static_cast<char>(code);
                    } else if (code < 0x800) {
                        out += static_cast<char>(0xC0 | (code >> 6));
                        out += static_cast<char>(0x80 | (code & 0x3F));
                    } else if (code < 0x10000) {
                        out += static_cast<char>(0xE0 | (code >> 12));
                        out += static_cast<char>(0x80 | ((code >> 6) & 0x3F));
                        out += static_cast<char>(0x80 | (code & 0x3F));
                    } else {
                        out += static_cast<char>(0xF0 | (code >> 18));
                        out += static_cast<char>(0x80 | ((code >> 12) & 0x3F));
                        out += static_cast<char>(0x80 | ((code >> 6) & 0x3F));
                        out += static_cast<char>(0x80 | (code & 0x3F));
                    }
                    break;
                }
                default:
                    fail("invalid escape character");
                }
            } else {
                out += ch;
            }
        }
        return out;
    }

    JsonValue parseNumber() {
        const char* begin = pos;
        if (peek() == '-') take();
        if (pos == end) fail("invalid number");
        if (*pos == '0') {
            take();
        } else if (*pos >= '1' && *pos <= '9') {
            while (pos != end && *pos >= '0' && *pos <= '9')
                ++pos;
        } else {
            fail("invalid number");
        }
        if (pos != end && *pos == '.') {
            ++pos;
            if (pos == end || *pos < '0' || *pos > '9') fail("invalid number");
            while (pos != end && *pos >= '0' && *pos <= '9')
                ++pos;
        }
        if (pos != end && (*pos == 'e' || *pos == 'E')) {
            ++pos;
            if (pos != end && (*pos == '+' || *pos == '-')) ++pos;
            if (pos == end || *pos < '0' || *pos > '9') fail("invalid number");
            while (pos != end && *pos >= '0' && *pos <= '9')
                ++pos;
        }
        return JsonValue::numberValue(std::string(begin, pos));
    }
};

} // namespace

JsonValue parseJson(const std::string& text) {
    JsonParser parser{text.data(), text.data() + text.size()};
    JsonValue value = parser.parseValue();
    parser.skipWhitespace();
    if (parser.pos != parser.end) throw JsonParseError("JSON parse error: trailing characters");
    return value;
}

} // namespace cx::lsp
