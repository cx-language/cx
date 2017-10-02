#include <string>
#include <vector>
#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/StringRef.h>

namespace delta {

class PackageManifest {
public:
    class Dependency {
    public:
        Dependency(std::string packageIdentifier, std::string packageVersion)
        : packageIdentifier(std::move(packageIdentifier)), packageVersion(std::move(packageVersion)) {}
        llvm::StringRef getPackageIdentifier() const { return packageIdentifier; }
        llvm::StringRef getPackageVersion() const { return packageVersion; }
        std::string getGitRepositoryUrl() const;
        std::string getFileSystemPath() const;

    private:
        std::string packageIdentifier;
        std::string packageVersion;
    };

public:
    PackageManifest(const std::string& packageRoot);
    llvm::ArrayRef<Dependency> getDeclaredDependencies() const { return declaredDependencies; }
    static const char manifestFileName[];

private:
    std::string packageName;
    std::vector<Dependency> declaredDependencies;
};

}
