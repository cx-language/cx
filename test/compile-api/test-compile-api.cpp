// Tests for cx::compileToC(), the in-memory cx to C compilation entry point
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
#include <optional>
#include <string>
#pragma warning(push, 0)
#include <llvm/Support/JSON.h>
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
                                   "    float[3] a = [1.0, 2.0, 3.0];\n"
                                   "    float[3] c = a * 2.0;\n"
                                   "    println(c[2]);\n"
                                   "    println(a == c ? 1 : 2);\n"
                                   "}\n",
                                   options);
    check(dispatch.status == 0, "dispatch mode compiles control flow");
    check(dispatch.cCode.find("goto") == std::string::npos, "dispatch mode generates no gotos");
    check(dispatch.cCode.find("_cx_pc") != std::string::npos, "dispatch mode uses a program counter");
    check(dispatch.cCode.find("float _array_op") != std::string::npos, "dispatch mode hoists the array op result");
    check(dispatch.cCode.find("_Bool _array_op") != std::string::npos, "dispatch mode hoists the array comparison result");
}

} // namespace

// The WebAssembly API entry points (defined in src/wasm/api.cpp, without the
// Emscripten bindings when built natively).
std::string cxCompileToC(const std::string& source, const std::string& importSearchPath);
std::string cxComplete(const std::string& source, const std::string& importSearchPath, int line, int character);

void testJson() {
    // The WebAssembly API entry point returns JSON. Parsed like the
    // playground (JSON.parse), so key order is insignificant.
    auto success = llvm::json::parse(cxCompileToC("void main() {\n    println(\"hi\");\n}\n", CX_TEST_SOURCE_DIR));
    check(!!success, "wasm API returns valid success JSON");
    const llvm::json::Object* root = success ? success->getAsObject() : nullptr;
    auto status = root ? root->getInteger("status") : std::nullopt;
    check(status && *status == 0, "wasm API success JSON has status 0");
    auto cCode = root ? root->getString("cCode") : std::nullopt;
    check(cCode && cCode->contains("hi"), "wasm API JSON contains the generated C code");
    auto failure = llvm::json::parse(cxCompileToC("void main() {\n    nope;\n}\n", CX_TEST_SOURCE_DIR));
    check(!!failure, "wasm API returns valid failure JSON");
    const llvm::json::Object* errorRoot = failure ? failure->getAsObject() : nullptr;
    auto errorStatus = errorRoot ? errorRoot->getInteger("status") : std::nullopt;
    check(errorStatus && *errorStatus == 1, "wasm API failure JSON has status 1");
    check(errorRoot && !errorRoot->getString("cCode"), "wasm API failure JSON has no C code");
}

void testComplete() {
    // In-scope completions: keywords, locals, top-level decls, stdlib.
    std::string json = cxComplete("int add(int x, int y) {\n"
                                  "    return x + y;\n"
                                  "}\n"
                                  "\n"
                                  "void main() {\n"
                                  "    int result = add(1, 2);\n"
                                  "    pri\n"
                                  "}\n",
                                  CX_TEST_SOURCE_DIR, 6, 7);
    check(json.rfind("{\"status\":0,\"items\":[", 0) == 0, "completion returns success JSON");
    check(json.find("\"label\":\"while\"") != std::string::npos, "completion includes keywords");
    check(json.find("\"label\":\"add\"") != std::string::npos, "completion includes top-level functions");
    check(json.find("\"label\":\"result\"") != std::string::npos, "completion includes enclosing locals");
    check(json.find("\"label\":\"println\"") != std::string::npos, "completion includes stdlib functions");
}

void testCompleteMember() {
    // Member completions after a dot.
    std::string json = cxComplete("struct Point {\n"
                                  "    int x;\n"
                                  "    int y;\n"
                                  "}\n"
                                  "void main() {\n"
                                  "    Point p = Point(0, 0);\n"
                                  "    p.\n"
                                  "}\n",
                                  CX_TEST_SOURCE_DIR, 6, 6);
    check(json.rfind("{\"status\":0,\"items\":[", 0) == 0, "member completion returns success JSON");
    check(json.find("\"label\":\"x\"") != std::string::npos, "member completion includes field x");
    check(json.find("\"label\":\"y\"") != std::string::npos, "member completion includes field y");
    check(json.find("\"label\":\"while\"") == std::string::npos, "member completion lists no keywords");
    check(json.find("\"label\":\"println\"") == std::string::npos, "member completion lists no scope functions");
}

void testCompleteClamp() {
    // Out-of-range cursors clamp instead of failing.
    std::string json = cxComplete("void main() {\n}\n", CX_TEST_SOURCE_DIR, -3, -1);
    check(json.rfind("{\"status\":0,\"items\":[", 0) == 0, "negative cursor returns success JSON");
    check(json.find("\"label\":\"while\"") != std::string::npos, "negative cursor completes from the start");
}

void testCompleteError() {
    // Completion succeeds despite errors in the code.
    std::string json = cxComplete("void main() {\n    nope;\n}\n", CX_TEST_SOURCE_DIR, 1, 8);
    check(json.rfind("{\"status\":0,\"items\":[", 0) == 0, "erroneous code returns success JSON");
    check(json.find("\"label\":\"while\"") != std::string::npos, "erroneous code still completes keywords");
}

int main(int argc, const char** argv) {
    if (argc != 2) {
        std::cerr << "usage: test_compile_api <hello|error|struct|warning|dispatch|json|complete|complete-member|complete-clamp|complete-error>\n";
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
    } else if (testCase == "complete") {
        testComplete();
    } else if (testCase == "complete-member") {
        testCompleteMember();
    } else if (testCase == "complete-clamp") {
        testCompleteClamp();
    } else if (testCase == "complete-error") {
        testCompleteError();
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
