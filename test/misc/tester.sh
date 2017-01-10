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
        echo "expected output: $expected_output"
        echo "actual output: $actual_output"
        exit 1
    fi
}

compile function-call.delta
compile function-call-before-declaration.delta
compile_and_run_and_check_exit_status exit-status.delta 42
compile_and_run_and_check_output extern-c-function.delta "foo"
