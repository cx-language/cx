#!/bin/bash

if [ $# != 2 ]; then
    echo "usage: $0 path-to-delta test-file"
    exit 1
fi

path_to_delta=$1
source_file=$2
ast_file=$2.ast

source "../helpers.sh"

diff_output "$source_file -print-ast" $ast_file

if [ $? -ne 0 ]; then
    echo "FAILED: AST of '$source_file' doesn't match expected AST '$ast_file'"
    exit 1
fi
