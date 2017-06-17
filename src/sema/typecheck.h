#pragma once

#include <vector>

namespace llvm {
    class StringRef;
    template<typename T> class ArrayRef;
    template<typename T, unsigned N> class SmallVector;
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
class FileUnit;
struct SrcLoc;
struct Type;
class Expr;

using ParserFunction = void(llvm::StringRef filePath, Module& module);

void setCurrentModule(Module& module);
void setCurrentFileUnit(FileUnit& fileUnit);
void addToSymbolTable(FuncDecl& decl);
void addToSymbolTable(FuncDecl&& decl);
void addToSymbolTable(GenericFuncDecl& decl);
void addToSymbolTable(InitDecl& decl);
void addToSymbolTable(DeinitDecl& decl);
void addToSymbolTable(TypeDecl& decl);
void addToSymbolTable(TypeDecl&& decl);
void addToSymbolTable(VarDecl& decl, bool isGlobal);
void addToSymbolTable(VarDecl&& decl);
Decl& findDecl(llvm::StringRef name, SrcLoc srcLoc, bool everywhere = false);
llvm::SmallVector<Decl*, 1> findDecls(llvm::StringRef name, bool everywhere = false);
void typecheck(Module& module, llvm::ArrayRef<llvm::StringRef> includePaths,
               ParserFunction& parse);
Type typecheck(Expr& expr);

}
