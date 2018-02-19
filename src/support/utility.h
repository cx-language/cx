#pragma once

#include <cassert>
#include <cstdlib> // std::abort
#include <fstream>
#include <memory>
#include <ostream>
#include <string>
#include <utility> // std::move
#include <vector>
#pragma warning(push, 0)
#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/StringExtras.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/raw_ostream.h>
#pragma warning(pop)
#include "../ast/location.h"

#ifndef NDEBUG
#define ASSERT(condition, ...) assert(condition)
#else
// Prevent unused variable warnings without evaluating the condition value:
#define ASSERT(condition, ...) ((void) sizeof(condition))
#endif

namespace delta {

struct Type;

inline std::ostream& operator<<(std::ostream& stream, llvm::StringRef string) {
    return stream.write(string.data(), string.size());
}

template<typename T, typename U>
std::unique_ptr<T> cast_unique_ptr(std::unique_ptr<U> ptr) {
    return std::unique_ptr<T>(llvm::cast<T>(ptr.release()));
}

template<typename SourceContainer, typename Mapper>
auto map(const SourceContainer& source, Mapper mapper) -> std::vector<decltype(mapper(*source.begin()))> {
    std::vector<decltype(mapper(*source.begin()))> result;
    result.reserve(source.size());
    for (auto& element : source) {
        result.emplace_back(mapper(element));
    }
    return result;
}

template<typename T>
std::vector<T> instantiate(llvm::ArrayRef<T> elements, const llvm::StringMap<Type>& genericArgs) {
    return map(elements, [&](const T& element) { return element->instantiate(genericArgs); });
}

/// Appends the elements of `source` to `target`.
template<typename TargetContainer, typename SourceContainer>
static void append(TargetContainer& target, const SourceContainer& source) {
    target.insert(target.end(), source.begin(), source.end());
}

template<typename T>
std::string toDisjunctiveList(llvm::ArrayRef<T> values, std::string (&stringifier)(T)) {
    std::string string;
    for (const T& value : values) {
        string += stringifier(value);
        if (values.size() > 2) string += ',';
        if (&value == values.end() - 2) string += " or ";
    }
    return string;
}

template<typename T>
struct StateSaver {
    StateSaver(T& state) : state(state), savedState(std::move(state)) {}
    ~StateSaver() { state = std::move(savedState); }

private:
    T& state;
    T savedState;
};

template<typename T>
StateSaver<T> makeStateSaver(T& state) {
    return StateSaver<T>(state);
}

#define CONCAT_IMPL(a, b) a##b
#define CONCAT(a, b) CONCAT_IMPL(a, b)
#define SAVE_STATE(state) const auto CONCAT(stateSaver, __COUNTER__) = makeStateSaver(state)

void skipWhitespace(llvm::StringRef& string);
llvm::StringRef readWord(llvm::StringRef& string);
llvm::StringRef readLine(llvm::StringRef& string);
std::string readLineFromFile(SourceLocation location);
void printDiagnostic(SourceLocation location, llvm::StringRef type, llvm::raw_ostream::Colors color, llvm::StringRef message);

class Note {
public:
    template<typename... Args>
    Note(SourceLocation location, Args&&... args)
    : location(location), message(llvm::join_items("", std::forward<Args>(args)...)) {}
    SourceLocation getLocation() const { return location; }
    llvm::StringRef getMessage() const { return message; }

private:
    SourceLocation location;
    std::string message;
};

class CompileError {
public:
    CompileError(SourceLocation location, std::string&& message, std::vector<Note>&& notes)
    : location(location), message(std::move(message)), notes(std::move(notes)) {}
    void print() const;

private:
    SourceLocation location;
    std::string message;
    std::vector<Note> notes;
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
    (void) expander{ 0, (void(void(printColored(std::forward<Args>(args), llvm::raw_ostream::SAVEDCOLOR))), 0)... };
    llvm::outs() << '\n';
    exit(1);
}

template<typename... Args>
[[noreturn]] void errorWithNotes(SourceLocation location, std::vector<Note>&& notes, Args&&... args) {
    std::string message;
    llvm::raw_string_ostream messageStream(message);
    using expander = int[];
    (void) expander{ 0, (void(void(messageStream << std::forward<Args>(args))), 0)... };
    throw CompileError(location, std::move(messageStream.str()), std::move(notes));
}

template<typename... Args>
[[noreturn]] void error(SourceLocation location, Args&&... args) {
    errorWithNotes(location, std::vector<Note>(), std::forward<Args>(args)...);
}

extern bool treatWarningsAsErrors;

template<typename... Args>
void warning(SourceLocation location, Args&&... args) {
    if (treatWarningsAsErrors) {
        error(location, std::forward<Args>(args)...);
    }

    std::string message;
    llvm::raw_string_ostream messageStream(message);
    using expander = int[];
    (void) expander{ 0, (void(void(messageStream << std::forward<Args>(args))), 0)... };
    printDiagnostic(location, "warning", llvm::raw_ostream::YELLOW, messageStream.str());
}

std::string getCCompilerPath();

} // namespace delta
