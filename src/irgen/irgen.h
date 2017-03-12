#pragma once

#include <vector>
#include <memory>

namespace llvm { class Module; }

namespace delta {

class Decl;

namespace irgen {
    llvm::Module& compile(const std::vector<std::unique_ptr<Decl>>& decls);
}

}
