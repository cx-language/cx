#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace llvm {
class LLVMContext;
class Module;
} // namespace llvm

namespace cx {

// True when the linked module can run in-process via LLVM's JIT instead of link-and-exec:
// it defines a main with a supported signature and every external it references resolves
// in this process. Anything else (missing main, -l/-framework libraries, missing runtime calls,
// Windows hosts) falls back to the link-and-exec path.
bool jitEligible(const llvm::Module& linkedModule);

// Runs main from the linked module in-process and returns its exit code.
int jitRun(std::unique_ptr<llvm::Module> linkedModule, std::unique_ptr<llvm::LLVMContext> ctx, const std::string& argv0,
           const std::vector<std::string>& programArgs);

} // namespace cx
