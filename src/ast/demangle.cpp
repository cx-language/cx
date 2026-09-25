#include "demangle.h"
#include "mangle.h"
#include <vector>
#pragma warning(push, 0)
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/StringSwitch.h>
#pragma warning(pop)

using namespace cx;

namespace {

struct DemangleParser {
    llvm::StringRef input;
    size_t pos = 0;
    bool failed = false;
    // Nesting depth of parseType. Capped so hostile input cannot overflow the
    // stack; real symbols nest far below the limit.
    int depth = 0;

    char peek() {
        if (pos >= input.size()) {
            failed = true;
            return '\0';
        }
        return input[pos];
    }

    // Non-failing peek for optional markers.
    char peekOptional() {
        if (pos >= input.size()) return '\0';
        return input[pos];
    }

    bool consume(char expected) {
        if (peek() != expected) {
            failed = true;
            return false;
        }
        ++pos;
        return true;
    }

    // Reads a run of decimal digits. Fails on empty.
    std::string parseDecimal() {
        size_t start = pos;
        while (pos < input.size() && input[pos] >= '0' && input[pos] <= '9')
            ++pos;
        if (pos == start) failed = true;
        return input.substr(start, pos - start).str();
    }

    uint64_t parseNumber() {
        std::string digits = parseDecimal();
        uint64_t value = 0;
        for (char digit : digits) {
            uint64_t addend = static_cast<uint64_t>(digit - '0');
            if (value > (~uint64_t(0) - addend) / 10) {
                failed = true;
                return 0;
            }
            value = value * 10 + addend;
        }
        return value;
    }

    // length-prefixed escaped identifier, returned unescaped.
    std::string parseIdentifier() {
        uint64_t length = parseNumber();
        if (failed || pos + length > input.size()) {
            failed = true;
            return "";
        }
        llvm::StringRef escaped = input.substr(pos, static_cast<size_t>(length));
        pos += static_cast<size_t>(length);
        std::string name;
        if (!unescapeMangledIdentifier(escaped, name)) failed = true;
        return name;
    }

    uint64_t parseArity() {
        uint64_t count = parseNumber();
        consume('_');
        return count;
    }

    std::string parseType();

    struct GenericArg {
        std::string text;
        bool isInt = false;
    };

    GenericArg parseGenericArg() {
        if (peek() == 'N') {
            ++pos;
            bool negative = peekOptional() == 'n';
            if (negative) ++pos;
            std::string digits = parseDecimal();
            consume('_');
            return {(negative ? "-" : "") + digits, true};
        }
        return {parseType(), false};
    }

    std::vector<GenericArg> parseGenericArgs() {
        std::vector<GenericArg> args;
        if (peekOptional() != 'I') return args;
        ++pos;
        while (peek() != 'E' && !failed) {
            args.push_back(parseGenericArg());
        }
        consume('E');
        return args;
    }

    std::string formatGenericArgs(const std::vector<GenericArg>& args) {
        if (args.empty()) return "";
        std::string result = "<";
        for (auto& arg : args) {
            result += arg.text;
            if (&arg != &args.back()) result += ", ";
        }
        return result + ">";
    }

    std::string formatNamedType(const std::string& name, const std::vector<GenericArg>& args, const std::string& module = "") {
        if (name == "Array" && args.size() == 2 && !args[0].isInt && args[1].isInt) {
            return args[0].text + "[" + args[1].text + "]";
        }
        return (module.empty() ? "" : module + ".") + name + formatGenericArgs(args);
    }
};

std::string DemangleParser::parseType() {
    if (++depth > 256) {
        failed = true;
        return "";
    }
    bool isConst = peekOptional() == 'K';
    if (isConst) ++pos;

    std::string result;
    char kind = peek();
    if (kind == '0') {
        // Anonymous C type. The leading zero cannot start a length prefix:
        // mangled identifiers are never empty, so lengths never have leading zeros.
        ++pos;
        result = "<anonymous C type #" + parseDecimal() + ">";
        consume('_');
    } else if (kind >= '0' && kind <= '9') {
        result = formatNamedType(parseIdentifier(), parseGenericArgs());
    } else {
        ++pos;
        switch (kind) {
        case 'M': {
            std::string module = parseIdentifier();
            std::string name = parseIdentifier();
            result = formatNamedType(name, parseGenericArgs(), module);
            break;
        }
        case 'O':
            result = parseType() + "?";
            break;
        case 'P':
            result = parseType() + "*";
            break;
        case 'R':
            result = parseType() + "&";
            break;
        case 'A':
            result = parseType() + "[*]";
            break;
        case 'T': {
            uint64_t count = parseArity();
            result = "(";
            for (uint64_t i = 0; i < count && !failed; ++i) {
                std::string name = parseIdentifier();
                result += parseType() + " " + name;
                if (i + 1 < count) result += ", ";
            }
            result += ")";
            break;
        }
        case 'F': {
            bool isVariadic = peekOptional() == 'v';
            if (isVariadic) ++pos;
            uint64_t count = parseArity();
            result = "(";
            for (uint64_t i = 0; i < count && !failed; ++i) {
                result += parseType();
                if (i + 1 < count) result += ", ";
            }
            if (isVariadic) result += count == 0 ? "..." : ", ...";
            result += ") -> " + parseType();
            break;
        }
        default:
            failed = true;
            break;
        }
    }

    --depth;
    if (failed) return "";
    return isConst ? "const " + result : result;
}

const char* operatorSpelling(llvm::StringRef code) {
    return llvm::StringSwitch<const char*>(code)
        .Case("pl", "+")
        .Case("mi", "-")
        .Case("ml", "*")
        .Case("dv", "/")
        .Case("rm", "%")
        .Case("eq", "==")
        .Case("ne", "!=")
        .Case("lt", "<")
        .Case("gt", ">")
        .Case("le", "<=")
        .Case("ge", ">=")
        .Case("pp", "++")
        .Case("mm", "--")
        .Case("ix", "[]")
        .Case("ixa", "[]=")
        .Case("ixm", "[-]")
        .Case("ixma", "[-]=")
        .Default(nullptr);
}

std::string parseFunction(DemangleParser& parser) {
    parser.consume('N');
    std::string module = parser.parseIdentifier();

    std::string receiver;
    if (parser.peekOptional() == 'M') {
        parser.consume('M');
        std::string receiverModule = parser.parseIdentifier();
        std::string receiverName = parser.parseIdentifier();
        receiver = receiverName + parser.formatGenericArgs(parser.parseGenericArgs());
        if (receiverModule != module) receiver = receiverModule + "." + receiver;
    }

    std::string name;
    if (parser.peekOptional() == 'o') {
        ++parser.pos;
        if (parser.peekOptional() == 'x') {
            ++parser.pos;
            name = parser.parseIdentifier();
        } else {
            std::string code = parser.parseIdentifier();
            if (const char* spelling = operatorSpelling(code)) {
                name = spelling;
            } else {
                name = code;
            }
        }
    } else {
        name = parser.parseIdentifier();
    }
    name += parser.formatGenericArgs(parser.parseGenericArgs());

    bool isPack = parser.peekOptional() == 'V';
    if (isPack) ++parser.pos;
    bool isVariadic = parser.peekOptional() == 'v';
    if (isVariadic) ++parser.pos;
    parser.consume('E');

    std::string returns = parser.parseType();

    uint64_t paramCount = parser.parseArity();
    std::string params;
    for (uint64_t i = 0; i < paramCount && !parser.failed; ++i) {
        std::string label;
        if (parser.peekOptional() == 'L') {
            ++parser.pos;
            label = parser.parseIdentifier() + ": ";
        }
        params += label + parser.parseType();
        if (i + 1 < paramCount) params += ", ";
    }
    if (isVariadic) params += params.empty() ? "..." : ", ...";

    std::string captures;
    if (parser.peekOptional() == 'C') {
        ++parser.pos;
        uint64_t captureCount = parser.parseArity();
        captures = " [captures: ";
        for (uint64_t i = 0; i < captureCount && !parser.failed; ++i) {
            captures += parser.parseType();
            if (i + 1 < captureCount) captures += ", ";
        }
        captures += "]";
    }

    std::string result = module + ".";
    if (!receiver.empty()) result += receiver + ".";
    result += name + "(" + params + ")" + captures + " -> " + returns;
    if (isPack) result += " [pack]";
    return result;
}

std::string parseGlobal(DemangleParser& parser) {
    parser.consume('G');
    std::string module = parser.parseIdentifier();
    std::string name = parser.parseIdentifier();
    parser.consume('E');
    return module + "." + name + " (global)";
}

} // namespace

std::string cx::demangle(llvm::StringRef symbol) {
    if (symbol == "main") return "main";
    if (!symbol.empty() && symbol.front() == '\01') return symbol.drop_front().str();
    if (!symbol.starts_with("_CX1")) return symbol.str();

    DemangleParser parser{symbol.drop_front(4)};
    std::string result;
    if (parser.peek() == 'G') {
        result = parseGlobal(parser);
    } else {
        result = parseFunction(parser);
    }
    if (parser.failed || parser.pos != parser.input.size()) return symbol.str();
    return result;
}

std::string cx::demangleLine(llvm::StringRef line) {
    std::string result;
    size_t pos = 0;
    while (pos < line.size()) {
        size_t start = line.find("_CX1", pos);
        if (start == llvm::StringRef::npos) break;
        size_t end = start + 4;
        while (end < line.size() && (isAsciiAlnum(line[end]) || line[end] == '_'))
            ++end;
        result += line.substr(pos, start - pos).str();
        result += demangle(line.substr(start, end - start));
        pos = end;
    }
    return result + line.substr(pos).str();
}
