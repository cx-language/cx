#pragma once

#include <string>
#include <utility>
#include <vector>
#pragma warning(push, 0)
#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/StringRef.h>
#pragma warning(pop)
#include "../driver/driver.h"

namespace cx {

struct BuildConfig {
    struct Dependency {
        std::string getFileSystemPath() const;

        std::string package;
        std::string url;
        std::string version;
    };

    // One dependency with its build file applied: package-scoped settings
    // composed into options, link contributions absolutized. Only the main
    // project's name, outputDirectory, and multitarget take effect.
    struct ResolvedDependency {
        std::string package;
        std::string version; // Empty for vendored packages.
        std::string rootDirectory; // Fetched path or vendor/<package>.
        std::string requiredBy; // Declarer for conflict errors.
        CompileOptions options; // defines, search paths, and cflags.
        std::vector<std::string> pkgConfigDependencies; // Routed by the driver.
        std::vector<std::string> librarySearchPaths;
        std::vector<std::string> libraries;
        std::vector<std::string> frameworks;
        std::vector<std::string> frameworkSearchPaths;
    };

    BuildConfig(std::string&& rootDirectory, std::vector<std::string> defines = {});
    std::vector<std::string> getTargetRootDirectories() const;
    static const char buildFileName[];

    std::string rootDirectory;
    std::string name;
    std::vector<Dependency> declaredDependencies;
    std::vector<ResolvedDependency> resolvedDependencies; // Transitive closure.
    bool multitarget = false;
    std::string outputDirectory = ".";
    std::vector<std::string> defines;
    std::vector<std::string> headerSearchPaths;
    std::vector<std::string> librarySearchPaths;
    std::vector<std::string> libraries;
    std::vector<std::string> frameworks;
    std::vector<std::string> pkgConfigDependencies;
    std::vector<std::string> warnings;
};

// A warning tunable through the build.cx `warnings` setting. Names match the
// -W flag suffixes: "conversion", "unused", "unused-result", "undef".
enum class WarningKind { Conversion, Unused, UnusedResult, Undef };

// Bit marking a warning as set on the command line (see applyWarningSettings).
constexpr unsigned warningBit(WarningKind kind) {
    return 1u << static_cast<unsigned>(kind);
}

// Splits a `warnings` entry into its kind and enablement (a "no-" prefix
// disables). Aborts on unknown names.
std::pair<WarningKind, bool> parseWarningSetting(llvm::StringRef entry);

// Applies a build.cx `warnings` list to options, in order (later entries win).
// Entries whose warningBit is set in explicitWarnings are skipped: an explicit
// command-line flag always wins over the setting.
void applyWarningSettings(CompileOptions& options, llvm::ArrayRef<std::string> warnings, unsigned explicitWarnings = 0);

} // namespace cx
