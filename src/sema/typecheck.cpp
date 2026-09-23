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
    deferTypechecking(instantiation);
    return instantiation;
}

static std::error_code importModuleSourcesInDirectoryRecursively(const llvm::Twine& directoryPath, Module& module, const CompileOptions& options) {
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
    typecheckModule(*module, *packageOptions);
    return *module;
}

void Typechecker::deferTypechecking(Decl* decl) {
    for (auto existing : declsToTypecheck) {
        if (existing == decl) {
            return;
        }
    }
    declsToTypecheck.push_back(decl);
}

void Typechecker::postProcess() {
    llvm::SaveAndRestore setPostProcessing(isPostProcessing, true);

    while (!declsToTypecheck.empty()) {
        auto currentDeclsToTypecheck = std::move(declsToTypecheck);

        for (auto* decl : currentDeclsToTypecheck) {
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
                typecheckTypeDecl(*llvm::cast<TypeDecl>(decl));
                break;
            case DeclKind::EnumDecl:
                typecheckEnumDecl(*llvm::cast<EnumDecl>(decl));
                break;
            default:
                llvm_unreachable("invalid deferred decl");
            }
        }
    }
}

static void checkUnusedDeclsInModule(const Module& module) {
    for (auto& sourceFile : module.sourceFiles) {
        for (auto& decl : sourceFile.topLevelDecls) {
            if (decl->isReferenced()) continue;

            if (decl->isFunctionDecl() || decl->isFunctionTemplate()) {
                if (decl->isMain()) continue;
                // Test functions are entry points for `cx test`, like main is for `cx run`.
                if (auto* functionDecl = llvm::dyn_cast<FunctionDecl>(decl); functionDecl && functionDecl->isTest) continue;
                if (auto* functionTemplate = llvm::dyn_cast<FunctionTemplate>(decl);
                    functionTemplate && functionTemplate->functionDecl->isTest) {
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

void Typechecker::typecheckModule(Module& module, const CompileOptions& packageOptions) {
    llvm::SaveAndRestore restoreModule(currentModule);
    llvm::SaveAndRestore restoreSourceFile(currentSourceFile);
    llvm::SaveAndRestore restoreOptions(options, packageOptions);

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

    // Typecheck implemented interfaces so that inherited methods and fields are added to the implementing type before they're referenced.
    for (auto& sourceFile : module.sourceFiles) {
        for (auto& decl : sourceFile.topLevelDecls) {
            currentModule = &module;
            currentSourceFile = &sourceFile;

            if (auto typeDecl = llvm::dyn_cast<TypeDecl>(decl)) {
                llvm::StringMap<GenericArg> genericArgs = {{"This", GenericArg(typeDecl->getType())}};

                for (Type interface : typeDecl->interfaces) {
                    try {
                        typecheckType(interface, typeDecl->accessLevel);
                    } catch (const CompileError& error) {
                        error.report();
                    }
                    if (!interface.getDecl()) continue;

                    // Enums have no fields, so an interface field requirement fails conformance instead.
                    if (!typeDecl->isEnumDecl()) {
                        std::vector<FieldDecl> inheritedFields;

                        for (auto& field : interface.getDecl()->fields) {
                            auto duplicate = llvm::find_if(typeDecl->fields, [&](const FieldDecl& f) { return f.getName() == field.getName(); });
                            if (duplicate != typeDecl->fields.end()) {
                                WARN(duplicate->getLocation(),
                                     "field '" << field.getName() << "' duplicates inherited field from interface '" << interface.getDecl()->getName() << "'");
                            }
                            inheritedFields.push_back(field.instantiate(genericArgs, *typeDecl));
                        }

                        typeDecl->fields.insert(typeDecl->fields.begin(), inheritedFields.begin(), inheritedFields.end());
                    }

                    for (auto member : interface.getDecl()->methods) {
                        auto methodDecl = llvm::cast<MethodDecl>(member);
                        if (methodDecl->body) {
                            auto copy = methodDecl->instantiate(genericArgs, {}, *typeDecl);
                            currentModule->addToSymbolTable(*copy);
                            typeDecl->addMethod(copy);
                        }
                    }
                }

                // The parser-generated constructor misses inherited fields, so
                // regenerate it now that they're added. Duplicate field names
                // can't form parameters; leave the parser version in that case.
                if (typeDecl->isStruct() && !typeDecl->interfaces.empty()) {
                    llvm::SmallDenseSet<llvm::StringRef, 8> fieldNames;
                    bool hasDuplicates = false;
                    for (auto& field : typeDecl->fields) {
                        if (!fieldNames.insert(field.getName()).second) {
                            hasDuplicates = true;
                            break;
                        }
                    }
                    if (!hasDuplicates) {
                        auto& methods = typeDecl->methods;
                        auto newEnd = llvm::remove_if(methods, [](Decl* decl) {
                            auto* ctor = llvm::dyn_cast<ConstructorDecl>(decl);
                            return ctor && ctor->isAutogenerated;
                        });
                        if (newEnd != methods.end()) {
                            methods.erase(newEnd, methods.end());
                            typeDecl->addAutogeneratedConstructor();
                        }
                    }
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

Decl* Typechecker::findDecl(llvm::StringRef name, Location location, Location endLocation) const {
    ASSERT(!name.empty());

    if (Decl* match = findDeclInModules(name, location, currentModule)) {
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
        return match;
    }

    if (Decl* match = findDeclInModules(name, location, Module::getStdlibModule())) {
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

std::vector<Decl*> Typechecker::findDecls(llvm::StringRef name, TypeDecl* receiverTypeDecl, bool inAllImportedModules) const {
    ASSERT(!name.empty());
    std::vector<Decl*> decls;

    if (!receiverTypeDecl) {
        for (FunctionDecl* function = currentFunction; function; function = function->parentFunction) {
            if (function->getTypeDecl()) {
                receiverTypeDecl = function->getTypeDecl();
                break;
            }
        }
    }

    if (receiverTypeDecl) {
        for (auto& decl : receiverTypeDecl->methods) {
            if (auto* functionDecl = llvm::dyn_cast<FunctionDecl>(decl)) {
                if (functionDecl->getName() == name) {
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
        appendUnique(decls, findDeclsInModules(name, Module::getAllImportedModules()));
    }

    appendUnique(decls, findDeclsInModules(name, Module::getStdlibModule()));

    return decls;
}
