#pragma once

#include <vector>

class Decl;
struct FuncDecl;
struct TypeDecl;

void addToSymbolTable(const FuncDecl& decl);
void addToSymbolTable(const TypeDecl& decl);
void typecheck(std::vector<Decl>& decls);
