#!/bin/bash

if [ $# != 1 ]; then
    echo "usage: $0 path-to-delta"
    exit 1
fi

path_to_delta=$1

source "../helpers.sh"

test_all inputs/*.delta

# Multifile test.
expected_output=$(cat inputs/multifile/output.delta-ast)
actual_output=$($path_to_delta -print-ast inputs/multifile/*.delta)
diff <(echo "$actual_output") <(echo "$expected_output")

if [ $? -ne 0 ]; then
    echo "FAILED: multifile parser test failed"
    exit 1
fi

# Multifile test with error.
actual_output=$($path_to_delta -fsyntax-only inputs/multifile-error/*.delta)
expected_output=$'inputs/multifile-error/b.delta:1:9: error: unknown identifier \'foo\'\nint b = foo;\n        ^'
validate_output

if [ $? -ne 0 ]; then
    echo "FAILED: multifile parser test with error failed"
    exit 1
fi
