#pragma once

#include <vector>

namespace llvm { class StringRef; }

namespace delta {

class Decl;
struct FuncDecl;
struct InitDecl;
struct TypeDecl;
struct VarDecl;
struct SrcLoc;

void addToSymbolTable(const FuncDecl& decl);
void addToSymbolTable(const InitDecl& decl);
void addToSymbolTable(const TypeDecl& decl);
void addToSymbolTable(const VarDecl& decl);
Decl& findInSymbolTable(llvm::StringRef name, SrcLoc srcLoc);
void typecheck(std::vector<Decl>& decls);

}
