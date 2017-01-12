if [ $# != 1 ]; then
    echo "usage: $0 path-to-delta"
    exit 1
fi

path_to_delta=$1

check_error() {
    source_file=$1
    expected_error="error: $2"

    compiler_stdout=$($path_to_delta -fsyntax-only inputs/$source_file)

    if [[ $expected_error != $compiler_stdout ]]; then
        echo "FAILED:"
        echo "expected error: \"$expected_error\""
        echo "actual output:  \"$compiler_stdout\""
        exit 1
    fi
}

check_error unknown-variable.delta "unknown identifier 'unknown'"
check_error mismatching-binary-operands.delta "operands to binary expression must have same type"
check_error variable-redefinition.delta "redefinition of 'foo'"
check_error function-redefinition.delta "redefinition of 'foo'"
check_error unknown-function.delta "unknown function 'foo'"
check_error too-few-arguments.delta "too few arguments to 'sum', expected 2"
check_error too-many-arguments.delta "too many arguments to 'sum', expected 2"
check_error invalid-argument-type.delta "invalid argument #1 type 'bool' to 'foo', expected 'int'"
check_error call-nonfunction-variable.delta "'foo' is not a function"
check_error return-value-in-void-function.delta "mismatching return type 'int', expected 'void'"
check_error non-bool-if-condition.delta "'if' condition must have type 'bool'"
check_error explicitly-typed-variable.delta "cannot initialize variable of type 'int' with 'bool'"
check_error assignment-of-invalid-type.delta "cannot assign 'int' to variable of type 'bool'"
check_error assignment-to-immutable.delta "cannot assign to immutable variable 'i'"
check_error assignment-to-function.delta "cannot assign to function"
check_error assignment-of-function.delta "function pointers not implemented yet"
check_error increment-of-immutable.delta "cannot increment immutable value"
check_error explicitly-typed-immutable-variable.delta "cannot decrement immutable value"
check_error int-literal-argument-overflow.delta "256 is out of range for parameter of type 'uint8'"
check_error negated-int-literal-argument-overflow.delta "invalid argument #1 type 'int' to 'foo', expected 'uint'"
check_error composite-type-redefinition.delta "redefinition of 'Foo'"
check_error nonexisting-member-access.delta "no member named 'bar' in 'foo'"
check_error unsupported-member-access.delta "'Foo' doesn't support member access"
check_error unknown-member-access-base.delta "unknown identifier 'foo'"
