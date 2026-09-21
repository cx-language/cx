#!/usr/bin/env bash

set -o pipefail

ROOTDIR=$(cd "$(dirname "$0")/.."; pwd)
cd $ROOTDIR
FILES=$(find docs examples scripts src std \( -name '*.h' -o -name '*.cpp' \))

while test $# -gt 0; do
    case "$1" in
        --check) CHECK=1
            ;;
        --*) echo "bad option $1"
            ;;
    esac
    shift
done

check_version() {
    CLANG_TOOLS_VERSION=23
    if ! $1 --version | grep --quiet "version $CLANG_TOOLS_VERSION"; then
        echo "WARNING: Wrong $1 version, expected $CLANG_TOOLS_VERSION."
    fi
}

run_clang_format() {
    ! (clang-format -output-replacements-xml $FILES | grep "<replacement " >/dev/null)
}

check_version clang-format

if [ -n "$CHECK" ]; then
    run_clang_format

    if [ $? -ne 0 ]; then
        echo "Run the 'format' target to format the code."
        exit 1
    else
        exit 0
    fi
fi

clang-format -i $FILES
