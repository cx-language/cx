#pragma once

namespace llvm {
class StringRef;
}

namespace delta {

struct Type;

class TypeResolver {
public:
    Type resolve(Type type) const;

protected:
    virtual Type resolveTypePlaceholder(llvm::StringRef name) const = 0;
};

}
