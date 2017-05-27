#!/usr/bin/env bash

CLANG_FORMAT_VERSION=6.0.0

if ! clang-format --version | grep --quiet $CLANG_FORMAT_VERSION; then
    echo "Wrong clang-format version, expected $CLANG_FORMAT_VERSION."
    exit 1
fi

shopt -s nullglob
ROOTDIR=$(cd "$(dirname "$0")/.."; pwd)
FILES=$(echo $ROOTDIR/src/{ast,delta,driver,irgen,package-manager,parser,sema,support}/*.{h,cpp})

if [ "$1" = "--check" ]; then
    if grep -En "(while|for|if) *\(.*\) *$" $FILES || grep -En "[^#]else *$" $FILES; then
        echo "Add braces to multiline while/for/if/else statements."
        exit 1
    fi

    clang-format -output-replacements-xml $FILES | grep "<replacement " | grep --invert-match "// namespace" >/dev/null

    if [ $? -ne 1 ]; then
        echo "Run $(basename $0) to format the code."
        exit 1
    else
        exit 0
    fi
fi

clang-format -i $FILES

# Remove namespace end comments. Making clang-format not emit namespace end comments causes it to
# remove the empty line before the ending brace, but we don't want that.
perl -i -pe 's/^\} \/\/ namespace[ \w]*$/}/g' $FILES
