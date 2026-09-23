// Tests for BuildConfig dependency parsing (build.cx `dependencies` entries).
//
// Each test case runs in its own process: the test binary takes the case name
// as its (single) argument. Cases that abort the process (rejected input) are
// asserted on via CTest PASS_REGULAR_EXPRESSION instead of in-process checks.

#include "../../src/build/config.h"
#include "../../src/build/dependencies.h"
#include <cstdlib>
#include <iostream>
#include <string>
#pragma warning(push, 0)
#include <llvm/ADT/STLExtras.h>
#include <llvm/ADT/SmallString.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>
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

void testSearchPaths() {
    auto project = writeTestProject("var headerSearchPaths = [\"vendor/mylib/include\"]\n"
                                    "var librarySearchPaths = [\"vendor/mylib/lib\"]\n"
                                    "var libraries = [\"mylib\"]\n");
    cx::BuildConfig config{std::string(project)};

    check(config.headerSearchPaths.size() == 1 && config.headerSearchPaths[0] == "vendor/mylib/include", "header search paths are parsed");
    check(config.librarySearchPaths.size() == 1 && config.librarySearchPaths[0] == "vendor/mylib/lib", "library search paths are parsed");
    check(config.libraries.size() == 1 && config.libraries[0] == "mylib", "libraries are parsed");
}

void testMissingBuildFile() {
    llvm::SmallString<128> dir;
    checkNoError(llvm::sys::fs::createUniqueDirectory("cx-build-config-test", dir), "create temp project directory");
    cx::BuildConfig config{std::string(dir.str())};
    check(config.name.empty(), "name defaults to empty");
    check(config.outputDirectory == ".", "output directory defaults to project root");
    check(!config.multitarget, "multitarget defaults to false");
}

// Points HOME at a fresh temp directory so dependency checkout paths are hermetic.
std::string pointHomeAtTempDir() {
    llvm::SmallString<128> home;
    checkNoError(llvm::sys::fs::createUniqueDirectory("cx-build-config-home", home), "create temp HOME directory");
#ifdef _WIN32
    _putenv(("HOME=" + home).str().c_str());
#else
    setenv("HOME", home.c_str(), 1);
#endif
    return home.str().str();
}

// Writes content to dir/relPath, creating parent directories.
void writeTestFile(const std::string& dir, const char* relPath, const char* content) {
    checkNoError(llvm::sys::fs::create_directories(dir + "/" + llvm::sys::path::parent_path(relPath).str()), "create parent directories");
    std::error_code error;
    llvm::raw_fd_ostream file(dir + "/" + relPath, error);
    checkNoError(error, "open file for writing");
    file << content;
    file.close();
}

const cx::BuildConfig::ResolvedDependency* findRecord(const cx::BuildConfig& config, const char* package) {
    for (auto& record : config.resolvedDependencies) {
        if (record.package == package) return &record;
    }
    return nullptr;
}

void testClosureTransitive() {
    std::string home = pointHomeAtTempDir();
    // Pre-seed checkouts so the walk never shells out to git.
    writeTestFile(home + "/.cx/dependencies/mylib@v1", "build.cx",
                  "var defines = [\"MYLIB\"]\n"
                  "var headerSearchPaths = [\"include\"]\n"
                  "var dependencies = [(package = \"helper\", url = \"https://example.com/helper.git\", version = \"v2\")]\n");
    writeTestFile(home + "/.cx/dependencies/helper@v2", "build.cx", "var defines = [\"HELPER\"]\n");
    auto project = writeTestProject("var dependencies = [(package = \"mylib\", url = \"https://example.com/mylib.git\", version = \"v1\")]\n");
    cx::BuildConfig config{std::string(project)};
    cx::CompileOptions baseOptions;
    cx::resolveDependencyClosure(config, baseOptions, /*fetchMissing=*/true);

    check(config.resolvedDependencies.size() == 2, "direct and transitive dependencies are collected");
    auto* mylib = findRecord(config, "mylib");
    auto* helper = findRecord(config, "helper");
    if (!mylib || !helper) return;
    check(mylib->version == "v1", "version is recorded");
    check(mylib->rootDirectory == home + "/.cx/dependencies/mylib@v1", "checkout path is recorded");
    check(mylib->requiredBy == "the project build file", "direct requirement is attributed");
    check(helper->requiredBy == "mylib@v1", "transitive requirement is attributed");
    check(llvm::is_contained(mylib->options.defines, "MYLIB"), "dependency defines apply to its package");
    check(llvm::is_contained(mylib->options.importSearchPaths, home + "/.cx/dependencies/mylib@v1/include"),
          "relative search paths absolutize against the dependency root");
    check(llvm::is_contained(helper->options.defines, "HELPER"), "transitive defines apply to their package");
}

void testClosureCycle() {
    std::string home = pointHomeAtTempDir();
    writeTestFile(home + "/.cx/dependencies/a@v1", "build.cx",
                  "var dependencies = [(package = \"b\", url = \"https://example.com/b.git\", version = \"v1\")]\n");
    writeTestFile(home + "/.cx/dependencies/b@v1", "build.cx",
                  "var dependencies = [(package = \"a\", url = \"https://example.com/a.git\", version = \"v1\")]\n");
    auto project = writeTestProject("var dependencies = [(package = \"a\", url = \"https://example.com/a.git\", version = \"v1\")]\n");
    cx::BuildConfig config{std::string(project)};
    cx::CompileOptions baseOptions;
    cx::resolveDependencyClosure(config, baseOptions, /*fetchMissing=*/true);

    check(config.resolvedDependencies.size() == 2, "cyclic dependencies terminate with one record each");
}

void testClosureVendored() {
    auto project = writeTestProject("var name = \"vendored\"\n");
    writeTestFile(project, "vendor/greet/build.cx", "var defines = [\"GREET\"]\n");
    cx::BuildConfig config{std::string(project)};
    cx::CompileOptions baseOptions;
    cx::resolveDependencyClosure(config, baseOptions, /*fetchMissing=*/false);

    check(config.resolvedDependencies.size() == 1, "vendored package is collected without fetching");
    auto* greet = findRecord(config, "greet");
    if (!greet) return;
    check(greet->version.empty(), "vendored packages carry no version");
    check(greet->requiredBy == "the vendor directory", "vendored requirement is attributed");
    check(llvm::is_contained(greet->options.defines, "GREET"), "vendored defines apply to their package");
}

void testClosureMissingSkipped() {
    pointHomeAtTempDir();
    auto project = writeTestProject("var dependencies = [(package = \"ghost\", url = \"https://example.com/ghost.git\", version = \"v1\")]\n");
    cx::BuildConfig config{std::string(project)};
    cx::CompileOptions baseOptions;
    cx::resolveDependencyClosure(config, baseOptions, /*fetchMissing=*/false);

    check(config.resolvedDependencies.empty(), "missing checkouts are skipped without fetching");
}

void testResolveDependency() {
    using Resolved = cx::BuildConfig::ResolvedDependency;
    Resolved first;
    first.package = "lib";
    first.rootDirectory = "/deps/lib@v1";
    first.requiredBy = "the project build file";
    Resolved same;
    same.package = "lib";
    same.rootDirectory = "/deps/lib@v1";
    same.requiredBy = "other@v1";
    Resolved second;
    second.package = "lib";
    second.rootDirectory = "/deps/lib@v2";
    second.requiredBy = "other@v1";
    Resolved vendored;
    vendored.package = "lib";
    vendored.rootDirectory = "./vendor/lib";
    vendored.requiredBy = "the vendor directory";

    std::vector<Resolved> sole = {first};
    auto single = cx::resolveDependency(sole, "lib");
    check(!single.ambiguous && single.dependency && single.dependency->rootDirectory == "/deps/lib@v1", "sole provider resolves");

    auto missing = cx::resolveDependency(sole, "ghost");
    check(!missing.ambiguous && !missing.dependency, "unknown package resolves empty");

    std::vector<Resolved> repeated = {first, same};
    auto collapse = cx::resolveDependency(repeated, "lib");
    check(!collapse.ambiguous && collapse.dependency && collapse.dependency->rootDirectory == "/deps/lib@v1", "repeated declarations collapse");

    std::vector<Resolved> conflicted = {first, second};
    auto conflict = cx::resolveDependency(conflicted, "lib");
    check(conflict.ambiguous && !conflict.dependency, "two versions report ambiguous");
    check(conflict.ambiguityDetail.find("/deps/lib@v1") != std::string::npos && conflict.ambiguityDetail.find("/deps/lib@v2") != std::string::npos,
          "ambiguity names both sources");

    std::vector<Resolved> overlapped = {first, vendored};
    auto overlap = cx::resolveDependency(overlapped, "lib");
    check(overlap.ambiguous, "vendored and fetched overlap reports ambiguous");
}

} // namespace

int main(int argc, const char** argv) {
    if (argc != 2) {
        std::cerr << "usage: test_build_config <git-urls|missing-url|missing-build-file|search-paths|\n"
                     "closure-transitive|closure-cycle|closure-vendored|closure-missing-skipped|resolve-dependency>\n";
        return 2;
    }

    std::string testCase = argv[1];
    if (testCase == "git-urls") {
        testGitUrls();
    } else if (testCase == "search-paths") {
        testSearchPaths();
    } else if (testCase == "missing-build-file") {
        testMissingBuildFile();
    } else if (testCase == "missing-url") {
        testMissingUrl();
    } else if (testCase == "closure-transitive") {
        testClosureTransitive();
    } else if (testCase == "closure-cycle") {
        testClosureCycle();
    } else if (testCase == "closure-vendored") {
        testClosureVendored();
    } else if (testCase == "closure-missing-skipped") {
        testClosureMissingSkipped();
    } else if (testCase == "resolve-dependency") {
        testResolveDependency();
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
