#pragma once

namespace llvm { class StringRef; }

namespace delta {

void importCHeader(llvm::StringRef headerName);

}
