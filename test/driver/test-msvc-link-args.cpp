// Tests for cx::msvcLinkArgs: -L/-l link settings translated to MSVC
// link.exe form (/LIBPATH plus .lib files).

#include "driver/driver.h"
#include <iostream>
#include <string>
#include <vector>

namespace {

int failures = 0;

void checkEqual(const std::vector<std::string>& actual, const std::vector<std::string>& expected, const char* message) {
    if (actual != expected) {
        std::cerr << "FAIL: " << message << '\n';
        std::cerr << "  expected:";
        for (const auto& arg : expected) std::cerr << " [" << arg << ']';
        std::cerr << "\n  actual:";
        for (const auto& arg : actual) std::cerr << " [" << arg << ']';
        std::cerr << '\n';
        failures++;
    } else {
        std::cout << "PASS: " << message << '\n';
    }
}

} // namespace

int main() {
    checkEqual(cx::msvcLinkArgs({}, {}), {}, "empty inputs produce no args");
    checkEqual(cx::msvcLinkArgs({"E:/raylib/lib"}, {"raylib"}), {"/LIBPATH:E:/raylib/lib", "raylib.lib"}, "issue example translates");
    checkEqual(cx::msvcLinkArgs({"."}, {"win32_bridge", "user32"}), {"/LIBPATH:.", "win32_bridge.lib", "user32.lib"}, "build.cx example translates");
    checkEqual(cx::msvcLinkArgs({"a", "b/c"}, {}), {"/LIBPATH:a", "/LIBPATH:b/c"}, "search paths keep order");
    checkEqual(cx::msvcLinkArgs({}, {"a", "b"}), {"a.lib", "b.lib"}, "libraries keep order");
    checkEqual(cx::msvcLinkArgs({}, {"libfoo"}), {"libfoo.lib"}, "no lib-prefix handling");
    checkEqual(cx::msvcLinkArgs({}, {"foo.lib"}), {"foo.lib"}, ".lib name passes through");
    checkEqual(cx::msvcLinkArgs({}, {"E:/libs/x.lib"}), {"E:/libs/x.lib"}, ".lib path passes through");
    checkEqual(cx::msvcLinkArgs({}, {"helper.obj"}), {"helper.obj"}, ".obj file passes through");
    checkEqual(cx::msvcLinkArgs({}, {"foo.a"}), {"foo.a"}, "foreign extension passes through for the linker to diagnose");
    checkEqual(cx::msvcLinkArgs({}, {"dir/foo"}), {"dir/foo"}, "extensionless path passes through");
    checkEqual(cx::msvcLinkArgs({}, {"./foo"}), {"./foo"}, "relative extensionless path passes through");
    checkEqual(cx::msvcLinkArgs({"lib dir"}, {"my lib"}), {"/LIBPATH:lib dir", "my lib.lib"}, "paths with spaces pass through");

    if (failures == 0) {
        std::cout << "msvc-link-args test passed.\n";
        return 0;
    }
    std::cerr << failures << " check(s) failed.\n";
    return 1;
}
