#include "ast/demangle.h"
#include "ast/mangle.h"
#include "ast/decl.h"
#include "ast/module.h"
#include <cassert>
#include <string>

using namespace cx;

static void checkDemangle(llvm::StringRef symbol, llvm::StringRef expected) {
    std::string demangled = demangle(symbol);
    assert(demangled == expected);
}

static void checkUnescape(llvm::StringRef escaped, llvm::StringRef expected) {
    std::string out;
    assert(unescapeMangledIdentifier(escaped, out));
    assert(out == expected);
    assert(escapeMangledIdentifier(out) == escaped);
}

int main() {
    assert(escapeMangledIdentifier("abc123") == "abc123");
    assert(escapeMangledIdentifier("") == "");
    checkUnescape("a__b", "a_b");
    checkUnescape("____lambda0", "__lambda0");
    checkUnescape("my_2Dheader__h", "my-header_h");
    checkUnescape("a_2Eb", "a.b");
    checkUnescape("_C3_BC", "ü");

    std::string out;
    assert(!unescapeMangledIdentifier("_", out));
    assert(!unescapeMangledIdentifier("a_", out));
    assert(!unescapeMangledIdentifier("_4", out));
    assert(!unescapeMangledIdentifier("_ZZ", out));
    assert(!unescapeMangledIdentifier("a-b", out));
    assert(!unescapeMangledIdentifier("a b", out));

    assert(stripAsmLabelMarker("foo") == "foo");
    assert(stripAsmLabelMarker("\01foo") == "foo");
    assert(stripAsmLabelMarker("") == "");

    checkDemangle("main", "main");
    checkDemangle("foo", "foo");
    checkDemangle("", "");
    checkDemangle("\01FOO", "FOO");
    checkDemangle("_CX1G4main7counterE", "main.counter (global)");
    checkDemangle("_CX1N4main3addEM3std5int322_M3std5int32M3std5int32", "main.add(std.int32, std.int32) -> std.int32");
    checkDemangle("_CX1N4main4moveE4void1_L1xM3std5int32", "main.move(x: std.int32) -> void");
    checkDemangle("_CX1N4maino2eqEM3std4bool2_RM4main3BoxRM4main3Box", "main.==(main.Box&, main.Box&) -> std.bool");
    checkDemangle("_CX1N4mainox6_2A_2AE4void0_", "main.**() -> void");
    checkDemangle("_CX1N4maino2zzE4void0_", "main.zz() -> void");
    checkDemangle("_CX1N4main3fooINn3_EE4void0_", "main.foo<-3>() -> void");
    checkDemangle("_CX1N4main3fooVE4void0_", "main.foo() -> void [pack]");
    checkDemangle("_CX1N4main3foovE4void1_M3std5int32", "main.foo(std.int32, ...) -> void");
    checkDemangle("_CX1N4main3fooE4void1_Fv1_M3std5int324void", "main.foo((std.int32, ...) -> void) -> void");
    // The "v" sits up front so nesting stays unambiguous: outer-variadic and
    // inner-variadic spell different symbols and demangle differently.
    checkDemangle("_CX1N4main3fooE4void1_Fv1_PM3std4charF1_M3std5int324void",
                  "main.foo((std.char*, ...) -> (std.int32) -> void) -> void");
    checkDemangle("_CX1N4main3fooE4void1_F1_PM3std4charFv1_M3std5int324void",
                  "main.foo((std.char*) -> (std.int32, ...) -> void) -> void");
    checkDemangle("_CX1N4main3fooE4void1_KAM3std5int32", "main.foo(const std.int32[*]) -> void");
    checkDemangle("_CX1N4main3fooE4void1_T1_1aM3std5int32", "main.foo((std.int32 a)) -> void");
    checkDemangle("_CX1N4main3fooE4void1_OKM3std5int32", "main.foo(const std.int32?) -> void");
    checkDemangle("_CX1N4main8fixedSumEM3std5int321_5ArrayIM3std5int32N3_E", "main.fixedSum(std.int32[3]) -> std.int32");
    checkDemangle("_CX1N4mainM4main7WrapperIM3std5int32E3getEM3std5int320_", "main.Wrapper<std.int32>.get() -> std.int32");
    checkDemangle("_CX1N4main11____lambda0EM3std5int321_M3std5int32C1_M3std5int32",
                  "main.__lambda0(std.int32) [captures: std.int32] -> std.int32");
    checkDemangle("_CX1N4main3fooE4void1_05_", "main.foo(<anonymous C type #5>) -> void");

    // Hostile input is rejected, never crashes: 300-deep nesting exceeds the
    // depth cap, and the 25-digit length overflows checked arithmetic.
    std::string deepNest = "_CX1N4main3fooE4void1_" + std::string(300, 'P') + "4void";
    checkDemangle(deepNest, deepNest);
    std::string longLength = "_CX1N4main3fooE4void1_9999999999999999999999999";
    checkDemangle(longLength, longLength);

    // Malformed symbols pass through unchanged instead of crashing.
    checkDemangle("_CX1", "_CX1");
    checkDemangle("_CX1N", "_CX1N");
    checkDemangle("_CX1GARBAGE", "_CX1GARBAGE");
    checkDemangle("_CX1N4mai", "_CX1N4mai");
    checkDemangle("_CX1N4main3fooE4void0_X", "_CX1N4main3fooE4void0_X");
    checkDemangle("_CX1N1-E4void0_", "_CX1N1-E4void0_");
    checkDemangle("_CX1N4main3fooE4void2_M3std5int32", "_CX1N4main3fooE4void2_M3std5int32");

    assert(demangleLine("no symbols here") == "no symbols here");
    assert(demangleLine("./a.out(_CX1N4main5innerE4void0_ + 0x1a) [0x0]")
           == "./a.out(main.inner() -> void + 0x1a) [0x0]");
    assert(demangleLine("_CX1G4main7counterE and _CX1GARBAGE") == "main.counter (global) and _CX1GARBAGE");

    // Types without declarations (as here, with no parsed stdlib) mangle bare.
    Type voidType = BasicType::get("void", {});
    Type intType = BasicType::get("int32", {});
    assert(mangleType(voidType) == "4void");
    assert(mangleType(intType) == "5int32");
    assert(mangleType(intType.withMutability(Mutability::Const)) == "K5int32");
    Type arrayType = BasicType::get("Array", {intType, GenericArg::fromInt(-1, Location())});
    assert(mangleType(arrayType) == "5ArrayI5int32Nn1_E");
    Type anonType = BasicType::get("", {});
    std::string anonMangled = mangleType(anonType);
    assert(anonMangled.starts_with("0") && anonMangled.ends_with("_") && anonMangled.size() > 2);
    assert(mangleType(BasicType::get("", {})) != anonMangled);

    Module mainModule("main");
    Location location;
    // Unlisted operators mangle by escaped spelling instead of crashing.
    FunctionDecl power(FunctionProto("**", {}, voidType), {}, AccessLevel::None, mainModule, location);
    assert(mangleFunctionDecl(power) == "_CX1N4mainox6_2A_2AE4void0_");
    FunctionDecl callee(FunctionProto("callee", {}, voidType), {}, AccessLevel::None, mainModule, location);
    assert(mangleFunctionDecl(callee) == "_CX1N4main6calleeE4void0_");
    FunctionDecl externDecl(FunctionProto("puts", {}, voidType, false, true), {}, AccessLevel::None, mainModule, location);
    assert(mangleFunctionDecl(externDecl) == "puts");
    FunctionDecl entry(FunctionProto("main", {}, voidType), {}, AccessLevel::None, mainModule, location);
    entry.isEntryPoint = true;
    assert(mangleFunctionDecl(entry) == "main");

    VarDecl global(intType, "x", nullptr, nullptr, AccessLevel::None, mainModule, location);
    assert(mangleGlobalVar(global) == "_CX1G4main1xE");
    Module cModule("some_h");
    cModule.isCHeaderImport = true;
    VarDecl cGlobal(intType, "errno", nullptr, nullptr, AccessLevel::None, cModule, location);
    assert(mangleGlobalVar(cGlobal) == "errno");
}
