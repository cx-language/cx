#!/bin/bash
if [ $# != 1 ]; then
    echo "usage: $0 path-to-delta"
    exit 1
fi

path_to_delta=$1

validate_ir() {
    source_file=$1
    ir_file=$2

    actual_ir=$($path_to_delta -codegen=ir -o=stdout $source_file)
    expected_ir=$(cat $ir_file)

    diff <(echo "$actual_ir") <(echo $'\n'"$expected_ir")

    if [ $? -ne 0 ]; then
        echo "FAILED: IR of '$source_file' doesn't match expected IR '$ir_file'"
        exit 1
    fi
}

for file in inputs/*.delta; do
    validate_ir $file ${file%.delta}.ll
done
