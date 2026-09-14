// Tests for cx::compileToC(), the in-memory C* to C compilation entry point
// that is also used by the WebAssembly build of the compiler.
//
// Like the command-line compiler, compileToC() is designed to run once per
// process, so each test case below runs in its own process: the test binary
// takes the case name as its (single) argument. Different cases must not run
// in the same process.
//
// Diagnostics are printed to stdout/stderr by the compiler; only the returned
// status codes and generated code are asserted here.

#include "../../src/driver/compile.h"
#include <iostream>
#include <string>

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

cx::CompileToCOptions testOptions() {
    cx::CompileToCOptions options;
    options.importSearchPaths.push_back(CX_TEST_SOURCE_DIR);
    return options;
}

void testHello() {
    // Hello world compiles to portable C.
    auto hello = cx::compileToC("main.cx", "void main() {\n    println(\"Hello world\");\n}\n", testOptions());
    check(hello.status == 0, "hello world compiles");
    check(hello.cCode.find("Hello world") != std::string::npos, "generated C contains the string literal");
    check(hello.cCode.find("__auto_type") == std::string::npos, "generated C uses no __auto_type GNU extension");
    check(hello.cCode.find("(void)") != std::string::npos, "generated C prototypes empty parameter lists with (void)");
}

void testError() {
    // Erroneous code fails compilation.
    auto broken = cx::compileToC("main.cx", "void main() {\n    undefinedIdentifier;\n}\n", testOptions());
    check(broken.status != 0, "unknown identifier fails compilation");
    check(broken.cCode.empty(), "no C code is generated for erroneous input");
}

void testStruct() {
    // Structs are emitted with their fields.
    auto program = cx::compileToC("main.cx", "struct S { int x; int y; }\nint main() { S s; s.x = 1; s.y = 2; return s.x + s.y; }\n", testOptions());
    check(program.status == 0, "struct program compiles");
    check(program.cCode.find(" y;") != std::string::npos, "generated C contains the struct fields");
}

void testWarning() {
    // Warnings don't fail compilation.
    auto warning = cx::compileToC("main.cx", "int unusedFunction() { return 1; }\nvoid main() {}\n", testOptions());
    check(warning.status == 0, "unused declaration warns but compiles");
}

void testDispatch() {
    // Dispatch (goto-free) mode generates no gotos or labels.
    cx::CompileToCOptions options = testOptions();
    options.dispatchMode = true;
    auto dispatch = cx::compileToC("main.cx",
                                   "int fib(int n) {\n"
                                   "    var a = 0;\n"
                                   "    var b = 1;\n"
                                   "    for (var i = 0; i < n; i++) {\n"
                                   "        var next = a + b;\n"
                                   "        a = b;\n"
                                   "        b = next;\n"
                                   "    }\n"
                                   "    return b;\n"
                                   "}\n"
                                   "void main() {\n"
                                   "    println(fib(10));\n"
                                   "    var b = true && false;\n"
                                   "    println(b ? 1 : 2);\n"
                                   "}\n",
                                   options);
    check(dispatch.status == 0, "dispatch mode compiles control flow");
    check(dispatch.cCode.find("goto") == std::string::npos, "dispatch mode generates no gotos");
    check(dispatch.cCode.find("_cx_pc") != std::string::npos, "dispatch mode uses a program counter");
}

} // namespace

// The WebAssembly API entry point (defined in src/wasm/api.cpp, without the
// Emscripten bindings when built natively).
std::string cxCompileToC(const std::string& source, const std::string& importSearchPath);

void testJson() {
    // The WebAssembly API entry point returns JSON.
    std::string json = cxCompileToC("void main() {\n    println(\"hi\");\n}\n", CX_TEST_SOURCE_DIR);
    check(json.rfind("{\"status\":0,\"cCode\":\"", 0) == 0, "wasm API returns success JSON");
    check(json.find("hi") != std::string::npos, "wasm API JSON contains the generated C code");
    check(json.back() == '}', "wasm API JSON is complete");
    std::string jsonError = cxCompileToC("void main() {\n    nope;\n}\n", CX_TEST_SOURCE_DIR);
    check(jsonError == "{\"status\":1}", "wasm API returns failure JSON for erroneous input");
}

int main(int argc, const char** argv) {
    if (argc != 2) {
        std::cerr << "usage: test_compile_api <hello|error|struct|warning|dispatch|json>\n";
        return 2;
    }

    std::string testCase = argv[1];
    if (testCase == "hello") {
        testHello();
    } else if (testCase == "error") {
        testError();
    } else if (testCase == "struct") {
        testStruct();
    } else if (testCase == "warning") {
        testWarning();
    } else if (testCase == "dispatch") {
        testDispatch();
    } else if (testCase == "json") {
        testJson();
    } else {
        std::cerr << "unknown test case '" << testCase << "'\n";
        return 2;
    }

    if (failures == 0) {
        std::cout << "test case '" << testCase << "' passed.\n";
        return 0;
    }
    std::cerr << failures << " check(s) failed in test case '" << testCase << "'.\n";
    return 1;
}
