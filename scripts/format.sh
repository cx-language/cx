#!/usr/bin/env bash

CLANG_FORMAT_VERSION=6.0.0

if ! clang-format --version | grep --quiet $CLANG_FORMAT_VERSION; then
    echo "Wrong clang-format version, expected $CLANG_FORMAT_VERSION."
    exit 1
fi

ROOTDIR=$(cd "$(dirname "$0")/.."; pwd)
FILES=$(echo $ROOTDIR/src/**/*.{h,cpp})

if [ "$1" = "--check" ]; then
    if grep -En "(while|for|if) *\(.*\) *$" $FILES || grep -En "[^#]else *$" $FILES; then
        echo "Add braces to multiline while/for/if/else statements."
        exit 1
    fi

    clang-format -output-replacements-xml $FILES | grep "<replacement " >/dev/null

    if [ $? -ne 1 ]; then
        echo "Run $(basename $0) to format the code."
        exit 1
    else
        exit 0
    fi
fi

clang-format -i $FILES
