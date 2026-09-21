#include "utility.h"
#include <algorithm>
#include <fstream>
#include <ostream>
#pragma warning(push, 0)
#include <llvm/ADT/SmallSet.h>
#include <llvm/Support/ErrorOr.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/Process.h>
#include <llvm/Support/Program.h>
#ifndef __EMSCRIPTEN__
// Signal handling is not available in the WebAssembly build.
#include <llvm/Support/Signals.h>
#endif
#pragma warning(pop)

using namespace cx;

namespace cx {
DiagnosticOptions diagnosticOptions;
int errors = 0;
std::vector<CollectedDiagnostic>* diagnosticCollector = nullptr;
} // namespace cx

std::ostream& cx::operator<<(std::ostream& stream, llvm::StringRef string) {
    return stream.write(string.data(), string.size());
}

std::string cx::readLineFromFile(Location location) {
    std::ifstream file(location.file);

    while (--location.line) {
        file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    std::string lineContent;
    std::getline(file, lineContent);
    return lineContent;
}

void cx::renameFile(llvm::Twine sourcePath, llvm::Twine targetPath) {
    // Rename atomically so rebuilding over a running executable swaps in a new
    // inode instead of overwriting the running image in place (on macOS that
    // breaks the code signature and every launch of the rebuilt binary dies).
    if (!llvm::sys::fs::rename(sourcePath, targetPath)) return;

    // Renaming across filesystems fails; fall back to replacing the target,
    // removing it first so the copy still lands on a fresh inode.
    auto permissions = llvm::sys::fs::getPermissions(sourcePath);
    if (auto error = permissions.getError()) {
        ABORT("couldn't get permissions for '" << sourcePath << "': " << error.message());
    }
    llvm::sys::fs::remove(targetPath);
    if (auto error = llvm::sys::fs::copy_file(sourcePath, targetPath)) {
        // A bad output path is a user error, not a compiler bug, so report it
        // without a stack trace even when CX_PRINT_STACK_TRACE is set.
        abort(StringBuilder() << "couldn't write output file '" << targetPath << "': " << error.message());
    }
    if (auto error = llvm::sys::fs::setPermissions(targetPath, *permissions)) {
        ABORT("couldn't set permissions for '" << targetPath << "': " << error.message());
    }
    if (auto error = llvm::sys::fs::remove(sourcePath)) {
        ABORT("couldn't remove '" << sourcePath << "': " << error.message());
    }
}

void cx::printDiagnostic(Location location, llvm::StringRef type, llvm::raw_ostream::Colors color, llvm::StringRef message) {
    if (llvm::errs().has_colors()) {
        llvm::errs().changeColor(llvm::raw_ostream::SAVEDCOLOR, true);
    }

    if (location.print()) {
        llvm::errs() << ": ";
    }

    printColored(type, color);
    printColored(": ", color);
    printColored(message, llvm::raw_ostream::SAVEDCOLOR);

    if (location.file && *location.file && location.isValid()) {
        auto line = readLineFromFile(location);
        llvm::errs() << '\n' << line << '\n';

        for (char ch : line.substr(0, location.column - 1)) {
            llvm::errs() << (ch != '\t' ? ' ' : '\t');
        }
        printColored('^', llvm::raw_ostream::GREEN);
    }

    llvm::errs() << '\n';
}

CompileError::CompileError(Location location, std::string&& message, std::vector<Note>&& notes)
: location(location), message(std::move(message)), notes(std::move(notes)) {}

void CompileError::report() const {
    if (message.empty()) return;
    reportError(location, StringBuilder() << message, notes);
}

void CompileError::reportAsWarning() const {
    if (message.empty()) return;
    reportWarning(location, StringBuilder() << message, notes);
}

std::optional<std::string> cx::findExternalCCompiler() {
#ifdef _WIN32
    auto compilers = {"cl.exe", "clang-cl.exe"};
#else
    auto compilers = {"cc", "clang", "gcc"};
#endif
    for (const char* compiler : compilers) {
        if (auto path = llvm::sys::findProgramByName(compiler)) {
            return std::move(*path);
        }
    }
    return std::nullopt;
}

std::string cx::getCxRootDir() {
    if (auto root = llvm::sys::Process::GetEnv("CX_ROOT")) {
        if (!root->empty()) return *root;
    }
#ifndef __EMSCRIPTEN__
    // Anchor on this function's address so dladdr(3) resolves to the running
    // binary (libcx links statically into each executable).
    std::string executable = llvm::sys::fs::getMainExecutable("", reinterpret_cast<void*>(&getCxRootDir));
    if (!executable.empty()) {
        std::string directory = llvm::sys::path::parent_path(executable).str();
        for (int level = 0; level < 3 && !directory.empty(); ++level) {
            const std::string candidates[] = {directory, directory + "/share/cx"};
            for (auto& candidate : candidates) {
                if (llvm::sys::fs::is_directory(candidate + "/std")) return candidate;
            }
            directory = llvm::sys::path::parent_path(directory).str();
        }
    }
#endif
#ifdef CX_ROOT_DIR
    return CX_ROOT_DIR;
#else
    return "";
#endif
}

void cx::printStackTrace() {
#ifndef __EMSCRIPTEN__
    if (auto env = llvm::sys::Process::GetEnv("CX_PRINT_STACK_TRACE")) {
        if (llvm::StringRef(*env).equals_insensitive("true") || *env == "1") {
            llvm::sys::PrintStackTrace(llvm::errs());
        }
    }
#else
    // Stack traces are not supported in the WebAssembly build.
#endif
}

void cx::abort(llvm::StringRef message) {
    if (diagnosticCollector) {
        // Single-shot structured-diagnostics mode (LSP query process):
        // unwind to the API boundary so the failure is reported as a
        // diagnostic instead of killing the process with exit(1).
        throw CompileError(Location(), message.str());
    }
    printColored("error: ", llvm::raw_ostream::RED);
    llvm::errs() << message << '\n';
    exit(1);
}

static void collectDiagnostic(Location location, const char* severity, llvm::StringRef message, llvm::ArrayRef<Note> notes) {
    CollectedDiagnostic diagnostic;
    diagnostic.location = location;
    diagnostic.severity = severity;
    diagnostic.message = message.str();
    diagnostic.notes.assign(notes.begin(), notes.end());
    diagnosticCollector->push_back(std::move(diagnostic));
}

void cx::reportError(Location location, llvm::StringRef message, llvm::ArrayRef<Note> notes) {
    errors++;
    if (diagnosticCollector) {
        collectDiagnostic(location, "error", message, notes);
        if (diagnosticOptions.errorLimit > 0 && errors > diagnosticOptions.errorLimit) {
            throw CompileError::dependentError();
        }
        return;
    }
    if (diagnosticOptions.errorLimit > 0 && errors > diagnosticOptions.errorLimit) {
#ifdef __EMSCRIPTEN__
        // Exiting the process is not an option inside the WebAssembly build,
        // so unwind to the API boundary instead. The error count already
        // ensures the compilation is reported as failed. Throwing a silent
        // dependent error avoids reporting the same error twice, as the
        // regular error handlers ignore errors with empty messages.
        throw CompileError::dependentError();
#else
        exit(1);
#endif
    }

    printDiagnostic(location, "error", llvm::raw_ostream::RED, message);

    for (auto& note : notes) {
        printDiagnostic(note.location, "note", llvm::raw_ostream::BLACK, note.message);
    }
}

struct ReportedWarning {
    std::string file;
    Location::IntegerType line;
    Location::IntegerType column;
    std::string message;

    bool operator==(const ReportedWarning& other) const = default;
    std::strong_ordering operator<=>(const ReportedWarning& other) const = default;
};

static llvm::SmallSet<ReportedWarning, 8> reportedWarnings;

void cx::reportWarning(Location location, llvm::StringRef message, llvm::ArrayRef<Note> notes) {
    if (diagnosticOptions.disableWarnings) return;

    // Overload resolution typechecks call arguments once per candidate; report each unique warning only once.
    ReportedWarning warning{location.file ? location.file : "", location.line, location.column, message.str()};
    if (!reportedWarnings.insert(warning).second) {
        return;
    }

    if (diagnosticOptions.warningsAsErrors) {
        reportError(location, message, notes);
    } else {
        if (diagnosticCollector) {
            collectDiagnostic(location, "warning", message, notes);
            return;
        }
        printDiagnostic(location, "warning", llvm::raw_ostream::YELLOW, message);

        for (auto& note : notes) {
            printDiagnostic(note.location, "note", llvm::raw_ostream::BLACK, note.message);
        }
    }
}
