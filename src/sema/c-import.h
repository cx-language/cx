#pragma once

namespace llvm {
class StringRef;
} // namespace llvm

namespace cx {

struct SourceFile;
struct SourceLocation;
struct CompileOptions;

/// Returns true if the header was found and successfully imported.
bool importCHeader(SourceFile& importer, llvm::StringRef headerName, const CompileOptions& options, SourceLocation importLocation);

} // namespace cx
