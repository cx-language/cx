#pragma once

#include <string>
#include <vector>

namespace cx {

struct IRModule;

// In-process JIT via AsmJit for `cx run` on 64-bit Windows: compiles cx IR
// straight to x64 machine code in memory, skipping LLVM IR building,
// optimization, and object emission. Implements the Win64 calling
// convention (single-integer chunks for small aggregates, sret/by-pointer
// above 8 bytes); other x64 targets keep the LLVM JIT. Mirrors the
// AArch64 backend in asmjit.cpp; see asmjit_x64.cpp.
struct X64AsmJitSession {
    // True when the session can run these modules: host is 64-bit Windows,
    // a main with 0 or 2 params is defined, no float80 crosses an extern
    // boundary, and every extern resolves in-process.
    static bool eligible(const std::vector<IRModule*>& modules);
    // Compiles all modules and runs main. Returns the program's exit code.
    static int run(const std::vector<IRModule*>& modules, const std::string& argv0, const std::vector<std::string>& programArgs);
};

} // namespace cx
