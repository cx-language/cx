#!/bin/bash

if [ $# != 2 ]; then
    echo "usage: $0 path-to-delta test-file"
    exit 1
fi

path_to_delta=$1
source_file=$2

source "../helpers.sh"

compile_successfully $source_file -codegen=c
./a.out
status=$?
rm a.out
exit $status
