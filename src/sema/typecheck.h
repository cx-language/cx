#pragma once

#include <vector>

class Decl;
struct FuncDecl;
struct InitDecl;
struct TypeDecl;

void addToSymbolTable(const FuncDecl& decl);
void addToSymbolTable(const InitDecl& decl);
void addToSymbolTable(const TypeDecl& decl);
void typecheck(std::vector<Decl>& decls);
