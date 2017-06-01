#pragma once

#include <vector>
#include <memory>
#include <string>
#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/StringRef.h>

namespace delta {

class Decl;

/// Container for the AST of a single file.
class FileUnit {
public:
    explicit FileUnit(llvm::StringRef filePath,
                      std::vector<std::unique_ptr<Decl>>&& topLevelDecls)
    : filePath(filePath), topLevelDecls(std::move(topLevelDecls)) { }
    llvm::ArrayRef<std::unique_ptr<Decl>> getTopLevelDecls() const { return topLevelDecls; }
    llvm::StringRef getFilePath() const { return filePath; }

private:
    std::string filePath;
    std::vector<std::unique_ptr<Decl>> topLevelDecls;
};

/// Container for the AST of a whole module, comprised of one or more FileUnits.
class Module {
public:
    void addFileUnit(FileUnit&& fileUnit) { fileUnits.emplace_back(std::move(fileUnit)); }
    llvm::ArrayRef<FileUnit> getFileUnits() const { return fileUnits; }

private:
    std::vector<FileUnit> fileUnits;
};

}
