// Tests for cx::renameFile: replacing an existing file must swap in a new
// inode instead of overwriting in place, so rebuilding over a running
// executable leaves the running image intact.

#include "../../src/support/utility.h"
#include <iostream>
#include <string>
#pragma warning(push, 0)
#include <llvm/ADT/SmallString.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/MemoryBuffer.h>
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

void writeFile(llvm::StringRef path, llvm::StringRef content) {
    std::error_code error;
    llvm::raw_fd_ostream file(path, error);
    if (error) {
        std::cerr << "FAIL: couldn't write '" << path.str() << "': " << error.message() << '\n';
        failures++;
        return;
    }
    file << content;
}

std::string readFile(llvm::StringRef path) {
    auto buffer = llvm::MemoryBuffer::getFile(path);
    if (!buffer) {
        std::cerr << "FAIL: couldn't read '" << path.str() << "'\n";
        failures++;
        return "";
    }
    return (*buffer)->getBuffer().str();
}

} // namespace

int main() {
    llvm::SmallString<128> dir;
    if (auto error = llvm::sys::fs::createUniqueDirectory("cx-rename-test", dir)) {
        std::cerr << "FAIL: couldn't create temp directory: " << error.message() << '\n';
        return 1;
    }
    std::string target = (dir + "/target").str();
    std::string source = (dir + "/source").str();

    writeFile(target, "old");
    if (auto error = llvm::sys::fs::setPermissions(target, llvm::sys::fs::all_read | llvm::sys::fs::all_write)) {
        std::cerr << "FAIL: couldn't set target permissions: " << error.message() << '\n';
        return 1;
    }
    llvm::sys::fs::UniqueID targetIDBefore;
    if (auto error = llvm::sys::fs::getUniqueID(target, targetIDBefore)) {
        std::cerr << "FAIL: couldn't stat target: " << error.message() << '\n';
        return 1;
    }

    writeFile(source, "new");
    if (auto error = llvm::sys::fs::setPermissions(source, llvm::sys::fs::all_read | llvm::sys::fs::all_write | llvm::sys::fs::all_exe)) {
        std::cerr << "FAIL: couldn't set source permissions: " << error.message() << '\n';
        return 1;
    }

    cx::renameFile(source, target);

    check(!llvm::sys::fs::exists(source), "source file is removed");
    check(readFile(target) == "new", "target has the new content");

    llvm::sys::fs::UniqueID targetIDAfter;
    if (auto error = llvm::sys::fs::getUniqueID(target, targetIDAfter)) {
        std::cerr << "FAIL: couldn't stat replaced target: " << error.message() << '\n';
        return 1;
    }
    check(targetIDAfter != targetIDBefore, "replaced target has a fresh inode");

    auto permissions = llvm::sys::fs::getPermissions(target);
    check(!permissions.getError() && (*permissions & llvm::sys::fs::all_exe) != llvm::sys::fs::no_perms, "target keeps the executable bit");

    if (failures == 0) {
        std::cout << "rename test passed.\n";
        return 0;
    }
    std::cerr << failures << " check(s) failed.\n";
    return 1;
}
