// Tests for cx::renameFile: replacing an existing file must swap in a new
// inode instead of overwriting in place, so rebuilding over a running
// executable leaves the running image intact.

#include "../../src/support/utility.h"
#include <cstdint>
#include <iostream>
#include <optional>
#include <string>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif
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

std::optional<llvm::sys::fs::UniqueID> getFileID(const std::string& path) {
#ifdef _WIN32
    // llvm::sys::fs::getUniqueID hashes the canonical path on Windows, so it
    // can't tell that a rename swapped the file behind a path. Compare the
    // volume serial + file index instead.
    int wideSize = MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, nullptr, 0);
    std::wstring widePath(wideSize, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, widePath.data(), wideSize);
    HANDLE handle =
        CreateFileW(widePath.c_str(), 0, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (handle == INVALID_HANDLE_VALUE) {
        std::cerr << "FAIL: couldn't open '" << path << "'\n";
        failures++;
        return std::nullopt;
    }
    BY_HANDLE_FILE_INFORMATION info;
    if (!GetFileInformationByHandle(handle, &info)) {
        std::cerr << "FAIL: couldn't stat '" << path << "'\n";
        failures++;
        CloseHandle(handle);
        return std::nullopt;
    }
    CloseHandle(handle);
    uint64_t index = (uint64_t(info.nFileIndexHigh) << 32) | info.nFileIndexLow;
    return llvm::sys::fs::UniqueID(info.dwVolumeSerialNumber, index);
#else
    llvm::sys::fs::UniqueID id;
    if (auto error = llvm::sys::fs::getUniqueID(path, id)) {
        std::cerr << "FAIL: couldn't stat '" << path << "': " << error.message() << '\n';
        failures++;
        return std::nullopt;
    }
    return id;
#endif
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
    auto targetIDBefore = getFileID(target);
    if (!targetIDBefore) return 1;

    writeFile(source, "new");
    if (auto error = llvm::sys::fs::setPermissions(source, llvm::sys::fs::all_read | llvm::sys::fs::all_write | llvm::sys::fs::all_exe)) {
        std::cerr << "FAIL: couldn't set source permissions: " << error.message() << '\n';
        return 1;
    }

    cx::renameFile(source, target);

    check(!llvm::sys::fs::exists(source), "source file is removed");
    check(readFile(target) == "new", "target has the new content");

    auto targetIDAfter = getFileID(target);
    if (!targetIDAfter) return 1;
    check(*targetIDAfter != *targetIDBefore, "replaced target has a fresh inode");

    auto permissions = llvm::sys::fs::getPermissions(target);
    check(!permissions.getError() && (*permissions & llvm::sys::fs::all_exe) != llvm::sys::fs::no_perms, "target keeps the executable bit");

    if (failures == 0) {
        std::cout << "rename test passed.\n";
        return 0;
    }
    std::cerr << failures << " check(s) failed.\n";
    return 1;
}
