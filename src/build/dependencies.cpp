#include "dependencies.h"
#include <string>
#include <system_error>
#include <vector>
#pragma warning(push, 0)
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/StringSet.h>
#include <llvm/Support/ErrorOr.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/Program.h>
#include <llvm/Support/raw_ostream.h>
#pragma warning(pop)
#include "../support/utility.h"
#include "config.h"

using namespace cx;

static std::string getGitPath() {
    llvm::ErrorOr<std::string> gitPath = llvm::sys::findProgramByName("git");

    if (!gitPath) {
        ABORT("couldn't find Git");
    }

    return *gitPath;
}

static void cloneGitRepository(const std::string& repositoryUrl, const std::string& path) {
    auto gitPath = getGitPath();
    llvm::StringRef args[] = {gitPath, "clone", repositoryUrl, path};

    std::string error;
    int status = llvm::sys::ExecuteAndWait(gitPath, args, std::nullopt, {}, 0, 0, &error);

    if (status != 0 || !error.empty()) {
        if (!error.empty()) error.insert(0, ": ");
        ABORT("'git clone " << repositoryUrl << " " << path << "' failed with exit status " << status << error);
    }
}

static void checkoutGitRevision(llvm::StringRef path, llvm::StringRef revision) {
    auto gitPath = getGitPath();
    auto gitDir = ("--git-dir=" + path + "/.git").str();
    auto workTree = ("--work-tree=" + path).str();
    llvm::StringRef args[] = {gitPath, gitDir, workTree, "checkout", revision, "--quiet"};

    std::string error;
    int status = llvm::sys::ExecuteAndWait(gitPath, args, std::nullopt, {}, 0, 0, &error);

    if (status != 0 || !error.empty()) {
        if (!error.empty()) error.insert(0, ": ");
        ABORT("'git checkout " << revision << "' failed with exit status " << status << error);
    }
}

std::string cx::absolutizePackagePath(llvm::StringRef rootDirectory, const std::string& path) {
    if (llvm::sys::path::is_absolute(path)) return path;
    return (rootDirectory + "/" + path).str();
}

std::string cx::absolutizeLibraryPath(llvm::StringRef rootDirectory, const std::string& library) {
    if (llvm::sys::path::is_absolute(library)) return library;
    if (library.find('/') == std::string::npos && library.find('\\') == std::string::npos) {
        std::string candidate = (rootDirectory + "/" + library).str();
        if (!llvm::sys::fs::exists(candidate)) return library;
        return candidate;
    }
    return absolutizePackagePath(rootDirectory, library);
}

void cx::resolveDependencyClosure(BuildConfig& config, const CompileOptions& baseOptions, bool fetchMissing) {
    struct WorkItem {
        bool isVendored = false;
        BuildConfig::Dependency dependency; // Set for git dependencies.
        std::string rootDirectory; // Set for vendored packages.
        std::string package; // Set for vendored packages.
        std::string requiredBy;
    };

    std::vector<WorkItem> worklist;
    for (auto& dependency : config.declaredDependencies) {
        worklist.push_back({false, dependency, "", "", "the project build file"});
    }

    // Vendored packages: each top-level vendor/ subdirectory is a package.
    // Deeper vendor/ directories compile with their package (no nested packages).
    {
        std::error_code error;
        std::vector<std::string> packageNames;
        llvm::sys::fs::directory_iterator it(config.rootDirectory + "/vendor", error), end;
        for (; it != end && !error; it.increment(error)) {
            if (llvm::sys::fs::is_directory(it->path())) {
                packageNames.push_back(llvm::sys::path::filename(it->path()).str());
            }
        }
        llvm::sort(packageNames);
        for (auto& name : packageNames) {
            worklist.push_back({true, {}, config.rootDirectory + "/vendor/" + name, name, "the vendor directory"});
        }
    }

    llvm::StringSet<> visited;
    visited.insert(config.rootDirectory);

    while (!worklist.empty()) {
        WorkItem item = std::move(worklist.back());
        worklist.pop_back();

        std::string root = item.isVendored ? item.rootDirectory : item.dependency.getFileSystemPath();
        std::string package = item.isVendored ? item.package : item.dependency.package;
        std::string version = item.isVendored ? "" : item.dependency.version;

        if (!item.isVendored) {
            if (!llvm::sys::fs::exists(root)) {
                if (!fetchMissing) continue;
                cloneGitRepository(item.dependency.url, root);
                checkoutGitRevision(root, item.dependency.version);
            }
        } else if (!llvm::sys::fs::is_directory(root)) {
            continue;
        }
        if (!visited.insert(root).second) continue;

        // A missing build file means defaults: the sources are still importable.
        BuildConfig depConfig(std::string(root), baseOptions.defines);
        std::string declarer = item.isVendored ? ("vendored package '" + package + "'") : (package + "@" + version);
        for (auto& transitive : depConfig.declaredDependencies) {
            worklist.push_back({false, transitive, "", "", declarer});
        }

        BuildConfig::ResolvedDependency record;
        record.package = std::move(package);
        record.version = std::move(version);
        record.rootDirectory = root;
        record.requiredBy = item.requiredBy;
        record.options = baseOptions;
        // Every package sees the project's shared vendor/ directory.
        record.options.importSearchPaths.push_back(config.rootDirectory + "/vendor");
        for (auto& define : depConfig.defines) {
            record.options.defines.push_back(define);
        }
        for (auto& path : depConfig.headerSearchPaths) {
            record.options.importSearchPaths.push_back(absolutizePackagePath(root, path));
        }
        record.pkgConfigDependencies = depConfig.pkgConfigDependencies;
        for (auto& path : depConfig.librarySearchPaths) {
            record.librarySearchPaths.push_back(absolutizePackagePath(root, path));
        }
        for (auto& library : depConfig.libraries) {
            record.libraries.push_back(absolutizeLibraryPath(root, library));
        }
        record.frameworks = depConfig.frameworks;
        config.resolvedDependencies.push_back(std::move(record));
    }
}

DependencyResolution cx::resolveDependency(const std::vector<BuildConfig::ResolvedDependency>& closure, llvm::StringRef packageName) {
    const BuildConfig::ResolvedDependency* found = nullptr;
    for (auto& record : closure) {
        if (record.package != packageName) continue;
        if (!found) {
            found = &record;
            continue;
        }
        if (found->rootDirectory == record.rootDirectory) continue; // Repeated declaration.
        return {nullptr, true,
                "package '" + packageName.str() + "' is provided by both '" + found->rootDirectory + "' (required by " + found->requiredBy + ") and '"
                    + record.rootDirectory + "' (required by " + record.requiredBy + ")"};
    }
    return {found, false, ""};
}

std::vector<std::string> cx::getSourceFiles(llvm::StringRef targetRoot, llvm::StringRef projectRoot) {
    std::vector<std::string> sourceFiles;
    std::error_code error;

    for (llvm::sys::fs::recursive_directory_iterator it(targetRoot, error), end; it != end; it.increment(error)) {
        if (error) {
            llvm::errs() << error.message() << '\n';
            break;
        }

        // Only the project root's build.cx is config, not source; anything
        // else compiles, including a build.cx below the project root
        // (see isRootBuildFile).
        if (llvm::sys::path::extension(it->path()) == ".cx" && !isVendoredPath(it->path()) && !isRootBuildFile(it->path(), projectRoot)) {
            sourceFiles.push_back(it->path());
        }
    }

    return sourceFiles;
}

bool cx::isVendoredPath(llvm::StringRef path) {
    for (auto it = llvm::sys::path::begin(path), end = llvm::sys::path::end(path); it != end; ++it) {
        if (*it == "vendor") return true;
    }
    return false;
}

bool cx::isRootBuildFile(llvm::StringRef path, llvm::StringRef rootDirectory) {
    if (llvm::sys::path::filename(path) != BuildConfig::buildFileName) return false;
    // Filesystem identity, not string comparison: spellings differ across
    // separator styles (Windows) and relative forms. Unresolvable paths
    // compile by default; only the known config is skipped.
    return llvm::sys::fs::equivalent(llvm::sys::path::parent_path(path), rootDirectory);
}
