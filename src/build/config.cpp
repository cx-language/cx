#include "config.h"
#include "../ast/module.h"
#include "../driver/driver.h"
#include "../parser/parse.h"
#include "../support/utility.h"
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/Process.h>

using namespace cx;

const char BuildConfig::buildFileName[] = "build.cx";

std::string BuildConfig::Dependency::getFileSystemPath() const {
    auto home = llvm::sys::Process::GetEnv("HOME");
    if (!home) {
        ABORT("environment variable HOME not set");
    }
    return *home + "/.cx/dependencies/" + package + "@" + version;
}

template<typename DeclT, typename DefaultValueT> static auto getConfigValue(Decl* decl, DefaultValueT defaultValue) {
    return decl ? llvm::cast<DeclT>(llvm::cast<VarDecl>(decl)->initializer)->value : defaultValue;
}

static std::vector<std::string> getStringList(Decl* decl) {
    if (!decl) return {};
    auto* array = llvm::cast<ArrayLiteralExpr>(llvm::cast<VarDecl>(decl)->initializer);
    return map(array->elements, [](Expr* element) { return llvm::cast<StringLiteralExpr>(element)->value; });
}

static const StringLiteralExpr* getRequiredString(const AnonymousStructExpr* anonymousStruct, llvm::StringRef name) {
    auto* element = anonymousStruct->getElementByName(name);
    if (!element) {
        ABORT("dependency is missing required '" << name << "' field (expected '(package = ..., url = ..., version = ...)')");
    }
    return llvm::cast<StringLiteralExpr>(element);
}

BuildConfig::BuildConfig(std::string&& rootDirectory, std::vector<std::string> defines) : rootDirectory(std::move(rootDirectory)) {
    if (this->rootDirectory.empty()) return;
    auto buildFilePath = this->rootDirectory + "/" + buildFileName;
    if (!llvm::sys::fs::exists(buildFilePath)) return;

    Module module(buildFileName);
    CompileOptions options;
    options.defines = std::move(defines);
    Parser parser(addFileBufferToModule(buildFilePath, module), module, options);
    parser.parse();
    // TODO: Type-check build file.

    auto& symbols = module.symbolTable;
    name = getConfigValue<StringLiteralExpr>(symbols.findOne("name"), "");
    multitarget = getConfigValue<BoolLiteralExpr>(symbols.findOne("multitarget"), false);
    outputDirectory = getConfigValue<StringLiteralExpr>(symbols.findOne("outputDirectory"), ".");
    this->defines = getStringList(symbols.findOne("defines"));
    headerSearchPaths = getStringList(symbols.findOne("headerSearchPaths"));
    librarySearchPaths = getStringList(symbols.findOne("librarySearchPaths"));
    libraries = getStringList(symbols.findOne("libraries"));
    frameworks = getStringList(symbols.findOne("frameworks"));
    pkgConfigDependencies = getStringList(symbols.findOne("pkgConfigDependencies"));

    if (auto* dependencies = symbols.findOne("dependencies")) {
        auto* array = llvm::cast<ArrayLiteralExpr>(llvm::cast<VarDecl>(dependencies)->initializer);
        for (auto& element : array->elements) {
            auto* anonymousStruct = llvm::cast<AnonymousStructExpr>(&*element);
            auto* package = getRequiredString(anonymousStruct, "package");
            auto* url = getRequiredString(anonymousStruct, "url");
            auto* version = getRequiredString(anonymousStruct, "version");
            declaredDependencies.push_back(Dependency(std::string(package->value), std::string(url->value), std::string(version->value)));
        }
    }
}

std::vector<std::string> BuildConfig::getTargetRootDirectories() const {
    if (!multitarget) return {rootDirectory};

    std::string sourceDir = rootDirectory;
    std::error_code error;

    for (llvm::sys::fs::directory_iterator it(rootDirectory, error), end; it != end; it.increment(error)) {
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
