#pragma once

#include <memory>
#include <string>
#include <unordered_map>
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
class SourceFile;
struct SrcLoc;
struct Type;
class Expr;

using ParserFunction = void(llvm::StringRef filePath, Module& module);

const std::unordered_map<std::string, std::shared_ptr<Module>>& getAllImportedModules();
void setCurrentModule(Module& module);
void setCurrentSourceFile(SourceFile& sourceFile);
void addToSymbolTable(FuncDecl& decl);
void addToSymbolTable(FuncDecl&& decl);
void addToSymbolTable(GenericFuncDecl& decl);
void addToSymbolTable(InitDecl& decl);
void addToSymbolTable(DeinitDecl& decl);
void addToSymbolTable(TypeDecl& decl);
void addToSymbolTable(TypeDecl&& decl);
void addToSymbolTable(VarDecl& decl);
void addToSymbolTable(VarDecl&& decl);
Decl& findDecl(llvm::StringRef name, SrcLoc srcLoc, bool everywhere = false);
llvm::SmallVector<Decl*, 1> findDecls(llvm::StringRef name, bool everywhere = false);
void typecheck(Module& module, llvm::ArrayRef<llvm::StringRef> importSearchPaths,
               ParserFunction& parse);
Type typecheck(Expr& expr);

}
