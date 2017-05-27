#include <cstdlib>
#include <yaml-cpp/yaml.h>
#include "manifest.h"
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

PackageManifest::PackageManifest(const std::string& packageRoot) {
    YAML::Node root;
    auto manifestPath = packageRoot + "/" + manifestFileName;

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
}
