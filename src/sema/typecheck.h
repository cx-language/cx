#pragma once

#include <vector>

namespace llvm { class StringRef; }
class Decl;
struct FuncDecl;
struct InitDecl;
struct TypeDecl;

void addToSymbolTable(const FuncDecl& decl);
void addToSymbolTable(const InitDecl& decl);
void addToSymbolTable(const TypeDecl& decl);
Decl& findInSymbolTable(llvm::StringRef name);
void typecheck(std::vector<Decl>& decls);
