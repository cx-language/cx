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

        // Build files are config, not source, so never compile them. Match by file name:
        // full paths can't be compared reliably across separator styles (Windows).
        if (llvm::sys::path::extension(it->path()) == ".cx" && llvm::sys::path::filename(it->path()) != BuildConfig::buildFileName) {
            sourceFiles.push_back(it->path());
        }
    }

    return sourceFiles;
}
