#include "utility.h"
#include <algorithm>
#include <fstream>
#include <ostream>
#ifdef __APPLE__
#include <limits.h>
#include <unistd.h>
#endif
#pragma warning(push, 0)
#include <llvm/ADT/SmallSet.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringExtras.h>
#include <llvm/Support/ErrorOr.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/Process.h>
#include <llvm/Support/Program.h>
#include <llvm/Support/VersionTuple.h>
#ifndef __EMSCRIPTEN__
// Signal handling is not available in the WebAssembly build.
#include <llvm/Support/Signals.h>
#endif
#pragma warning(pop)

#ifdef _MSC_VER
#define popen _popen
#define pclose _pclose
#define WEXITSTATUS(x) x
#endif

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

void cx::printDiagnostic(Location location, llvm::StringRef type, llvm::raw_ostream::Colors color, llvm::StringRef message, Location endLocation) {
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

        // Underline the source range when the end is past the start on the
        // same line, or to the end of the line for multiline ranges.
        int endColumn = -1;
        if (endLocation.isValid() && endLocation.line >= location.line && (endLocation.line > location.line || endLocation.column > location.column)) {
            endColumn = endLocation.line == location.line ? std::min<int>(endLocation.column, int(line.size()) + 1) : int(line.size()) + 1;
        }
        if (endColumn > location.column) {
            for (int i = location.column; i < endColumn; i++) {
                printColored('~', llvm::raw_ostream::GREEN);
            }
        } else {
            printColored('^', llvm::raw_ostream::GREEN);
        }
    }

    llvm::errs() << '\n';
}

CompileError::CompileError(Location location, std::string&& message, std::vector<Note>&& notes, Location endLocation)
: location(location), message(std::move(message)), notes(std::move(notes)), endLocation(endLocation) {}

void CompileError::report() const {
    if (message.empty()) return;
    reportError(location, StringBuilder() << message, notes, endLocation);
}

void CompileError::reportAsWarning() const {
    if (message.empty()) return;
    reportWarning(location, StringBuilder() << message, notes, endLocation);
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

int cx::exec(const char* command, std::string& output) {
    FILE* pipe = popen(command, "r");
    if (!pipe) {
        ABORT("failed to execute '" << command << "'");
    }

    try {
        char buffer[128];
        while (fgets(buffer, sizeof buffer, pipe)) {
            output += buffer;
        }
    } catch (...) {
        pclose(pipe);
        throw;
    }

    int status = pclose(pipe);
    return WEXITSTATUS(status);
}

const std::vector<std::string>& cx::getCCompilerSearchPaths() {
    static std::vector<std::string> paths;
    static bool queried = false;
    if (!queried) {
        queried = true;
#ifdef __EMSCRIPTEN__
        // No host C toolchain inside the cx-wasm module (it lives in a
        // separate module), so there is nothing to query; callers correctly
        // treat the empty list as "no system headers".
        return paths;
#else
        PhaseTimer timer("c-search-paths");
        auto cCompilerPath = findExternalCCompiler();
#ifdef __APPLE__
        // macOS ships no /usr/include, so the SDK provides all system headers and the
        // query below would fire for nearly every build. The SDK location follows from
        // the selected developer directory (explicit xcode-select choice first, then
        // xcode-select's own fallback order), probed with stat instead of a ~25ms
        // compiler spawn, along with the toolchain's clang resource directory (home of
        // compiler builtin headers like stdarg.h). Only SDK/usr/include matters in
        // practice alongside it: the toolchain's other dirs carry Swift/lexer leftovers
        // that would only shadow our matching LLVM builtin headers. The SDK and the
        // resource dir must come from the same developer directory, and the resolved
        // compiler must be Apple's own; anything else falls back to spawning below.
        std::string developerDirs[3];
        char selectedDir[PATH_MAX];
        ssize_t selectedLen = readlink("/var/db/xcode_select_link", selectedDir, sizeof(selectedDir) - 1);
        if (selectedLen > 0) {
            selectedDir[selectedLen] = '\0';
            developerDirs[0] = selectedDir;
        }
        developerDirs[1] = "/Applications/Xcode.app/Contents/Developer";
        developerDirs[2] = "/Library/Developer/CommandLineTools";
        bool useStaticPaths = !cCompilerPath || cCompilerPath->starts_with("/usr/bin/");
        for (auto& developerDir : developerDirs) {
            if (!developerDir.empty() && cCompilerPath && cCompilerPath->starts_with(developerDir)) useStaticPaths = true;
        }
        if (useStaticPaths) {
            for (auto& developerDir : developerDirs) {
                if (developerDir.empty()) continue;
                const char* sdkSubpath = nullptr;
                for (auto* candidate : {"/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk", "/SDKs/MacOSX.sdk"}) {
                    if (llvm::sys::fs::is_directory(developerDir + candidate + "/usr/include")) {
                        sdkSubpath = candidate;
                        break;
                    }
                }
                // Old clang versions linger next to the current one; the newest
                // matches what `cc` reports (dotted entries symlink to it).
                std::string clangIncludeDir;
                llvm::VersionTuple newestClangVersion;
                for (auto* clangSubpath : {"/Toolchains/XcodeDefault.xctoolchain/usr/lib/clang", "/usr/lib/clang"}) {
                    std::error_code error;
                    llvm::sys::fs::directory_iterator end;
                    for (llvm::sys::fs::directory_iterator it(developerDir + clangSubpath, error); !error && it != end; it.increment(error)) {
                        llvm::VersionTuple version;
                        // tryParse returns true on failure; keep the newest seen.
                        if (version.tryParse(llvm::sys::path::filename(it->path())) || version <= newestClangVersion) continue;
                        std::string includeDir = it->path() + "/include";
                        if (llvm::sys::fs::is_directory(includeDir)) {
                            newestClangVersion = version;
                            clangIncludeDir = includeDir;
                        }
                    }
                }
                if (!sdkSubpath || clangIncludeDir.empty()) continue;
                paths.push_back(clangIncludeDir);
                paths.push_back(developerDir + sdkSubpath + "/usr/include");
                std::string frameworksDir = developerDir + sdkSubpath + "/System/Library/Frameworks";
                if (llvm::sys::fs::is_directory(frameworksDir)) paths.push_back(frameworksDir);
                return paths;
            }
        }
#endif
        // MSVC-family drivers don't print GNU-style search paths for -E -v,
        // and stock Windows has no grep for the pipeline below (system headers
        // come from INCLUDE instead), so skip the probe for them.
        llvm::StringRef compilerName = cCompilerPath ? llvm::sys::path::filename(*cCompilerPath) : "";
        if (!compilerName.empty() && compilerName != "cl.exe" && compilerName != "clang-cl.exe") {
            std::string command = "echo | " + *cCompilerPath + " -E -v - 2>&1 | grep '^ /'";
            std::string output;
            exec(command.c_str(), output);
            llvm::SmallVector<llvm::StringRef, 8> lines;
            llvm::SplitString(output, lines, "\n");
            for (auto line : lines) {
                auto path = line.trim();
                if (llvm::sys::fs::is_directory(path)) {
                    paths.push_back(path.str());
                }
            }
        }
#endif
    }
    return paths;
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

void cx::reportError(Location location, llvm::StringRef message, llvm::ArrayRef<Note> notes, Location endLocation) {
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

    printDiagnostic(location, "error", llvm::raw_ostream::RED, message, endLocation);

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

void cx::reportWarning(Location location, llvm::StringRef message, llvm::ArrayRef<Note> notes, Location endLocation) {
    if (diagnosticOptions.disableWarnings) return;

    // Overload resolution typechecks call arguments once per candidate; report each unique warning only once.
    ReportedWarning warning{location.file ? location.file : "", location.line, location.column, message.str()};
    if (!reportedWarnings.insert(warning).second) {
        return;
    }

    if (diagnosticOptions.warningsAsErrors) {
        reportError(location, message, notes, endLocation);
    } else {
        if (diagnosticCollector) {
            collectDiagnostic(location, "warning", message, notes);
            return;
        }
        printDiagnostic(location, "warning", llvm::raw_ostream::YELLOW, message, endLocation);

        for (auto& note : notes) {
            printDiagnostic(note.location, "note", llvm::raw_ostream::BLACK, note.message);
        }
    }
}
