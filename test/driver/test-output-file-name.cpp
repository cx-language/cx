// Tests for cx::withExecutableExtension: directory-build output names gain
// .exe on Windows unless already present.

#include "driver/driver.h"
#include <iostream>
#include <string>

namespace {

int failures = 0;

void checkEqual(const std::string& actual, const std::string& expected, const char* message) {
    if (actual != expected) {
        std::cerr << "FAIL: " << message << '\n';
        std::cerr << "  expected: [" << expected << "]\n";
        std::cerr << "  actual:   [" << actual << "]\n";
        failures++;
    } else {
        std::cout << "PASS: " << message << '\n';
    }
}

} // namespace

int main() {
    checkEqual(cx::withExecutableExtension("myproject", true), "myproject.exe", "project name gains .exe on Windows");
    checkEqual(cx::withExecutableExtension("myproject", false), "myproject", "project name untouched elsewhere");
    checkEqual(cx::withExecutableExtension("myproject.exe", true), "myproject.exe", ".exe name passes through");
    checkEqual(cx::withExecutableExtension("MYPROJECT.EXE", true), "MYPROJECT.EXE", "extension match is case-insensitive");
    checkEqual(cx::withExecutableExtension("my.project", true), "my.project.exe", "dotted name gains .exe");

    if (failures == 0) {
        std::cout << "output-file-name test passed.\n";
        return 0;
    }
    std::cerr << failures << " check(s) failed.\n";
    return 1;
}
