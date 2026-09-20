#include "typecheck.h"
#pragma warning(push, 0)
#include <llvm/ADT/SmallPtrSet.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/SaveAndRestore.h>
#pragma warning(pop)
#include "../ast/module.h"
#include "../build/config.h"
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

    for (llvm::sys::fs::recursive_directory_iterator it(directoryPath, error), end; it != end; it.increment(error)) {
        if (error) break;
        if (llvm::sys::path::extension(it->path()) == ".cx") {
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

llvm::ErrorOr<const Module&> Typechecker::importModule(SourceFile* importer, const BuildConfig* config, llvm::StringRef moduleName) {
    auto it = Module::getAllImportedModulesMap().find(moduleName);
    if (it != Module::getAllImportedModulesMap().end()) {
        if (importer) importer->addImportedModule(it->second);
        return *it->second;
    }

    auto module = new Module(moduleName.str());
    std::error_code error;

    if (config) {
        for (auto& dependency : config->declaredDependencies) {
            if (dependency.package == moduleName) {
                error = importModuleSourcesInDirectoryRecursively(dependency.getFileSystemPath(), *module, options);
                goto done;
            }
        }
    }

    for (llvm::StringRef importPath : options.importSearchPaths) {
        auto modulePath = (importPath + "/" + moduleName).str();
        if (llvm::sys::fs::is_directory(modulePath)) {
            error = importModuleSourcesInDirectoryRecursively(modulePath, *module, options);
            goto done;
        }
    }

done:
    if (error) return error;
    if (importer) importer->addImportedModule(module);
    Module::getAllImportedModulesMap()[module->name] = module;
    typecheckModule(*module, nullptr);
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
                WARN(decl->getLocation(), "unused declaration '" << decl->getName() << "'");
            }
        }
    }
}

// Warns about unused declarations after the whole program is typechecked, so
// that references from importing modules are seen. Runs dependencies first.
static void checkUnusedDeclsTransitive(const Module& module, llvm::SmallPtrSetImpl<const Module*>& visited) {
    if (!visited.insert(&module).second) return;
    for (auto* imported : module.getImportedModules()) {
        if (imported->name != "std" && !imported->isCHeaderImport) {
            checkUnusedDeclsTransitive(*imported, visited);
        }
    }
    checkUnusedDeclsInModule(module);
}

void Typechecker::checkUnusedDecls(const Module& mainModule) {
    if (options.noUnusedWarnings) return;
    llvm::SmallPtrSet<const Module*, 8> visited;
    checkUnusedDeclsTransitive(mainModule, visited);
}

void Typechecker::typecheckModule(Module& module, const BuildConfig* config) {
    llvm::SaveAndRestore restoreModule(currentModule);
    llvm::SaveAndRestore restoreSourceFile(currentSourceFile);

    auto stdModule = importModule(nullptr, nullptr, "std");
    if (!stdModule) {
        ABORT("couldn't import the standard library: " << stdModule.getError().message());
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
                typecheckImportDecl(*llvm::cast<ImportDecl>(decl), config);
                postProcess();
            } catch (const CompileError& error) {
                error.report();
            }
        }
    }

    // Typecheck implemented interfaces so that inherited methods and fields are added to the implementing type before they're referenced.
    for (auto& sourceFile : module.sourceFiles) {
        for (auto& decl : sourceFile.topLevelDecls) {
            currentModule = &module;
            currentSourceFile = &sourceFile;

            if (auto typeDecl = llvm::dyn_cast<TypeDecl>(decl)) {
                llvm::StringMap<Type> genericArgs = {{"This", typeDecl->getType()}};

                for (Type interface : typeDecl->interfaces) {
                    try {
                        typecheckType(interface, typeDecl->accessLevel);
                    } catch (const CompileError& error) {
                        error.report();
                    }
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

                    for (auto member : interface.getDecl()->methods) {
                        auto methodDecl = llvm::cast<MethodDecl>(member);
                        if (methodDecl->body) {
                            auto copy = methodDecl->instantiate(genericArgs, {}, *typeDecl);
                            currentModule->addToSymbolTable(*copy);
                            typeDecl->addMethod(copy);
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
                    typecheckTopLevelDecl(*decl, config);
                    postProcess();
                } catch (const CompileError& error) {
                    error.report();
                }
            }
        }
    }
}

static llvm::SmallVector<Decl*, 8> findDeclsInModules(llvm::StringRef name, llvm::ArrayRef<Module*> modules, bool topLevelOnly = false) {
    ASSERT(!name.empty());
    llvm::SmallVector<Decl*, 8> decls;

    for (auto& module : modules) {
        auto matches = topLevelOnly ? module->symbolTable.findInTopLevelScope(name) : module->symbolTable.findFirst(name);
        llvm::append_range(decls, matches);
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
    } else if (llvm::all_of(decls, [](Decl* decl) { return decl->getModule() && decl->getModule()->name.ends_with("_h"); })) {
        // For duplicate definitions in C headers, return the last definition.
        // TODO: This should only work for declarations of the same thing.
        return decls.back(); // For duplicate definitions in C headers, return the last definition.
    } else {
        ERROR(location, "ambiguous reference to '" << name << "'");
    }
}

Decl* Typechecker::findDecl(llvm::StringRef name, Location location) const {
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
        }
    }

    if (Decl* match = findDeclInModules(name, location, Module::getStdlibModule())) {
        return match;
    }

    if (Decl* match = findDeclInModules(name, location, currentSourceFile->importedModules)) {
        return match;
    }

    ERROR(location, "unknown identifier '" << name << "'");
}

static void appendUnique(std::vector<Decl*>& target, llvm::ArrayRef<Decl*> source) {
    for (auto& element : source) {
        // TODO: Should this ever be false? I.e. should the same decl ever be in multiple different modules?
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
            // TODO: Only one comparison should be needed.
            if (field.getName() == name || field.getQualifiedName() == name) {
                decls.emplace_back(&field);
            }
        }
    }

    if (currentModule->name != "std") {
        appendUnique(decls, findDeclsInModules(name, currentModule, false));
        appendUnique(decls, findDeclsInModules(name, currentModule, true)); // HACK, TODO: one find function should be enough
    }

    appendUnique(decls, findDeclsInModules(name, Module::getStdlibModule()));

    if (currentSourceFile && !inAllImportedModules) {
        appendUnique(decls, findDeclsInModules(name, currentSourceFile->importedModules));
    } else {
        appendUnique(decls, findDeclsInModules(name, Module::getAllImportedModules()));
    }

    return decls;
}
