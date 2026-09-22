#include "dependencies.h"
#include <string>
#include <system_error>
#include <vector>
#pragma warning(push, 0)
#include <llvm/ADT/StringRef.h>
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

void cx::fetchDependencies(const BuildConfig& config) {
    for (auto& dependency : config.declaredDependencies) {
        auto path = dependency.getFileSystemPath();

        if (!llvm::sys::fs::exists(path)) {
            cloneGitRepository(dependency.url, path);
            checkoutGitRevision(path, dependency.version);
        }
    }
}

std::vector<std::string> cx::getSourceFiles(llvm::StringRef rootDirectory) {
    std::vector<std::string> sourceFiles;
    std::error_code error;

    for (llvm::sys::fs::recursive_directory_iterator it(rootDirectory, error), end; it != end; it.increment(error)) {
        if (error) {
            llvm::errs() << error.message() << '\n';
            break;
        }

        // The root build.cx is config, not source; anything else compiles,
        // including a build.cx below the root (see isRootBuildFile).
        if (llvm::sys::path::extension(it->path()) == ".cx" && !isVendoredPath(it->path()) && !isRootBuildFile(it->path(), rootDirectory)) {
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
