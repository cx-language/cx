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

template<typename DeclT, typename DefaultValueT> static auto getConfigValue(llvm::StringRef name, Decl* decl, DefaultValueT defaultValue) {
    auto* var = decl ? llvm::dyn_cast<VarDecl>(decl) : nullptr;
    auto* value = var ? llvm::dyn_cast<DeclT>(var->initializer) : nullptr;
    if (decl && !value) {
        ABORT("invalid '" << name << "' value in build file (wrong type)");
    }
    return value ? value->value : defaultValue;
}

static std::vector<std::string> getStringList(llvm::StringRef name, Decl* decl) {
    if (!decl) return {};
    auto* var = llvm::dyn_cast<VarDecl>(decl);
    auto* array = var ? llvm::dyn_cast<ArrayLiteralExpr>(var->initializer) : nullptr;
    if (!array) {
        ABORT("invalid '" << name << "' value in build file (expected a list of strings)");
    }
    std::vector<std::string> result;
    for (Expr* element : array->elements) {
        auto* string = llvm::dyn_cast<StringLiteralExpr>(element);
        if (!string) {
            ABORT("invalid '" << name << "' value in build file (expected a list of strings)");
        }
        result.push_back(string->value);
    }
    return result;
}

static Decl* findConfigKey(SymbolTable& symbols, llvm::StringRef name) {
    auto results = symbols.findFirst(name);
    if (results.empty()) return nullptr;
    if (results.size() > 1) {
        ABORT("duplicate '" << name << "' key in build file");
    }
    return results.front();
}

static const StringLiteralExpr* getRequiredString(const AnonymousStructExpr* anonymousStruct, llvm::StringRef name) {
    auto* element = anonymousStruct->getElementByName(name);
    if (!element) {
        ABORT("dependency is missing required '" << name << "' field (expected '(package = ..., url = ..., version = ...)')");
    }
    auto* string = llvm::dyn_cast<StringLiteralExpr>(element);
    if (!string) {
        ABORT("invalid '" << name << "' value in dependency entry (expected a string)");
    }
    return string;
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
    name = getConfigValue<StringLiteralExpr>("name", findConfigKey(symbols, "name"), "");
    multitarget = getConfigValue<BoolLiteralExpr>("multitarget", findConfigKey(symbols, "multitarget"), false);
    outputDirectory = getConfigValue<StringLiteralExpr>("outputDirectory", findConfigKey(symbols, "outputDirectory"), ".");
    this->defines = getStringList("defines", findConfigKey(symbols, "defines"));
    headerSearchPaths = getStringList("headerSearchPaths", findConfigKey(symbols, "headerSearchPaths"));
    librarySearchPaths = getStringList("librarySearchPaths", findConfigKey(symbols, "librarySearchPaths"));
    libraries = getStringList("libraries", findConfigKey(symbols, "libraries"));
    frameworks = getStringList("frameworks", findConfigKey(symbols, "frameworks"));
    pkgConfigDependencies = getStringList("pkgConfigDependencies", findConfigKey(symbols, "pkgConfigDependencies"));

    if (auto* dependencies = findConfigKey(symbols, "dependencies")) {
        auto* var = llvm::dyn_cast<VarDecl>(dependencies);
        auto* array = var ? llvm::dyn_cast<ArrayLiteralExpr>(var->initializer) : nullptr;
        if (!array) {
            ABORT("invalid 'dependencies' value in build file (expected a list of '(package = ..., url = ..., version = ...)' structs)");
        }
        for (auto& element : array->elements) {
            auto* anonymousStruct = llvm::dyn_cast<AnonymousStructExpr>(&*element);
            if (!anonymousStruct) {
                ABORT("invalid 'dependencies' entry in build file (expected '(package = ..., url = ..., version = ...)')");
            }
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
