#include "manifest.h"
#include <cstdlib>
#include <yaml-cpp/yaml.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>
#include "../support/utility.h"

using namespace delta;

const char PackageManifest::manifestFileName[] = "package.yml";

std::string PackageManifest::Dependency::getGitRepositoryUrl() const {
    return "https://github.com/" + packageIdentifier + ".git";
}

std::string PackageManifest::Dependency::getFileSystemPath() const {
    const char* home = std::getenv("HOME");
    if (!home) {
        printErrorAndExit("environment variable HOME not set");
    }
    return std::string(home) + "/.delta/dependencies/" + packageIdentifier + "@" + packageVersion;
}

PackageManifest::PackageManifest(std::string&& packageRoot) : packageRoot(std::move(packageRoot)) {
    YAML::Node root;
    auto manifestPath = this->packageRoot + "/" + manifestFileName;

    try {
        root = YAML::LoadFile(manifestPath);
    } catch (const YAML::BadFile&) {
        printErrorAndExit("couldn't open file '", manifestPath, "'");
    }

    if (auto name = root["name"]) {
        packageName = name.as<std::string>();
    }

    for (auto dependency : root["dependencies"]) {
        declaredDependencies.emplace_back(dependency.first.as<std::string>(), dependency.second.as<std::string>());
    }

    multitarget = root["multitarget"].as<bool>(false);
    outputDirectory = root["outputDirectory"].as<std::string>("bin");
}

std::vector<std::string> PackageManifest::getTargetRootDirectories() const {
    if (!isMultiTarget()) return { packageRoot };

    std::string sourceDir = packageRoot;
    std::error_code error;

    for (llvm::sys::fs::directory_iterator it(packageRoot, error), end; it != end; it.increment(error)) {
        if (!llvm::sys::fs::is_directory(it->path())) continue;
        llvm::StringRef dir = llvm::sys::path::filename(it->path());
        if (dir.equals_lower("src") || dir.equals_lower("source") || dir.equals_lower("sources")) {
            sourceDir += llvm::sys::path::get_separator();
            sourceDir += dir;
            break;
        }
    }

    std::vector<std::string> targetDirs;

    for (llvm::sys::fs::directory_iterator it(sourceDir, error), end; it != end; it.increment(error)) {
        if (llvm::sys::fs::is_directory(it->path())) {
            targetDirs.push_back(it->path());
        }
    }

    return targetDirs;
}
