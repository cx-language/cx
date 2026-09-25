#pragma once

#include <string>

// Name mangling, version _CX1.
//
// Mangles cx functions and globals to unique linker symbols, and cx types to
// unique C struct names. Output is [A-Za-z0-9_] only, so it is valid in LLVM
// IR, C, and object files. Identifiers are escaped (alnum as-is, '_' as
// "__", any other byte as "_XX" uppercase hex) and length-prefixed by
// escaped length, so decoding is unambiguous. See demangle.h for the parser.
//
// Grammar:
//
//   mangled      := "_CX1" ( function | global )
//   function     := "N" identifier                       // module
//                   [ "M" identifier identifier generic-args ]  // receiver: module, name, args
//                   ( identifier | operator )
//                   generic-args
//                   [ "V" ]                               // variadic-pack instantiation
//                   [ "v" ]                               // C-style variadic (...)
//                   "E"
//                   type                                  // return type
//                   arity param*                          // user parameters
//                   [ "C" arity type* ]                   // lambda captures
//   param        := [ "L" identifier ] type              // optional public label
//   global       := "G" identifier identifier "E"        // module, name
//   identifier   := decimal-length escaped-bytes
//   arity        := decimal-count "_"
//   generic-args := [ "I" generic-arg* "E" ]             // omitted when empty
//   generic-arg  := "N" [ "n" ] digits "_"               // int; "n" marks negative
//                 | type
//   operator     := "o" identifier                       // short code, e.g. "o2pl" for "+"
//                 | "ox" identifier                      // escaped spelling for unlisted operators
//   type         := [ "K" ] type-core                     // "K" marks const
//   type-core    := "M" identifier identifier generic-args  // declared type: module, name, args
//                 | identifier generic-args              // builtin, or "0<index>_" for anonymous C types
//                 | "O" type                              // Optional
//                 | "P" type                              // pointer
//                 | "R" type                              // reference (borrow)
//                 | "A" type                              // array pointer T[*]
//                 | "T" arity ( identifier type )*        // anonymous struct: named fields
//                 | "F" [ "v" ] arity type* type         // function type: variadic, params, return
//
// The function-type "v" sits up front so nested function types stay
// unambiguous: a trailing marker could not be told apart from an inner
// type's own marker.
//
// Unmangled exceptions: extern functions keep their C name, asmLabel
// functions mangle to "\01" + label (LLVM exact-symbol marker), and the
// single entry-point main keeps "main". C-imported globals keep their name.

namespace llvm {
class raw_string_ostream;
class StringRef;
} // namespace llvm

namespace cx {

struct FunctionDecl;
struct Type;
struct VarDecl;

bool isAsciiAlnum(char ch);
void mangleType(llvm::raw_string_ostream& stream, Type type);
std::string mangleType(Type type);
std::string mangleFunctionDecl(const FunctionDecl& functionDecl);
std::string mangleGlobalVar(const VarDecl& varDecl);
std::string escapeMangledIdentifier(llvm::StringRef name);
// Inverse of escapeMangledIdentifier. Returns false on malformed input.
bool unescapeMangledIdentifier(llvm::StringRef escaped, std::string& out);
// Strips a leading "\01" LLVM exact-symbol marker, if present.
llvm::StringRef stripAsmLabelMarker(llvm::StringRef symbol);

} // namespace cx
