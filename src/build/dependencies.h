#pragma once

#include <optional>
#include <string>
#include <vector>
#pragma warning(push, 0)
#include <llvm/ADT/ArrayRef.h>
#pragma warning(pop)
#include "../driver/driver.h"
#include "config.h"

namespace llvm {
class StringRef;
}

namespace cx {

// Reads every dependency's build file into config.resolvedDependencies, fetching
// git dependencies first when fetchMissing (the language server never fetches).
// baseOptions carries invocation-global flags (no project settings); each record
// composes them with its package's settings. Must run before parsing anything,
// since defines select #if branches.
void resolveDependencyClosure(BuildConfig& config, const CompileOptions& baseOptions, bool fetchMissing);
// Splits `pkg-config --cflags --libs` output into package-scoped compile parts
// and link parts.
struct PkgConfigSplit {
    std::vector<std::string> defines;
    std::vector<std::string> headerSearchPaths;
    std::vector<std::string> frameworkSearchPaths;
    std::vector<std::string> cflags;
    std::vector<std::string> librarySearchPaths;
    std::vector<std::string> libraries;
    std::vector<std::string> frameworks;
};
// Queries `pkg-config --cflags --libs` for the given packages, splitting the
// output. Returns nullopt when pkg-config is missing or the query fails; the
// driver aborts, the language server degrades to no pkg-config paths.
std::optional<PkgConfigSplit> queryPkgConfigFlags(llvm::ArrayRef<std::string> packages);
struct DependencyResolution {
    const BuildConfig::ResolvedDependency* dependency = nullptr;
    bool ambiguous = false; // Several distinct sources provide the package.
    std::string ambiguityDetail; // Set when ambiguous, for the diagnostic.
};
// Finds the closure record providing packageName. Exact duplicates (same root
// from repeated declarations) collapse; distinct roots report ambiguous.
DependencyResolution resolveDependency(const std::vector<BuildConfig::ResolvedDependency>& closure, llvm::StringRef packageName);
// Joins a package-relative path onto its root; absolute paths pass through.
std::string absolutizePackagePath(llvm::StringRef rootDirectory, const std::string& path);
// Joins a package-relative library onto its root, leaving bare -l names (e.g.
// "GL") for the linker; a bare name that names a file in the package still
// resolves against it.
std::string absolutizeLibraryPath(llvm::StringRef rootDirectory, const std::string& library);
// All .cx files under targetRoot, except the project root's build.cx.
// Only the project root's build file is config; a build.cx anywhere else
// (including a multitarget target root) compiles as an ordinary source.
std::vector<std::string> getSourceFiles(llvm::StringRef targetRoot, llvm::StringRef projectRoot);
// True when path lies under a 'vendor' directory. Vendored packages are
// imported by name, never compiled as part of the importing module.
bool isVendoredPath(llvm::StringRef path);
// True when path is rootDirectory's build.cx, the one reserved build file.
// A build.cx anywhere else is an ordinary source file and compiles as usual.
bool isRootBuildFile(llvm::StringRef path, llvm::StringRef rootDirectory);

} // namespace cx
