#pragma once

#include <cassert>
#include <iosfwd>
#include <string>
#include <utility> // std::move
#include <vector>
#pragma warning(push, 0)
#include <llvm/ADT/ArrayRef.h>
#include <llvm/Support/raw_ostream.h>
#pragma warning(pop)
#include "../ast/location.h"

namespace llvm {
class StringRef;
}

#ifndef NDEBUG
#define ASSERT(condition, ...) assert(condition)
#else
// Prevent unused variable warnings without evaluating the condition values.
#define ASSERT(condition, ...) ((void)sizeof(condition))
#endif

namespace cx {

/// Diagnostic options. These mirror the corresponding command-line options
/// defined in the driver, but live here as plain data so that they can also
/// be set by API users (such as the WebAssembly build) that don't link
/// against LLVM's command-line parser.
struct DiagnosticOptions {
    bool disableWarnings = false;
    bool warningsAsErrors = false;
    int errorLimit = 10;
};

extern DiagnosticOptions diagnosticOptions;

/// Number of errors reported so far during the current compilation.
extern int errors;

std::ostream& operator<<(std::ostream& stream, llvm::StringRef string);

template<typename SourceContainer, typename Mapper> auto map(const SourceContainer& source, Mapper mapper) -> std::vector<decltype(mapper(*source.begin()))> {
    std::vector<decltype(mapper(*source.begin()))> result;
    result.reserve(source.size());
    for (auto& element : source) {
        result.emplace_back(mapper(element));
    }
    return result;
}

#define NOTNULL(x) (ASSERT(x), x)

struct StringBuilder : llvm::raw_string_ostream {
    StringBuilder() : llvm::raw_string_ostream(string) {}
    operator llvm::StringRef() const { return string; } // NOLINT(*-explicit-constructor)
    operator const char*() const { return string.c_str(); } // NOLINT(*-explicit-constructor)

    std::string string;
};

std::string readLineFromFile(Location location);
void renameFile(llvm::Twine sourcePath, llvm::Twine targetPath);
void printDiagnostic(Location location, llvm::StringRef type, llvm::raw_ostream::Colors color, llvm::StringRef message, Location endLocation = {});

struct Note {
    Location location;
    std::string message;
};

/// A single diagnostic collected during compilation.
///
/// When `diagnosticCollector` is non-null, `reportError()` and
/// `reportWarning()` append to it instead of printing to stdout. Instead of
/// exiting the process when the error limit is exceeded, they unwind with a
/// silent `CompileError` so the caller still gets back every diagnostic
/// collected so far. This is used by single-shot compiler invocations that
/// report structured diagnostics (currently the language server's one-shot
/// query processes), each of which runs the frontend exactly once in a fresh
/// process and then exits.
struct CollectedDiagnostic {
    Location location;
    std::string severity; // "error" or "warning".
    std::string message;
    std::vector<Note> notes;
};

/// When non-null, diagnostics are collected here instead of being printed.
/// Owned by the caller. Only used in fresh single-shot processes that run one
/// compilation and exit; never reused across compilations. Not thread-safe.
extern std::vector<CollectedDiagnostic>* diagnosticCollector;

struct CompileError : std::exception {
    CompileError(Location location, std::string&& message, std::vector<Note>&& notes = {}, Location endLocation = {});
    /// Creates a specific type of compile error used for propagating non-reported errors that are the result of a previous, reported error.
    /// For example using a variable whose initializer has an error.
    static CompileError dependentError() { return CompileError(Location(), ""); }
    const char* what() const noexcept override { return message.c_str(); }
    void report() const;
    void reportAsWarning() const;

    Location location;
    std::string message; // Empty if this is a silent "dependent error", resulting from a previous, reported error.
    std::vector<Note> notes;
    Location endLocation; // One past the last underlined character. Invalid renders a point instead of a range.
};

template<typename T> void printColored(const T& text, llvm::raw_ostream::Colors color) {
    if (llvm::errs().has_colors()) llvm::errs().changeColor(color, true);
    llvm::errs() << text;
    if (llvm::errs().has_colors()) llvm::errs().resetColor();
}

void printStackTrace();
[[noreturn]] void abort(llvm::StringRef message);
void reportError(Location location, llvm::StringRef message, llvm::ArrayRef<Note> notes = {}, Location endLocation = {});
void reportWarning(Location location, llvm::StringRef message, llvm::ArrayRef<Note> notes = {}, Location endLocation = {});

#define ABORT(args) \
    { \
        printStackTrace(); \
        abort(StringBuilder() << args); \
    }

#define ERROR(location, args) \
    { \
        printStackTrace(); \
        throw CompileError(location, std::move((StringBuilder() << args).string)); \
    }

#define ERROR_WITH_NOTES(location, notes, args) \
    { \
        printStackTrace(); \
        throw CompileError(location, std::move((StringBuilder() << args).string), notes); \
    }

#define REPORT_ERROR(location, args) \
    { \
        printStackTrace(); \
        reportError(location, StringBuilder() << args); \
    }

#define REPORT_ERROR_WITH_NOTES(location, notes, args) \
    { \
        printStackTrace(); \
        reportError(location, StringBuilder() << args, notes); \
    }

#define WARN(location, args) \
    { \
        reportWarning(location, StringBuilder() << args); \
    }

#define ERROR_RANGE(begin, end, args) \
    { \
        printStackTrace(); \
        throw CompileError(begin, std::move((StringBuilder() << args).string), {}, end); \
    }

#define WARN_RANGE(begin, end, args) \
    { \
        reportWarning(begin, StringBuilder() << args, {}, end); \
    }

std::optional<std::string> findExternalCCompiler();

/// Locates the directory containing the `std/` standard-library directory.
/// `CX_ROOT` wins, then directories relative to the running executable are
/// probed (installed `<prefix>/bin` + `<prefix>/share/cx` layouts and in-tree
/// build dirs), with the compile-time source directory as a dev fallback.
std::string getCxRootDir();

} // namespace cx
