#pragma once

#include <string>
#include <vector>

namespace cx {

/// Options for compileToC(). Unlike the driver, which reads these from
/// command-line flags and the environment, API users pass them explicitly so
/// that no process-global state is required.
struct CompileToCOptions {
    /// Directories that are searched for imported modules (in particular the
    /// directory containing the standard library's "std" subdirectory).
    std::vector<std::string> importSearchPaths;
    bool noUnusedWarnings = false;
    /// Generate goto-free C code using dispatch loops. Required for C
    /// compilers without goto support (e.g. the web playground toolchain).
    bool dispatchMode = false;
};

struct CompileToCResult {
    /// 0 on success, 1 if compilation failed with (already reported) errors.
    int status = 0;
    /// The generated C code. Only valid if status == 0.
    std::string cCode;
};

/// Compiles cx source code held in memory to C code.
///
/// Like the command-line compiler, this is designed to run once per process:
/// global state (error count, type caches, imported modules) is intentionally
/// never cleaned up. Callers that compile repeatedly (such as the
/// WebAssembly build backing the online playground) must start fresh state
/// for each compilation instead - for WebAssembly that means instantiating a
/// new module, which gives a pristine address space just like a new process.
///
/// Diagnostics are written to llvm::errs() (which the WebAssembly build
/// captures), and the generated C code is returned. No files are written and
/// no subprocesses are spawned.
///
/// C header imports ("import \"foo.h\"") require the Clang-based importer.
/// Builds compiled with CX_NO_C_IMPORT reject them with an error.
CompileToCResult compileToC(const char* fileName, const char* source, const CompileToCOptions& options);

} // namespace cx
