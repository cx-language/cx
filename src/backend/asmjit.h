#pragma once

#include <string>
#include <vector>

namespace cx {

struct IRModule;

// In-process JIT via AsmJit for `cx run`: compiles cx IR straight to machine
// code in memory, skipping LLVM IR building, optimization, and object emission.
// AArch64 hosts use the emitter in asmjit.cpp, 64-bit Windows the x64 emitter
// in asmjit_x64.h; other hosts keep the LLVM JIT.
struct AsmJitSession {
    // True when the session can run these modules: host is AArch64 or 64-bit
    // Windows, a main with 0 or 2 params is defined, and every extern
    // resolves in-process.
    static bool eligible(const std::vector<IRModule*>& modules);
    // Compiles all modules and runs main. Returns the program's exit code.
    static int run(const std::vector<IRModule*>& modules, const std::string& argv0, const std::vector<std::string>& programArgs);
};

} // namespace cx
