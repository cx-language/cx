#include "package-manager.h"
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
#include "manifest.h"
#include "../support/utility.h"

using namespace delta;

static std::string getGitPath() {
    llvm::ErrorOr<std::string> gitPath = llvm::sys::findProgramByName("git");

    if (!gitPath) {
        printErrorAndExit("couldn't find Git");
    }

    return *gitPath;
}

static void cloneGitRepository(const std::string& repositoryUrl, const std::string& path) {
    auto gitPath = getGitPath();
    llvm::StringRef args[] = { gitPath, "clone", repositoryUrl, path };

    std::string error;
    int status = llvm::sys::ExecuteAndWait(gitPath, args, llvm::None, {}, 0, 0, &error);

    if (status != 0 || !error.empty()) {
        if (!error.empty()) error.insert(0, ": ");
        printErrorAndExit("'git clone ", repositoryUrl, " ", path, "' failed with exit status ", status, error);
    }
}

static void checkoutGitRevision(const std::string& path, const std::string& revision) {
    auto gitPath = getGitPath();
    auto gitDir = "--git-dir=" + path + "/.git";
    auto workTree = "--work-tree=" + path;
    llvm::StringRef args[] = { gitPath, gitDir, workTree, "checkout", revision, "--quiet" };

    std::string error;
    int status = llvm::sys::ExecuteAndWait(gitPath, args, llvm::None, {}, 0, 0, &error);

    if (status != 0 || !error.empty()) {
        if (!error.empty()) error.insert(0, ": ");
        printErrorAndExit("'git checkout ", revision, "' failed with exit status ", status, error);
    }
}

void delta::fetchDependencies(llvm::StringRef packageRoot) {
    PackageManifest manifest(packageRoot);

    for (auto& dependency : manifest.getDeclaredDependencies()) {
        auto path = dependency.getFileSystemPath();

        if (!llvm::sys::fs::exists(path)) {
            cloneGitRepository(dependency.getGitRepositoryUrl(), path);
            checkoutGitRevision(path, dependency.getPackageVersion());
        }
    }
}

std::vector<std::string> delta::getSourceFiles(llvm::StringRef rootDirectory, llvm::StringRef packageManifestPath) {
    std::vector<std::string> sourceFiles;
    std::error_code error;

    for (llvm::sys::fs::recursive_directory_iterator it(rootDirectory, error), end; it != end; it.increment(error)) {
        if (error) {
            llvm::errs() << error.message() << '\n';
            break;
        }

        if (llvm::sys::path::extension(it->path()) == ".delta" && it->path() != packageManifestPath) {
            sourceFiles.push_back(it->path());
        }
    }

    return sourceFiles;
}
