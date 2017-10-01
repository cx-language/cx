#include <string>
#include <system_error>
#include <vector>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/raw_ostream.h>
#include "package-manager.h"

using namespace delta;

std::vector<std::string> delta::getSourceFiles(llvm::StringRef packageRoot) {
    std::vector<std::string> sourceFiles;
    std::error_code error;

    for (llvm::sys::fs::recursive_directory_iterator it(packageRoot, error), end;
         it != end; it.increment(error)) {
        if (error) {
            llvm::errs() << error.message() << '\n';
            break;
        }

        if (llvm::sys::path::extension(it->path()) == ".delta") {
            sourceFiles.push_back(it->path());
        }
    }

    return sourceFiles;
}
