#pragma once

#include <llvm/IR/IRBuilder.h>

namespace delta {

class Module;
class Expr;
struct Type;

namespace irgen {
    llvm::Module& compile(const Module& module);
    llvm::Value* codegen(const Expr& expr);
    llvm::Type* toIR(Type type);
    llvm::Module& getIRModule();
    llvm::IRBuilder<>& getBuilder();
    llvm::LLVMContext& getContext();
}

}
