#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>
#pragma warning(push, 0)
#include <llvm/Support/CodeGen.h>
#pragma warning(pop)

namespace llvm {
class LLVMContext;
class Module;
template<typename T> class ArrayRef;
} // namespace llvm

namespace cx {

// True when the linked module can run in-process via LLVM's JIT instead of link-and-exec:
// it defines a main with a supported signature and every external it references resolves
// in this process. Anything else (missing main, unloadable libraries, missing runtime calls)
// falls back to the link-and-exec path.
bool jitEligible(const llvm::Module& linkedModule);

// Loads -l/-framework link dependencies into this process so a run/test invocation
// can JIT instead of linking. Returns false when any dependency cannot be provided
// in-process (static archives and object files, missing files, load failures);
// the caller then falls back to link-and-exec.
bool loadLinkLibrariesForJit(llvm::ArrayRef<std::string> librarySearchPaths, llvm::ArrayRef<std::string> libraries,
                             llvm::ArrayRef<std::string> frameworkSearchPaths, llvm::ArrayRef<std::string> frameworks);

// Runs main from the linked module in-process and returns its exit code.
// The codegen optimization level mirrors emit-obj (None in debug builds,
// Aggressive with --release); ORC's default would run the optimizing
// SelectionDAG pipeline even for debug builds, several times slower.
int jitRun(std::unique_ptr<llvm::Module> linkedModule, std::unique_ptr<llvm::LLVMContext> ctx, const std::string& argv0,
           const std::vector<std::string>& programArgs, llvm::CodeGenOptLevel optLevel);

} // namespace cx
