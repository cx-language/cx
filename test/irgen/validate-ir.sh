#!/bin/bash

if [ $# != 1 ]; then
    echo "usage: $0 path-to-delta"
    exit 1
fi

path_to_delta=$1

source "../helpers.sh"

for file in inputs/*.delta; do
    diff_output "$file -o=stdout" ${file%.delta}.ll $'\n'

    if [ $? -ne 0 ]; then
        echo "FAILED: IR of '$file' doesn't match expected IR '${file%.delta}.ll'"
        exit 1
    fi
done
