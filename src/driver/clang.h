#pragma once

namespace llvm {
template<typename T>
class ArrayRef;
}

namespace delta {

int invokeClang(llvm::ArrayRef<const char*> args);

} // namespace delta
