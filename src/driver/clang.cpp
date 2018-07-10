#include "clang.h"
#include <memory>
#include <utility>
#pragma warning(push, 0)
#include <clang/Basic/Diagnostic.h>
#include <clang/Basic/DiagnosticIDs.h>
#include <clang/Basic/VirtualFileSystem.h> // Fixes "Member access into incomplete type 'clang::vfs::FileSystem'".
#include <clang/Driver/Compilation.h>
#include <clang/Driver/Driver.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <llvm/ADT/ArrayRef.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/raw_ostream.h>
#pragma warning(pop)

int delta::invokeClang(llvm::ArrayRef<const char*> args) {
    auto* diagClient = new clang::TextDiagnosticPrinter(llvm::errs(), nullptr);
    diagClient->setPrefix(llvm::sys::path::filename(args[0]));
    clang::DiagnosticsEngine diags(new clang::DiagnosticIDs(), nullptr, diagClient);
    clang::driver::Driver driver(args[0], llvm::sys::getDefaultTargetTriple(), diags);
    std::unique_ptr<clang::driver::Compilation> compilation(driver.BuildCompilation(args));
    if (compilation) {
        llvm::SmallVector<std::pair<int, const clang::driver::Command*>, 4> failingCommands;
        return driver.ExecuteCompilation(*compilation, failingCommands);
    }
    diags.getClient()->finish();
    return 1;
}
