#pragma once

namespace llvm {
template<typename T>
class ArrayRef;
}

namespace cx {

int invokeClang(llvm::ArrayRef<const char*> args);

} // namespace cx
