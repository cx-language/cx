#pragma once

namespace llvm {
    class StringRef;
}

namespace delta {

class FileUnit;

FileUnit parse(llvm::StringRef filePath);

}
