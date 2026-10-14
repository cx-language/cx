#include "ast/type.h"
#include <cassert>

using namespace cx;

int main() {
    // Plain types intern to one base and self-compare equal.
    Type int1 = Type::getInt32();
    Type int2 = Type::getInt32();
    assert(int1.typeBase == int2.typeBase);
    assert(int1 == int2);

    // Spelling twins are distinct bases but still equal.
    Type spelled1 = Type::getInt32();
    spelled1.aliasSpelling = "int";
    Type spelled2 = Type::getInt32();
    spelled2.aliasSpelling = "myint";
    Type list1 = BasicType::get("List", {GenericArg(spelled1)});
    Type list2 = BasicType::get("List", {GenericArg(spelled2)});
    assert(list1.typeBase != list2.typeBase);
    assert(list1 == list2);

    // Unresolved placeholders never compare equal, not even to themselves.
    Type u1 = UnresolvedType::get();
    Type u2 = UnresolvedType::get();
    assert(u1.typeBase != u2.typeBase);
    assert(!(u1 == u2));
    assert(!(u1 == u1));

    // A type containing a placeholder is not equal to its own base.
    Type arr = BasicType::get("Array", {GenericArg(Type::getInt32()), GenericArg(u1)});
    assert(!(arr == arr));

    // Anonymous (empty-named) types never compare equal either.
    Type anon = BasicType::get("", {});
    assert(!(anon == anon));

    return 0;
}
