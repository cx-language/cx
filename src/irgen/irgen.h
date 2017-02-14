#pragma once

#include <vector>
#include <llvm/ADT/StringRef.h>
#include "../ast/decl.h"

namespace llvm { class Module; }

namespace delta {

namespace irgen {
    llvm::Module& compile(const std::vector<Decl>& decls);
}

}
