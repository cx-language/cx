#pragma once

#include <memory>
#include <llvm/Support/MemoryBuffer.h>

namespace llvm {
    class StringRef;
}

namespace delta {

class FileUnit;
class Expr;

FileUnit parse(llvm::StringRef filePath);
std::unique_ptr<Expr> parseExpr(std::unique_ptr<llvm::MemoryBuffer> input);

}
