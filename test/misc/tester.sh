#!/bin/bash

if [ $# != 1 ]; then
    echo "usage: $0 path-to-delta"
    exit 1
fi

path_to_delta=$1

source "../helpers.sh"

compile_successfully function-call.delta -fsyntax-only
compile_successfully function-call-before-declaration.delta -fsyntax-only
compile_successfully comments.delta -fsyntax-only
compile_successfully int-literal-autocast.delta -fsyntax-only
compile_successfully assign-ptr-to-mutable-into-ptr-to-immutable.delta -fsyntax-only
compile_successfully void-return.delta -fsyntax-only
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
compile_and_run_and_check_exit_status member-function-call.delta 42
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
check_error mixed-case-hex-literal.delta 2:16 "mixed letter case in hex literal"
check_error leading-zero.delta 2:13 "numbers cannot start with 0[0-9], use 0o prefix for octal literal"
check_error newline-inside-string-literal.delta 2:13 "newline inside string literal"
check_error unexpected-character-after-zero.delta 2:14 "unexpected '_'"
check_error unknown-token.delta 1:1 "unknown token '\`'"
check_error tab.delta 2:10 "unknown identifier 'b'"
