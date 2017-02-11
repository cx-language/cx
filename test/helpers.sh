#!/bin/bash

compile_successfully() {
    compiler_stdout=$($path_to_delta inputs/$1 "${@:2}")

    if [ $? -ne 0 ]; then
        echo "FAILED: $compiler_stdout"
        exit 1
    fi
}

compile_and_run_and_check_exit_status() {
    compile_successfully $1
    expected_exit_status=$2
    ./a.out
    actual_exit_status=$?
    rm a.out

    if [ $actual_exit_status -ne $expected_exit_status ]; then
        echo "FAILED: expected exit status: $expected_exit_status," \
             "actual exit status: $actual_exit_status"
        exit 1
    fi
}

validate_output() {
    if [[ "$actual_output" != "$expected_output" ]]; then
        echo "FAILED:"
        echo "expected output: \"$expected_output\""
        echo "actual output:   \"$actual_output\""
        exit 1
    fi
}

compile_and_run_and_check_output() {
    compile_successfully $1
    expected_output=$2
    actual_output="$(./a.out)"
    rm a.out
    validate_output
}

check_error() {
    source_file=inputs/$1
    line_number=$(echo $2 | sed 's/:.*//')
    column=$(echo $2 | sed 's/.*://')
    line_content=$(sed -n "${line_number}p" $source_file)
    expected_output="$source_file:$2: error: $3"$'\n'"$line_content"$'\n'"$(printf '%*s^' $(($column - 1)))"
    actual_output="$($path_to_delta $source_file)"
    validate_output
}

diff_output() {
    expected_output=$(cat $2)
    actual_output=$($path_to_delta $1)
    output_prefix=$3
    diff <(echo "$actual_output") <(echo "$output_prefix$expected_output")
}
