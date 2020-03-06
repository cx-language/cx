#include "clang.h"
#include <memory>
#include <utility>
#pragma warning(push, 0)
#include <clang/Basic/Diagnostic.h>
#include <clang/Basic/DiagnosticIDs.h>
#include <clang/Driver/Compilation.h>
#include <clang/Driver/Driver.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <llvm/ADT/ArrayRef.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/VirtualFileSystem.h> // Fixes "error: invalid use of incomplete type ‘class llvm::vfs::FileSystem’" on GCC.
#include <llvm/Support/raw_ostream.h>
#pragma warning(pop)

int delta::invokeClang(llvm::ArrayRef<const char*> args) {
    auto* diagClient = new clang::TextDiagnosticPrinter(llvm::errs(), new clang::DiagnosticOptions());
    diagClient->setPrefix(llvm::sys::path::filename(args[0]));
    clang::DiagnosticsEngine diags(new clang::DiagnosticIDs(), nullptr, diagClient);
    clang::driver::Driver driver(args[0], llvm::sys::getDefaultTargetTriple(), diags);
    std::unique_ptr<clang::driver::Compilation> compilation(driver.BuildCompilation(args));

    int result = 1;
    if (compilation && !compilation->containsError()) {
        llvm::SmallVector<std::pair<int, const clang::driver::Command*>, 4> failingCommands;
        result = driver.ExecuteCompilation(*compilation, failingCommands);

        for (auto& [commandResult, failingCommand] : failingCommands) {
            if (!result) result = commandResult;

            // If result status is < 0, then the driver command signalled an error.
            // If result status is 70, then the driver command reported a fatal error.
            // On Windows, abort will return an exit code of 3.  In these cases,
            // generate additional diagnostic information if possible.
            bool DiagnoseCrash = commandResult < 0 || commandResult == 70;
#ifdef _WIN32
            DiagnoseCrash |= commandResult == 3;
#endif
            if (DiagnoseCrash) {
                driver.generateCompilationDiagnostics(*compilation, *failingCommand);
                break;
            }
        }
    }

    diags.getClient()->finish();

#ifdef _WIN32
    // Exit status should not be negative on Win32, unless abnormal termination.
    // Once abnormal termination was caught, negative status should not be
    // propagated.
    if (result < 0) result = 1;
#endif

    // If we have multiple failing commands, we return the result of the first
    // failing command.
    return result;
}
