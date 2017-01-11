#pragma once

#include <vector>

class Decl;
struct FuncDecl;

void addToSymbolTable(const FuncDecl& decl);
void typecheck(std::vector<Decl>& decls);
