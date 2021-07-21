#pragma once

namespace llvm {
template<typename T>
class ArrayRef;
}

namespace cx {

struct Decl;
struct Stmt;
struct Expr;

llvm::raw_ostream& operator<<(llvm::raw_ostream& stream, const Decl& decl);
llvm::raw_ostream& operator<<(llvm::raw_ostream& stream, const Stmt& stmt);
llvm::raw_ostream& operator<<(llvm::raw_ostream& stream, const Expr& expr);
llvm::raw_ostream& operator<<(llvm::raw_ostream& stream, llvm::ArrayRef<Stmt*> stmts);

} // namespace cx
