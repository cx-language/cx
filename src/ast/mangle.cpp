#include "mangle.h"
#include <cstdio>
#pragma warning(push, 0)
#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/StringSwitch.h>
#pragma warning(pop)
#include "../ast/decl.h"
#include "../ast/module.h"

using namespace cx;

static const char cxPrefix[] = "_CX1";

std::string cx::escapeMangledIdentifier(llvm::StringRef name) {
    std::string escaped;
    for (char c : name) {
        if (isAsciiAlnum(c)) {
            escaped += c;
        } else if (c == '_') {
            escaped += "__";
        } else {
            char hex[4];
            snprintf(hex, sizeof(hex), "_%02X", static_cast<unsigned char>(c));
            escaped += hex;
        }
    }
    return escaped;
}

bool cx::unescapeMangledIdentifier(llvm::StringRef escaped, std::string& out) {
    out.clear();
    for (size_t i = 0; i < escaped.size();) {
        char ch = escaped[i];
        if (ch != '_') {
            if (!isAsciiAlnum(ch)) return false;
            out += ch;
            ++i;
        } else if (escaped.substr(i, 2) == "__") {
            out += '_';
            i += 2;
        } else {
            if (i + 3 > escaped.size()) return false;
            auto hexValue = [](char digit) -> int {
                if (digit >= '0' && digit <= '9') return digit - '0';
                if (digit >= 'A' && digit <= 'F') return digit - 'A' + 10;
                return -1;
            };
            int high = hexValue(escaped[i + 1]), low = hexValue(escaped[i + 2]);
            if (high < 0 || low < 0) return false;
            out += (char)(high * 16 + low);
            i += 3;
        }
    }
    return true;
}

llvm::StringRef cx::stripAsmLabelMarker(llvm::StringRef symbol) {
    if (!symbol.empty() && symbol.front() == '\01') return symbol.drop_front();
    return symbol;
}

bool cx::isAsciiAlnum(char ch) {
    auto code = static_cast<unsigned char>(ch);
    return (code >= '0' && code <= '9') || (code >= 'A' && code <= 'Z') || (code >= 'a' && code <= 'z');
}

static bool isOperator(const FunctionDecl& functionDecl) {
    auto ch = static_cast<unsigned char>(functionDecl.getName().front());
    bool isAlpha = (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z');
    return !isAlpha && ch != '_';
}

static const char* operatorCode(llvm::StringRef name) {
    return llvm::StringSwitch<const char*>(name)
        .Case("+", "pl")
        .Case("-", "mi")
        .Case("*", "ml")
        .Case("/", "dv")
        .Case("%", "rm")
        .Case("==", "eq")
        .Case("!=", "ne")
        .Case("<", "lt")
        .Case(">", "gt")
        .Case("<=", "le")
        .Case(">=", "ge")
        .Case("++", "pp")
        .Case("--", "mm")
        .Case("[]", "ix")
        .Case("[]=", "ixa")
        .Case("[-]", "ixm")
        .Case("[-]=", "ixma")
        .Default(nullptr);
}

static void mangleIdentifier(llvm::raw_string_ostream& stream, llvm::StringRef name) {
    std::string escaped = escapeMangledIdentifier(name);
    stream << escaped.size();
    stream << escaped;
}

static void mangleGenericArgs(llvm::raw_string_ostream& stream, llvm::ArrayRef<GenericArg> genericArgs) {
    if (!genericArgs.empty()) {
        stream << 'I';
        for (GenericArg genericArg : genericArgs) {
            if (genericArg.isInt()) {
                int64_t value = genericArg.getInt();
                uint64_t magnitude = value < 0 ? 0ULL - static_cast<uint64_t>(value) : static_cast<uint64_t>(value);
                stream << 'N';
                if (value < 0) stream << 'n';
                stream << magnitude << '_';
            } else {
                mangleType(stream, genericArg.getType());
            }
        }
        stream << 'E';
    }
}

void cx::mangleType(llvm::raw_string_ostream& stream, Type type) {
    // Spelling twins share one identity; mangle the canonical one so the same
    // type never gets two symbols.
    type = type.canonicalTwin();
    if (!type.isMutable()) stream << 'K';

    switch (type.getKind()) {
    case TypeKind::BasicType:
        if (type.isOptionalType()) {
            stream << 'O';
            mangleType(stream, type.getWrappedType());
        } else if (type.getName().empty()) {
            // Anonymous C types have no name; the creation-order index tells them apart.
            stream << '0' << type.typeBase->identityIndex << '_';
        } else if (TypeDecl* typeDecl = type.getDecl()) {
            stream << 'M';
            mangleIdentifier(stream, typeDecl->getModule()->name);
            mangleIdentifier(stream, type.getName());
            mangleGenericArgs(stream, type.getGenericArgs());
        } else {
            mangleIdentifier(stream, type.getName());
            mangleGenericArgs(stream, type.getGenericArgs());
        }
        break;
    case TypeKind::ArrayPointerType:
        stream << 'A';
        mangleType(stream, type.getElementType());
        break;
    case TypeKind::AnonymousStructType: {
        auto elements = type.getAnonymousStructElements();
        stream << 'T' << elements.size() << '_';
        for (auto& element : elements) {
            mangleIdentifier(stream, element.name);
            mangleType(stream, element.type);
        }
        break;
    }
    case TypeKind::FunctionType: {
        auto paramTypes = type.getParamTypes();
        stream << 'F';
        if (llvm::cast<FunctionType>(type.typeBase)->isVariadic) stream << 'v';
        stream << paramTypes.size() << '_';
        for (Type paramType : paramTypes) {
            mangleType(stream, paramType);
        }
        mangleType(stream, type.getReturnType());
        break;
    }
    case TypeKind::PointerType:
        stream << (type.isReferenceType() ? 'R' : 'P');
        mangleType(stream, type.getPointee());
        break;
    case TypeKind::UnresolvedType:
        llvm_unreachable("invalid unresolved type");
    }
}

std::string cx::mangleFunctionDecl(const FunctionDecl& functionDecl) {
    std::string mangled;
    llvm::raw_string_ostream stream(mangled);

    if (!functionDecl.proto.asmLabel.empty()) {
        stream << '\01' << functionDecl.proto.asmLabel;
    } else if (functionDecl.isExtern()) {
        stream << functionDecl.getName();
    } else if (functionDecl.isEntryPoint) {
        stream << "main";
    } else {
        stream << cxPrefix;
        stream << 'N';
        mangleIdentifier(stream, functionDecl.module.name);

        if (auto* typeDecl = functionDecl.getTypeDecl()) {
            stream << 'M';
            mangleIdentifier(stream, typeDecl->getModule()->name);
            mangleIdentifier(stream, typeDecl->getName());
            mangleGenericArgs(stream, typeDecl->genericArgs);
        }

        if (isOperator(functionDecl)) {
            if (const char* code = operatorCode(functionDecl.getName())) {
                stream << 'o';
                mangleIdentifier(stream, code);
            } else {
                stream << "ox";
                mangleIdentifier(stream, functionDecl.getName());
            }
        } else {
            mangleIdentifier(stream, functionDecl.getName());
        }

        mangleGenericArgs(stream, functionDecl.genericArgs);
        if (functionDecl.isPackInstantiation) stream << 'V';
        if (functionDecl.isVariadic()) stream << 'v';
        stream << 'E';

        mangleType(stream, functionDecl.getReturnType());

        auto params = functionDecl.getParams();
        stream << params.size() << '_';
        for (auto& param : params) {
            if (param.isPublic) {
                stream << 'L';
                mangleIdentifier(stream, param.getName());
            }
            mangleType(stream, param.type);
        }

        if (!functionDecl.captures.empty()) {
            stream << 'C' << functionDecl.captures.size() << '_';
            for (auto* captured : functionDecl.captures) {
                mangleType(stream, captured->getCaptureType());
            }
        }
    }

    stream.flush();
    return mangled;
}

std::string cx::mangleGlobalVar(const VarDecl& varDecl) {
    if (varDecl.module.isCHeaderImport) return varDecl.getName().str();

    std::string mangled;
    llvm::raw_string_ostream stream(mangled);
    stream << cxPrefix << 'G';
    mangleIdentifier(stream, varDecl.module.name);
    mangleIdentifier(stream, varDecl.getName());
    stream << 'E';
    stream.flush();
    return mangled;
}

std::string cx::mangleType(Type type) {
    std::string mangledName;
    llvm::raw_string_ostream stream(mangledName);
    mangleType(stream, type);
    return std::move(stream.str());
}
