// Tests for BuildConfig dependency parsing (build.cx `dependencies` entries).
//
// Each test case runs in its own process: the test binary takes the case name
// as its (single) argument. Cases that abort the process (rejected input) are
// asserted on via CTest PASS_REGULAR_EXPRESSION instead of in-process checks.

#include "../../src/build/config.h"
#include <cstdlib>
#include <iostream>
#include <string>
#pragma warning(push, 0)
#include <llvm/ADT/SmallString.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_ostream.h>
#pragma warning(pop)

namespace {

int failures = 0;

void check(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        failures++;
    } else {
        std::cout << "PASS: " << message << '\n';
    }
}

void checkNoError(std::error_code error, const char* message) {
    if (error) {
        std::cerr << "FAIL: " << message << ": " << error.message() << '\n';
        failures++;
    } else {
        std::cout << "PASS: " << message << '\n';
    }
}

// Writes buildContent as build.cx into a fresh temp directory and returns its path.
std::string writeTestProject(const char* buildContent) {
    llvm::SmallString<128> dir;
    checkNoError(llvm::sys::fs::createUniqueDirectory("cx-build-config-test", dir), "create temp project directory");
    std::string buildFilePath = (dir + "/build.cx").str();
    std::error_code error;
    llvm::raw_fd_ostream file(buildFilePath, error);
    checkNoError(error, "open temp build.cx for writing");
    file << buildContent;
    file.close();
    return dir.str().str();
}

void testGitUrls() {
    // Point HOME at a temp dir so getFileSystemPath() is hermetic.
    llvm::SmallString<128> home;
    checkNoError(llvm::sys::fs::createUniqueDirectory("cx-build-config-home", home), "create temp HOME directory");
#ifdef _WIN32
    _putenv(("HOME=" + home).str().c_str());
#else
    setenv("HOME", home.c_str(), 1);
#endif

    auto project = writeTestProject("var dependencies = [\n"
                                    "    (package = \"https-lib\", url = \"https://example.com/libs/https-lib.git\", version = \"v1.0\"),\n"
                                    "    (package = \"ssh-lib\", url = \"git@example.com:libs/ssh-lib.git\", version = \"main\"),\n"
                                    "    (package = \"file-lib\", url = \"file:///tmp/libs/file-lib.git\", version = \"abc123\"),\n"
                                    "]\n");
    cx::BuildConfig config{std::string(project)};

    check(config.declaredDependencies.size() == 3, "all three dependencies are parsed");
    if (config.declaredDependencies.size() != 3) return;
    check(config.declaredDependencies[0].package == "https-lib", "package name is parsed");
    check(config.declaredDependencies[0].url == "https://example.com/libs/https-lib.git", "https URL is stored verbatim");
    check(config.declaredDependencies[0].version == "v1.0", "version is parsed");
    check(config.declaredDependencies[1].url == "git@example.com:libs/ssh-lib.git", "scp-like SSH URL is stored verbatim");
    check(config.declaredDependencies[2].url == "file:///tmp/libs/file-lib.git", "file URL is stored verbatim");
    check(config.declaredDependencies[0].getFileSystemPath() == (home + "/.cx/dependencies/https-lib@v1.0").str(),
          "checkout path is derived from package and version");
}

void testMissingUrl() {
    // The old two-field form is rejected with an actionable error. The abort
    // below is the expected outcome; CTest matches its message.
    auto project = writeTestProject("var dependencies = [(package = \"owner/repo\", version = \"v1.0\")]\n");
    cx::BuildConfig config{std::string(project)};
    check(false, "dependency without url field is rejected");
}

} // namespace

int main(int argc, const char** argv) {
    if (argc != 2) {
        std::cerr << "usage: test_build_config <git-urls|missing-url>\n";
        return 2;
    }

    std::string testCase = argv[1];
    if (testCase == "git-urls") {
        testGitUrls();
    } else if (testCase == "missing-url") {
        testMissingUrl();
    } else {
        std::cerr << "unknown test case '" << testCase << "'\n";
        return 2;
    }

    if (failures == 0) {
        std::cout << "test case '" << testCase << "' passed.\n";
        return 0;
    }
    std::cerr << failures << " check(s) failed in test case '" << testCase << "'.\n";
    return 1;
}
