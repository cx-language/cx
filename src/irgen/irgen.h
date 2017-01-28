#pragma once

#include <vector>
#include <llvm/ADT/StringRef.h>
#include "../ast/decl.h"

namespace irgen {
    void compile(const std::vector<Decl>& decls, llvm::StringRef fileName);
}
