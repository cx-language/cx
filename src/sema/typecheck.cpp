#include "typecheck.h"
#include <system_error>
#pragma warning(push, 0)
#include <llvm/ADT/DenseSet.h>
#include <llvm/ADT/SmallPtrSet.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/SaveAndRestore.h>
#pragma warning(pop)
#include "../ast/module.h"
#include "../build/dependencies.h"
#include "../driver/driver.h"
#include "../parser/parse.h"

using namespace cx;

TypeDecl* Typechecker::getTypeDecl(const BasicType& type) {
    if (auto* typeDecl = type.decl) {
        return typeDecl;
    }

    auto decls = findDecls(type.getQualifiedName());

    if (!decls.empty()) {
        ASSERT(decls.size() == 1);
        return llvm::dyn_cast_or_null<TypeDecl>(decls[0]);
    }

    decls = findDecls(type.name);
    if (decls.empty()) return nullptr;
    ASSERT(decls.size() == 1);
    auto instantiation = llvm::cast<TypeTemplate>(decls[0])->instantiate(type.genericArgs);
    currentModule->addToSymbolTable(*instantiation);
    // Array is a builtin-backed declaration. Resolve its methods on demand so
    // using one array operation does not typecheck every Array method.
    if (type.name != "Array") deferTypechecking(instantiation);
    return instantiation;
}

static std::error_code importModuleSourcesInDirectoryRecursively(const llvm::Twine& directoryPath, Module& module, const CompileOptions& options) {
    PhaseTimer timer("parse-" + module.name);
    std::error_code error;
    std::vector<std::string> paths;

    // ponytail: an imported package's own vendor/ subdirectory (transitive vendoring) compiles as part of it;
    // split into nested packages via `import` once a package needs its vendored deps versioned independently.
    for (llvm::sys::fs::recursive_directory_iterator it(directoryPath, error), end; it != end; it.increment(error)) {
        if (error) break;
        // Only the package root's build file is reserved (see isRootBuildFile).
        if (llvm::sys::path::extension(it->path()) == ".cx" && !isRootBuildFile(it->path(), directoryPath.str())) {
            paths.push_back(it->path());
        }
    }

    if (!error) {
        llvm::sort(paths);

        for (auto& path : paths) {
            Parser parser(addFileBufferToModule(path, module), module, options);
            parser.parse();
        }
    }

    if (module.sourceFiles.empty()) {
        REPORT_ERROR(Location(), "Module '" << module.name << "' import failed: no source files found in '" << directoryPath << "' or its subdirectories");
    }

    return error;
}

llvm::ErrorOr<const Module&> Typechecker::importModule(SourceFile* importer, llvm::StringRef moduleName) {
    auto it = Module::getAllImportedModulesMap().find(moduleName);
    if (it != Module::getAllImportedModulesMap().end()) {
        if (importer) importer->addImportedModule(it->second);
        return *it->second;
    }

    PhaseTimer timer("import-" + moduleName.str());
    auto module = new Module(moduleName.str());
    std::error_code error = std::make_error_code(std::errc::no_such_file_or_directory);

    // Each package parses and typechecks with its own options; a cached module
    // is always complete, so reimporting never reparses.
    const CompileOptions* packageOptions = &options;
    const BuildConfig::ResolvedDependency* resolution = nullptr;
    if (dependencies) {
        auto result = resolveDependency(*dependencies, moduleName);
        if (result.ambiguous) {
            // typecheckImportDecl reports this at the import; the std pre-import
            // below has no location, so report here instead.
            REPORT_ERROR(Location(), result.ambiguityDetail);
            return error;
        }
        resolution = result.dependency;
    }

    if (resolution) {
        packageOptions = &resolution->options;
        error = importModuleSourcesInDirectoryRecursively(resolution->rootDirectory, *module, *packageOptions);
        goto done;
    }

    for (llvm::StringRef importPath : options.importSearchPaths) {
        auto modulePath = (importPath + "/" + moduleName).str();
        if (llvm::sys::fs::is_directory(modulePath)) {
            error = importModuleSourcesInDirectoryRecursively(modulePath, *module, *packageOptions);
            goto done;
        }
    }

done:
    if (error) return error;
    if (importer) importer->addImportedModule(module);
    Module::getAllImportedModulesMap()[module->name] = module;
    typecheckModule(*module, *packageOptions, false);
    return *module;
}

void Typechecker::deferTypechecking(Decl* decl) {
    // Fully checked declarations need no work; declarations whose body is
    // being checked are already on the drain stack (e.g. recursion), so
    // re-queueing them would ping-pong forever.
    if (decl->checkState == Decl::CheckState::Checked || decl->checkState == Decl::CheckState::CheckingBody) {
        return;
    }
    for (auto existing : declsToTypecheck) {
        if (existing == decl) {
            return;
        }
    }
    declsToTypecheck.push_back(decl);
}

void Typechecker::markReferenced(Decl* decl) {
    decl->referenced = true;
    // Body-bearing declarations are checked on demand; everything else is
    // either already checked (globals are prepass roots, locals inline) or
    // resolved on demand (aliases). Type templates are never queued: there
    // is no postProcess case for them, and uses instantiate instead.
    switch (decl->kind) {
    case DeclKind::FunctionDecl:
    case DeclKind::MethodDecl:
    case DeclKind::ConstructorDecl:
    case DeclKind::DestructorDecl:
    case DeclKind::FunctionTemplate:
    case DeclKind::TypeDecl:
    case DeclKind::EnumDecl:
        deferTypechecking(decl);
        break;
    default:
        break;
    }
}

void Typechecker::postProcess() {
    llvm::SaveAndRestore setPostProcessing(isPostProcessing, true);

    while (!declsToTypecheck.empty()) {
        auto currentDeclsToTypecheck = std::move(declsToTypecheck);

        // Each queued declaration reports independently, like the eager loop: one bad
        // declaration must not abort checking (or diagnostics) of the rest of the queue.
        for (auto* decl : currentDeclsToTypecheck) {
            try {
                switch (decl->kind) {
                case DeclKind::FunctionDecl:
                case DeclKind::MethodDecl:
                case DeclKind::ConstructorDecl:
                case DeclKind::DestructorDecl:
                    typecheckFunctionDecl(*llvm::cast<FunctionDecl>(decl));
                    break;
                case DeclKind::FunctionTemplate:
                    typecheckFunctionTemplate(*llvm::cast<FunctionTemplate>(decl));
                    break;
                case DeclKind::TypeDecl:
                    typecheckTypeSignature(*llvm::cast<TypeDecl>(decl));
                    break;
                case DeclKind::EnumDecl:
                    typecheckEnumSignature(*llvm::cast<EnumDecl>(decl));
                    break;
                default:
                    llvm_unreachable("invalid deferred decl");
                }
            } catch (const CompileError& error) {
                error.report();
            }
        }
    }
}

static void checkUnusedDeclsInModule(const Module& module) {
    for (auto& sourceFile : module.sourceFiles) {
        for (auto& decl : sourceFile.topLevelDecls) {
            if (decl->isReferenced()) continue;

            if (decl->isFunctionDecl() || decl->isFunctionTemplate()) {
                if (auto* functionDecl = llvm::dyn_cast<FunctionDecl>(decl)) {
                    // Any main-module non-method "main" is either the entry point
                    // or already diagnosed (a duplicate); never warn on those.
                    if (functionDecl->isMain() && !functionDecl->isMethodDecl()) continue;
                } else if (decl->isMain()) {
                    continue;
                }
                // Test functions are entry points for `cx test`, like main is for `cx run`.
                if (auto* functionDecl = llvm::dyn_cast<FunctionDecl>(decl); functionDecl && functionDecl->isTest) continue;
                if (auto* functionTemplate = llvm::dyn_cast<FunctionTemplate>(decl); functionTemplate && functionTemplate->functionDecl->isTest) {
                    continue;
                }
                WARN(decl->getLocation(), "unused declaration '" << decl->getName() << "'");
            }
        }
    }
}

// Warns about unused declarations after the whole program is typechecked, so
// that references from importing modules are seen. Only the main module is
// checked: imported modules are library code the importer doesn't control.
void Typechecker::checkUnusedDecls(const Module& mainModule) {
    if (options.noUnusedWarnings) return;
    checkUnusedDeclsInModule(mainModule);
}

void Typechecker::typecheckModule(Module& module, const CompileOptions& packageOptions, bool isMainModule) {
    llvm::SaveAndRestore restoreModule(currentModule);
    llvm::SaveAndRestore restoreSourceFile(currentSourceFile);
    llvm::SaveAndRestore restoreOptions(options, packageOptions);
    if (isMainModule) mainModule = &module;

    auto stdModule = importModule(nullptr, "std");
    if (!stdModule) {
        std::string searched;
        for (auto& path : options.importSearchPaths) {
            searched += "\n  " + path + "/std";
        }
        ABORT("couldn't import the standard library (" << stdModule.getError().message() << "); searched:" << searched
                                                       << "\n(set CX_ROOT to the directory containing std/ to override)");
    }

    // Process all imports before typechecking anything else, so that deferred typechecking
    // doesn't depend on the order in which files happen to be processed. C headers in particular
    // can't refer to declarations in the importing module, so importing them early is always safe.
    for (auto& sourceFile : module.sourceFiles) {
        for (auto& decl : sourceFile.topLevelDecls) {
            if (!llvm::isa<ImportDecl>(decl)) continue;
            currentModule = &module;
            currentSourceFile = &sourceFile;

            try {
                typecheckImportDecl(*llvm::cast<ImportDecl>(decl));
                postProcess();
            } catch (const CompileError& error) {
                error.report();
            }
        }
    }

    currentModule = &module;
    canonicalizeTypeAliases();

    // The main module is checked eagerly so errors surface even in unused
    // code. Imported modules only parse here; their declarations check on
    // first use (globals are still always checked: their initializers run).
    // --check-all checks everything eagerly, e.g. for validating dependencies.
    bool checkEagerly = isMainModule || module.isCHeaderImport || options.checkAll;
    if (checkEagerly) {
        // Typecheck implemented interfaces so that inherited methods and fields are added to the implementing type before they're referenced.
        for (auto& sourceFile : module.sourceFiles) {
            for (auto& decl : sourceFile.topLevelDecls) {
                currentModule = &module;
                currentSourceFile = &sourceFile;

                if (auto typeDecl = llvm::dyn_cast<TypeDecl>(decl)) {
                    ensureInterfaces(*typeDecl);
                }
            }
        }
    }

    // Infer the types of global variables for use before their declaration.
    for (auto& sourceFile : module.sourceFiles) {
        currentModule = &module;
        currentSourceFile = &sourceFile;

        for (auto& decl : sourceFile.topLevelDecls) {
            if (auto* varDecl = llvm::dyn_cast<VarDecl>(decl)) {
                try {
                    typecheckVarDecl(*varDecl);
                } catch (const CompileError& error) {
                    error.report();
                }
            } else if (auto* typeDecl = llvm::dyn_cast<TypeDecl>(decl)) {
                for (auto* staticConst : typeDecl->staticConsts) {
                    try {
                        typecheckVarDecl(*staticConst);
                    } catch (const CompileError& error) {
                        error.report();
                    }
                }
            }
        }

        postProcess();
    }

    if (checkEagerly) {
        for (auto& sourceFile : module.sourceFiles) {
            for (auto& decl : sourceFile.topLevelDecls) {
                currentModule = &module;
                currentSourceFile = &sourceFile;

                // Imports were already processed in the pre-pass above.
                if (!decl->isVarDecl() && !decl->isImportDecl()) {
                    try {
                        typecheckTopLevelDecl(*decl);
                        postProcess();
                    } catch (const CompileError& error) {
                        error.report();
                    }
                }
            }
        }
    }

    // IRGen looks these up directly in the std symbol table, bypassing sema
    // name resolution, so they would never be demand-checked otherwise. Only
    // the main module triggers this: it runs after all imports are parsed.
    if (isMainModule) {
        try {
            ensureImplicitRuntimeUses(module);
            postProcess();
        } catch (const CompileError& error) {
            error.report();
        }
    }

    postProcess();
}

void Typechecker::ensureImplicitRuntimeUses(const Module& mainModule) {
    auto* stdModule = Module::getStdlibModule();
    if (!stdModule) return;

    // IRGen materializes argv only for the entry-point main with one parameter
    // (see emitMainArgv in irgen-decl.cpp); only the main module can hold it.
    bool usesArgv = false;
    for (auto& sourceFile : mainModule.sourceFiles) {
        for (auto* decl : sourceFile.topLevelDecls) {
            if (auto* functionDecl = llvm::dyn_cast<FunctionDecl>(decl); functionDecl && functionDecl->isEntryPoint && functionDecl->getParams().size() == 1) {
                usesArgv = true;
            }
        }
    }

    // Single-callee runtime hooks, mirroring the backend lookups in
    // irgen-decl.cpp (malloc) and irgen-expr.cpp (assertFail). Missing decls
    // are the backend's error to report, as before.
    if (usesArgv) {
        if (Decl* mallocDecl = stdModule->symbolTable.findOne("malloc")) markReferenced(mallocDecl);
    }
    if (usesArgv || implicitUses.assertCall || implicitUses.checkedArithmetic || implicitUses.unwrap || implicitUses.enumSwitch) {
        if (Decl* assertDecl = stdModule->symbolTable.findOne("assertFail")) markReferenced(assertDecl);
    }

    // Overload sets IRGen scans by parameter shape. Ensure each signature before
    // reading it, then whole-check only the overloads IRGen might call. These
    // predicates mirror the backend scans; keep them in sync: string.init arities
    // live in irgen-decl.cpp (emitMainArgv) and irgen-expr.cpp (emitStringLiteralExpr),
    // the string == in irgen-stmt.cpp (emitStringSwitchStmt). The backend asserts the
    // callees are checked, so a missed implicitUses flag fails loudly in tests
    // instead of miscompiling.
    if (usesArgv || implicitUses.stringLiteral) {
        for (Decl* decl : stdModule->symbolTable.findInTopLevelScope("string.init")) {
            ensureSignature(*decl);
            auto* ctor = llvm::dyn_cast<ConstructorDecl>(decl);
            if (!ctor) continue;
            auto params = ctor->getParams();
            if (usesArgv && params.size() == 1 && params[0].type.isPointerType() && params[0].type.getPointee().isChar()) markReferenced(decl);
            if (implicitUses.stringLiteral && params.size() == 2 && params[0].type.isPointerType() && params[1].type.isInt32()) markReferenced(decl);
        }
    }
    if (implicitUses.stringSwitch) {
        for (Decl* decl : stdModule->symbolTable.findInTopLevelScope("==")) {
            ensureSignature(*decl);
            auto* functionDecl = llvm::dyn_cast<FunctionDecl>(decl);
            if (!functionDecl) continue;
            auto params = functionDecl->getParams();
            if (params.size() == 2 && params[0].type.isBasicType() && params[0].type.getName() == "string" && params[1].type.isBasicType()
                && params[1].type.getName() == "string") {
                markReferenced(decl);
            }
        }
    }
}

static llvm::SmallVector<Decl*, 8> findDeclsInModules(llvm::StringRef name, llvm::ArrayRef<Module*> modules) {
    ASSERT(!name.empty());
    llvm::SmallVector<Decl*, 8> decls;

    for (auto& module : modules) {
        llvm::append_range(decls, module->symbolTable.findFirst(name));
    }

    return decls;
}

static Decl* findDeclInModules(llvm::StringRef name, Location location, llvm::ArrayRef<Module*> modules) {
    ASSERT(!name.empty());
    auto decls = findDeclsInModules(name, modules);

    if (decls.size() == 1) {
        return decls[0];
    } else if (decls.empty()) {
        return nullptr;
    } else if (llvm::all_of(decls, [](Decl* decl) { return decl->getModule() && decl->getModule()->isCHeaderImport; })
               && llvm::all_of(decls, [&](Decl* decl) { return decl->kind == decls[0]->kind; })) {
        // Duplicate declarations of the same thing from C headers resolve to the last one.
        return decls.back();
    } else {
        ERROR(location, "ambiguous reference to '" << name << "'");
    }
}

void Typechecker::setDeclContext(Decl& decl) {
    Module* module = decl.getModule();
    if (!module) return;
    const char* file = decl.getLocation().file;
    if (currentModule == module && currentSourceFile && file && currentSourceFile->filePath == file) return;
    currentModule = module;
    if (file) {
        for (auto& sourceFile : module->sourceFiles) {
            if (sourceFile.filePath == file) {
                currentSourceFile = &sourceFile;
                return;
            }
        }
    }
    // Synthesized declarations carry no file; the caller's file context applies.
}

Decl* Typechecker::findDecl(llvm::StringRef name, Location location, Location endLocation) {
    ASSERT(!name.empty());

    if (Decl* match = findDeclInModules(name, location, currentModule)) {
        ensureSignature(*match);
        return match;
    }

    for (FunctionDecl* function = currentFunction; function; function = function->parentFunction) {
        if (auto* typeDecl = function->getTypeDecl()) {
            for (auto& field : typeDecl->fields) {
                if (field.getName() == name) {
                    return &field;
                }
            }
            for (auto* staticConst : typeDecl->staticConsts) {
                if (staticConst->getName() == name) {
                    return staticConst;
                }
            }
        }
    }

    if (Decl* match = findDeclInModules(name, location, currentSourceFile->importedModules)) {
        ensureSignature(*match);
        return match;
    }

    if (Decl* match = findDeclInModules(name, location, Module::getStdlibModule())) {
        ensureSignature(*match);
        return match;
    }

    ERROR_RANGE(location, endLocation, "unknown identifier '" << name << "'");
}

static void appendUnique(std::vector<Decl*>& target, llvm::ArrayRef<Decl*> source) {
    for (auto& element : source) {
        // findDecls searches overlapping scopes (stdlib both directly and via
        // getAllImportedModules), so the same decl legitimately appears twice.
        if (!llvm::is_contained(target, element)) {
            target.push_back(element);
        }
    }
}

std::vector<Decl*> Typechecker::findDecls(llvm::StringRef name, TypeDecl* receiverTypeDecl, bool inAllImportedModules) {
    // Anonymous C-imported types have empty names and never resolve; callers
    // like findTypeAlias forward them during field checking.
    if (name.empty()) return {};
    std::vector<Decl*> decls;

    if (!receiverTypeDecl) {
        for (FunctionDecl* function = currentFunction; function; function = function->parentFunction) {
            if (function->getTypeDecl()) {
                receiverTypeDecl = function->getTypeDecl();
                break;
            }
        }
    }

    // Member lookup materializes interface-provided members (default method implementations
    // are copied into the type). This matters for types that are never otherwise ensured,
    // such as the structs behind builtin types: literals using them perform no lookup.
    if (receiverTypeDecl) ensureInterfaces(*receiverTypeDecl);

    if (receiverTypeDecl) {
        for (auto& decl : receiverTypeDecl->methods) {
            if (auto* functionDecl = llvm::dyn_cast<FunctionDecl>(decl)) {
                // Unqualified for implicit-receiver lookup, qualified for explicit member access.
                // The qualified match matters when the instantiation's methods were registered in a
                // different module's symbol table than the one this lookup searches.
                if (functionDecl->getName() == name || functionDecl->getQualifiedName() == name) {
                    decls.emplace_back(decl);
                }
            } else if (auto* functionTemplate = llvm::dyn_cast<FunctionTemplate>(decl)) {
                if (functionTemplate->getQualifiedName() == name) {
                    decls.emplace_back(decl);
                }
            }
        }

        for (auto& field : receiverTypeDecl->fields) {
            // Unqualified for implicit-receiver lookup, qualified for explicit member access.
            if (field.getName() == name || field.getQualifiedName() == name) {
                decls.emplace_back(&field);
            }
        }

        for (auto* staticConst : receiverTypeDecl->staticConsts) {
            if (staticConst->getName() == name) {
                decls.emplace_back(staticConst);
            }
        }
    }

    if (currentModule->name != "std") {
        appendUnique(decls, currentModule->symbolTable.findInAllScopes(name));
    }

    if (currentSourceFile && !inAllImportedModules) {
        appendUnique(decls, findDeclsInModules(name, currentSourceFile->importedModules));
    } else {
        // Deferred checking runs after all imports are known, so the global scope would also
        // find C headers imported by other files. Those shadow stdlib names by design, so only
        // search the ones this file actually imports.
        llvm::SmallVector<Module*, 8> modules;
        for (Module* module : Module::getAllImportedModules()) {
            if (!module->isCHeaderImport || (currentSourceFile && llvm::is_contained(currentSourceFile->importedModules, module))) {
                modules.push_back(module);
            }
        }
        appendUnique(decls, findDeclsInModules(name, modules));
    }

    appendUnique(decls, findDeclsInModules(name, Module::getStdlibModule()));

    // Overload resolution compares candidate signatures, so all candidates
    // (including losers) get signature-checked on first lookup. Bodies still
    // wait for an actual reference.
    for (Decl* decl : decls) {
        ensureSignature(*decl);
    }

    return decls;
}
