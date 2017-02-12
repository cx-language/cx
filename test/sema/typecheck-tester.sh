#!/bin/bash

if [ $# != 1 ]; then
    echo "usage: $0 path-to-delta"
    exit 1
fi

path_to_delta=$1

source "../helpers.sh"

check_error unknown-variable.delta 2:15 "unknown identifier 'unknown'"
check_error mismatching-binary-operands.delta 2:17 "invalid operands to binary expression ('int' and 'bool')"
check_error variable-redefinition.delta 3:9 "redefinition of 'foo'"
check_error function-redefinition.delta 2:6 "redefinition of 'foo'"
check_error unknown-function.delta 2:23 "unknown identifier 'foo'"
check_error too-few-arguments.delta 2:25 "too few arguments to 'sum', expected 2"
check_error too-many-arguments.delta 2:25 "too many arguments to 'sum', expected 2"
check_error invalid-argument-type.delta 4:17 "invalid argument #1 type 'mutable bool' to 'foo', expected 'int'"
check_error call-nonfunction-variable.delta 3:15 "'foo' is not a function"
check_error return-no-value-in-non-void-function.delta 2:5 "expected return statement to return a value of type 'int'"
check_error return-value-in-void-function.delta 1:26 "mismatching return type 'int', expected 'void'"
check_error non-bool-if-condition.delta 2:9 "'if' condition must have type 'bool'"
check_error explicitly-typed-variable.delta 2:13 "cannot initialize variable of type 'int' with 'bool'"
check_error assignment-of-invalid-type.delta 3:9 "cannot assign 'int' to variable of type 'mutable bool'"
check_error assignment-to-immutable.delta 3:7 "cannot assign to immutable variable 'i'"
check_error assignment-to-dereferenced-immutable.delta 5:13 "cannot assign to immutable expression"
check_error assign-ptr-to-immutable-into-ptr-to-mutable.delta 4:22 "cannot initialize variable of type 'mutable int*' with 'int*'"
check_error assignment-to-function.delta 3:9 "cannot assign to function"
check_error assignment-of-function.delta 3:13 "function pointers not implemented yet"
check_error increment-of-immutable.delta 3:6 "cannot increment immutable value"
check_error explicitly-typed-immutable-variable.delta 3:6 "cannot decrement immutable value"
check_error int-literal-argument-overflow.delta 3:9 "256 is out of range for parameter of type 'uint8'"
check_error negated-int-literal-argument-overflow.delta 3:9 "invalid argument #1 type 'int' to 'foo', expected 'uint'"
check_error composite-type-redefinition.delta 2:7 "redefinition of 'Foo'"
check_error nonexisting-member-access.delta 9:19 "no member named 'bar' in 'foo'"
check_error unsupported-member-access.delta 8:15 "'Foo' doesn't support member access"
check_error unsupported-subscript.delta 5:5 "cannot subscript 'mutable uint8*', expected array or pointer-to-array"
check_error illegal-subscript-index-type.delta 5:17 "illegal subscript index type 'bool', expected 'int'"
check_error compile-time-out-of-bounds-array-index.delta 5:17 "accessing array out-of-bounds with index 5, array size is 5"
check_error unknown-member-access-base.delta 8:15 "unknown identifier 'foo'"
check_error immutable-member-mutation.delta 11:8 "cannot decrement immutable value"
check_error mismatching-argument-label.delta 3:22 "invalid label 'qux' for argument #2, expected 'bar'"
check_error mismatching-argument-label-init.delta 4:15 "invalid label 'bar' for argument #1, expected 'foo'"
check_error excess-argument-label.delta 3:7 "excess argument label 'foo' for argument #1, expected no label"
check_error illegal-cast.delta 2:13 "illegal cast from 'bool' to 'int**'"
check_error cast-ptr-to-immutable-into-ptr-to-mutable.delta 4:20 "illegal cast from 'void*' to 'mutable int*'"
check_error return-array-with-wrong-size.delta 3:5 "mismatching return type 'mutable int[2]', expected 'int[1]'"
check_error untyped-null.delta 2:9 "couldn't infer type of 'foo', add a type annotation"
