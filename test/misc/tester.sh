#!/bin/bash
if [ $# != 1 ]; then
    echo "usage: $0 path-to-delta"
    exit 1
fi

path_to_delta=$1

compile() {
    source_file=$1

    $path_to_delta -fsyntax-only inputs/$source_file

    if [ $? -ne 0 ]; then
        exit 1
    fi
}

compile_and_run_and_check_exit_status() {
    source_file=$1
    expected_exit_status=$2

    $path_to_delta inputs/$source_file

    if [ $? -ne 0 ]; then
        exit 1
    fi

    ./a.out
    actual_exit_status=$?
    rm a.out

    if [ $actual_exit_status -ne $expected_exit_status ]; then
        echo "FAILED: expected exit status: $expected_exit_status," \
             "actual exit status: $actual_exit_status"
        exit 1
    fi
}

compile_and_run_and_check_output() {
    source_file=$1
    expected_output=$2

    $path_to_delta inputs/$source_file

    if [ $? -ne 0 ]; then
        exit 1
    fi

    actual_output=$(./a.out)
    rm a.out

    if [[ $actual_output != $expected_output ]]; then
        echo "FAILED:"
        echo "expected output: \"$expected_output\""
        echo "actual output:   \"$actual_output\""
        exit 1
    fi
}

compile_and_check_error() {
    source_file=$1
    expected_output="inputs/$source_file:$2: error: $3"

    actual_output=$($path_to_delta inputs/$source_file)

    if [ $? -eq 0 ]; then
        exit 1
    fi

    if [[ "$actual_output" != "$expected_output" ]]; then
        echo "FAILED:"
        echo "expected output: \"$expected_output\""
        echo "actual output:   \"$actual_output\""
        exit 1
    fi
}

compile function-call.delta
compile function-call-before-declaration.delta
compile comments.delta
compile int-literal-autocast.delta
compile assign-ptr-to-mutable-into-ptr-to-immutable.delta
compile void-return.delta
compile_and_run_and_check_exit_status composite-types.delta 5
compile_and_run_and_check_exit_status mutable-member-mutation.delta 6
compile_and_run_and_check_exit_status composite-type-use-before-declaration.delta 2
compile_and_run_and_check_exit_status exit-status.delta 42
compile_and_run_and_check_exit_status void-main.delta 0
compile_and_run_and_check_exit_status variable-assignment.delta 42
compile_and_run_and_check_exit_status explicitly-typed-mutable-variable.delta 3
compile_and_run_and_check_exit_status named-parameters.delta 5
compile_and_run_and_check_exit_status malloc-realloc-free.delta 0
compile_and_run_and_check_exit_status cast.delta 1
compile_and_run_and_check_exit_status dereference-operator.delta 42
compile_and_run_and_check_exit_status uninitialized.delta 70
#compile_and_run_and_check_exit_status member-function-call.delta 42 # FIXME: Mutating member functions.
compile_and_run_and_check_exit_status member-function-return-type.delta 21
compile_and_run_and_check_exit_status composite-type-parameter-member-access.delta 15
compile_and_run_and_check_exit_status leading-underscore.delta 42
compile_and_run_and_check_exit_status import-stdarg.h.delta 0
compile_and_run_and_check_output extern-c-function.delta "foo"
compile_and_run_and_check_output print-string-literal.delta "Hello, World!"
compile_and_run_and_check_output if.delta $'a\nc\ne'
compile_and_run_and_check_output comparison-operators.delta $'a\nc\ne\ng\ni\nk'
compile_and_run_and_check_output while-loop.delta $'foo\nfoo\nfoo'
compile_and_run_and_check_output array-subscript-via-pointer.delta "Foo"
compile_and_run_and_check_output reference-operator.delta "Bar"
compile_and_run_and_check_output return-array.delta "Bar"
compile_and_run_and_check_output import-c-header.delta "foo bar"
compile_and_check_error mixed-case-hex-literal.delta 2:16 "mixed letter case in hex literal"
compile_and_check_error leading-zero.delta 2:13 "numbers cannot start with 0[0-9], use 0o prefix for octal literal"
compile_and_check_error newline-inside-string-literal.delta 2:13 "newline inside string literal"
compile_and_check_error unexpected-character-after-zero.delta 2:14 "unexpected '_'"
compile_and_check_error unknown-token.delta 1:1 "unknown token '\`'"
