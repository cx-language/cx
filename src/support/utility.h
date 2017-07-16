#pragma once

#include <cstdlib> // std::abort
#include <fstream>
#include <ostream>
#include <string>
#include <vector>
#include <utility> // std::move
#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/raw_ostream.h>
#include "../ast/location.h"

namespace delta {

inline std::ostream& operator<<(std::ostream& stream, llvm::StringRef string) {
    return stream.write(string.data(), string.size());
}

template<typename SourceContainer, typename T, typename Param>
std::vector<T> map(const SourceContainer& source, T (& mapper)(Param)) {
    std::vector<T> result;
    result.reserve(source.size());
    for (const auto& element : source) result.emplace_back(mapper(element));
    return result;
}

/// Appends the elements of `target` to `source`.
template<typename TargetContainer, typename SourceContainer>
static void append(TargetContainer& target, const SourceContainer& source) {
    target.append(source.begin(), source.end());
}

template<typename T>
std::string toDisjunctiveList(llvm::ArrayRef<T> values, std::string (& stringifier)(T)) {
    std::string string;
    for (const T& value : values) {
        string += stringifier(value);
        if (values.size() > 2) string += ',';
        if (&value == values.end() - 2) string += " or ";
    }
    return string;
}

class CompileError {
public:
    CompileError(SourceLocation location, std::string&& message)
    : location(location), message(std::move(message)) { }
    void print() const;

private:
    SourceLocation location;
    std::string message;
};

template<typename T>
void printColored(const T& text, llvm::raw_ostream::Colors color) {
    if (llvm::outs().has_colors()) llvm::outs().changeColor(color, true);
    llvm::outs() << text;
    if (llvm::outs().has_colors()) llvm::outs().resetColor();
}

template<typename... Args>
[[noreturn]] void printErrorAndExit(Args&&... args) {
    printColored("error: ", llvm::raw_ostream::RED);
    using expander = int[];
    (void)expander{0, (void(void(printColored(std::forward<Args>(args),
                                              llvm::raw_ostream::SAVEDCOLOR))), 0)...};
    llvm::outs() << '\n';
    exit(1);
}

template<typename... Args>
[[noreturn]] void error(SourceLocation location, Args&&... args) {
    std::string message;
    llvm::raw_string_ostream messageStream(message);
    using expander = int[];
    (void)expander{0, (void(void(messageStream << std::forward<Args>(args))), 0)...};
    throw CompileError(location, std::move(messageStream.str()));
}

[[noreturn]] inline void fatalError(const char* message) {
    llvm::errs() << "FATAL ERROR: " << message << '\n';
    abort();
}

}
