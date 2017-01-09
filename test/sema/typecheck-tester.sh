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
