#pragma once

#include <string>
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
};

} // namespace cx
