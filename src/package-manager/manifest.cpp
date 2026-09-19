#include "manifest.h"
#include "../ast/module.h"
#include "../driver/driver.h"
#include "../parser/parse.h"
#include "../support/utility.h"
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/Process.h>

using namespace cx;

const char PackageManifest::manifestFileName[] = "package.cx";

std::string PackageManifest::Dependency::getGitRepositoryUrl() const {
    return "https://github.com/" + packageIdentifier + ".git";
}

std::string PackageManifest::Dependency::getFileSystemPath() const {
    auto home = llvm::sys::Process::GetEnv("HOME");
    if (!home) {
        ABORT("environment variable HOME not set");
    }
    return *home + "/.cx/dependencies/" + packageIdentifier + "@" + packageVersion;
}

template<typename DeclT, typename DefaultValueT> static auto getConfigValue(Decl* decl, DefaultValueT defaultValue) {
    return decl ? llvm::cast<DeclT>(llvm::cast<VarDecl>(decl)->initializer)->value : defaultValue;
}

static std::vector<std::string> getStringList(Decl* decl) {
    if (!decl) return {};
    auto* array = llvm::cast<ArrayLiteralExpr>(llvm::cast<VarDecl>(decl)->initializer);
    return map(array->elements, [](Expr* element) { return llvm::cast<StringLiteralExpr>(element)->value; });
}

PackageManifest::PackageManifest(std::string&& packageRoot, std::vector<std::string> defines) : packageRoot(std::move(packageRoot)) {
    auto manifestPath = this->packageRoot + "/" + manifestFileName;
    if (!llvm::sys::fs::exists(manifestPath)) return;

    Module module(manifestFileName);
    CompileOptions options;
    options.defines = std::move(defines);
    Parser parser(addFileBufferToModule(manifestPath, module), module, options);
    parser.parse();
    // TODO: Type-check package manifest.

    auto& symbols = module.symbolTable;
    packageName = getConfigValue<StringLiteralExpr>(symbols.findOne("name"), "");
    multitarget = getConfigValue<BoolLiteralExpr>(symbols.findOne("multitarget"), false);
    outputDirectory = getConfigValue<StringLiteralExpr>(symbols.findOne("outputDirectory"), "bin");
    this->defines = getStringList(symbols.findOne("defines"));
    libraries = getStringList(symbols.findOne("libraries"));
    frameworks = getStringList(symbols.findOne("frameworks"));
    pkgConfigDependencies = getStringList(symbols.findOne("pkgConfigDependencies"));

    if (auto* dependencies = symbols.findOne("dependencies")) {
        auto* array = llvm::cast<ArrayLiteralExpr>(llvm::cast<VarDecl>(dependencies)->initializer);
        for (auto& element : array->elements) {
            auto* tuple = llvm::cast<TupleExpr>(&*element);
            auto* package = llvm::cast<StringLiteralExpr>(tuple->getElementByName("package"));
            auto* version = llvm::cast<StringLiteralExpr>(tuple->getElementByName("version"));
            declaredDependencies.push_back(Dependency(std::string(package->value), std::string(version->value)));
        }
    }
}

std::vector<std::string> PackageManifest::getTargetRootDirectories() const {
    if (!multitarget) return {packageRoot};

    std::string sourceDir = packageRoot;
    std::error_code error;

    for (llvm::sys::fs::directory_iterator it(packageRoot, error), end; it != end; it.increment(error)) {
        if (!llvm::sys::fs::is_directory(it->path())) continue;
        llvm::StringRef dir = llvm::sys::path::filename(it->path());
        if (dir.equals_insensitive("src") || dir.equals_insensitive("source") || dir.equals_insensitive("sources")) {
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
