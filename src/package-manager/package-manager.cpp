#include <string>
#include <system_error>
#include <vector>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/ErrorOr.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/Program.h>
#include <llvm/Support/raw_ostream.h>
#include "package-manager.h"
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

    const char* args[] = {
        gitPath.c_str(),
        "clone",
        repositoryUrl.c_str(),
        path.c_str(),
        nullptr
    };

    std::string error;
    int status = llvm::sys::ExecuteAndWait(gitPath, args, nullptr, nullptr, 0, 0, &error);

    if (status != 0 || !error.empty()) {
        printErrorAndExit("'git clone ", repositoryUrl, " ", path, "' failed with exit status ",
                          status, error.empty() ? "" : ": ", error);
    }
}

static void checkoutGitRevision(const std::string& path, const std::string& revision) {
    auto gitPath = getGitPath();
    auto gitDir = "--git-dir=" + path + "/.git";
    auto workTree = "--work-tree=" + path;

    const char* args[] = {
        gitPath.c_str(),
        gitDir.c_str(),
        workTree.c_str(),
        "checkout",
        revision.c_str(),
        "--quiet",
        nullptr
    };

    std::string error;
    int status = llvm::sys::ExecuteAndWait(gitPath, args, nullptr, nullptr, 0, 0, &error);

    if (status != 0 || !error.empty()) {
        printErrorAndExit("'git checkout ", revision, "' failed with exit status ",
                          status, error.empty() ? "" : ": ", error);
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

std::vector<std::string> delta::getSourceFiles(llvm::StringRef packageRoot) {
    std::vector<std::string> sourceFiles;
    std::error_code error;

    for (llvm::sys::fs::recursive_directory_iterator it(packageRoot, error), end;
         it != end; it.increment(error)) {
        if (error) {
            llvm::errs() << error.message() << '\n';
            break;
        }

        if (llvm::sys::path::extension(it->path()) == ".delta") {
            sourceFiles.push_back(it->path());
        }
    }

    return sourceFiles;
}
