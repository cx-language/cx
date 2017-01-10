#pragma once

#include <vector>

class Decl;
class FuncDecl;

void addToSymbolTable(const FuncDecl& decl);
void typecheck(std::vector<Decl>& decls);
