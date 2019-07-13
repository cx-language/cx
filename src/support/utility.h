#pragma once

#include <cassert>
#include <ostream>
#include <string>
#include <utility> // std::move
#include <vector>
#pragma warning(push, 0)
#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/StringExtras.h>
#include <llvm/ADT/StringRef.h>
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

#define NOTNULL(x) (ASSERT(x), x)

class StringFormatter : public llvm::raw_string_ostream {
public:
    StringFormatter() : llvm::raw_string_ostream(message) {}

private:
    std::string message;
};

std::string readLineFromFile(SourceLocation location);
void renameFile(llvm::Twine sourcePath, llvm::Twine targetPath);
void printDiagnostic(SourceLocation location, llvm::StringRef type, llvm::raw_ostream::Colors color, llvm::StringRef message);

struct Note {
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

[[noreturn]] void abort(StringFormatter& message);
[[noreturn]] void errorWithNotes(SourceLocation location, std::vector<Note>&& notes, StringFormatter& message);
[[noreturn]] void error(SourceLocation location, StringFormatter& message);
void warn(SourceLocation location, StringFormatter& message);

enum class WarningMode { Default, Suppress, TreatAsErrors };
extern WarningMode warningMode;

#define ABORT(args) \
    { \
        StringFormatter s; \
        s << args; \
        abort(s); \
    }

#define ERROR_WITH_NOTES(location, notes, args) \
    { \
        StringFormatter s; \
        s << args; \
        errorWithNotes(location, notes, s); \
    }

#define ERROR(location, args) \
    { \
        StringFormatter s; \
        s << args; \
        error(location, s); \
    }

#define WARN(location, args) \
    { \
        StringFormatter s; \
        s << args; \
        warn(location, s); \
    }

std::string getCCompilerPath();

} // namespace delta
