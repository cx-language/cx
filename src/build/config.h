#pragma once

#include <string>
#include <vector>
#pragma warning(push, 0)
#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/StringRef.h>
#pragma warning(pop)

namespace cx {

struct BuildConfig {
    struct Dependency {
        std::string getFileSystemPath() const;

        std::string package;
        std::string url;
        std::string version;
    };

    BuildConfig(std::string&& rootDirectory, std::vector<std::string> defines = {});
    std::vector<std::string> getTargetRootDirectories() const;
    static const char buildFileName[];

    std::string rootDirectory;
    std::string name;
    std::vector<Dependency> declaredDependencies;
    bool multitarget = false;
    std::string outputDirectory = ".";
    std::vector<std::string> defines;
    std::vector<std::string> libraries;
    std::vector<std::string> frameworks;
    std::vector<std::string> pkgConfigDependencies;
};

} // namespace cx
