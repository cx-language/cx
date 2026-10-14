#include "ast/type.h"
#include <cassert>

using namespace cx;

int main() {
    for (const char* name :
         {"int8",  "int16",  "int32",  "int64",   "uint8",      "uint16",      "uint32",  "uint64",   "c_size_t", "c_schar", "c_uchar", "c_short", "c_ushort",
          "c_int", "c_uint", "c_long", "c_ulong", "c_longlong", "c_ulonglong", "c_float", "c_double", "float32",  "float64", "float80", "bool",    "char"}) {
        assert(Type::isBuiltinScalar(name));
    }
    // Aliases and other types are not builtin scalars, even at colliding lengths.
    for (const char* name : {"", "int", "uint", "float", "double", "string", "void", "null", "boolx", "c_intx", "Array", "Optional", "main"}) {
        assert(!Type::isBuiltinScalar(name));
    }
    return 0;
}
