#include "ast/arena.h"
#include "ast/expr.h"
#include <cassert>
#include <string>

int main() {
    std::string source = "identifier";
    llvm::StringRef interned = cx::internString(source);
    source.assign("changed!!");
    assert(interned == "identifier");

    llvm::StringRef duplicate = cx::internString("identifier");
    assert(interned.data() == duplicate.data());

    llvm::StringRef other = cx::internString("other");
    assert(interned.data() != other.data());

    llvm::StringRef empty = cx::internString("");
    assert(empty.data() == cx::internString("").data());

    llvm::StringRef embeddedNul("a\0b", 3);
    assert(embeddedNul.data() != cx::internString("a").data());
    assert(embeddedNul.size() == 3);

    cx::VarExpr firstVariable("shared", {});
    cx::VarExpr secondVariable("shared", {});
    assert(firstVariable.identifier.data() == secondVariable.identifier.data());
}
