#pragma once

#include <vector>

namespace llvm {
    class StringRef;
    template<typename T> class ArrayRef;
}

namespace delta {

class Decl;
class FuncDecl;
class GenericFuncDecl;
class InitDecl;
class DeinitDecl;
class TypeDecl;
class VarDecl;
class Module;
struct SrcLoc;

void addToSymbolTable(FuncDecl& decl);
void addToSymbolTable(FuncDecl&& decl);
void addToSymbolTable(GenericFuncDecl& decl);
void addToSymbolTable(InitDecl& decl);
void addToSymbolTable(DeinitDecl& decl);
void addToSymbolTable(TypeDecl& decl);
void addToSymbolTable(TypeDecl&& decl);
void addToSymbolTable(VarDecl& decl);
void addToSymbolTable(VarDecl&& decl);
Decl& findInSymbolTable(llvm::StringRef name, SrcLoc srcLoc);
llvm::ArrayRef<Decl*> findInSymbolTable(llvm::StringRef name);
llvm::ArrayRef<Module> getImportedModules();
void typecheck(Module& module, llvm::ArrayRef<llvm::StringRef> includePaths);

}
