#!/usr/bin/env bash

BUILD_PATH="$1"
CLANG_TOOLS_VERSION=7.0.0

check_version() {
    if ! $1 --version | grep --quiet $CLANG_TOOLS_VERSION; then
        echo "WARNING: Wrong $1 version, expected $CLANG_TOOLS_VERSION."
    fi
}

check_version clang-format
check_version clang-tidy

ROOTDIR=$(cd "$(dirname "$0")/.."; pwd)
FILES=$(echo $ROOTDIR/src/**/*.{h,cpp})

if [ "$2" = "--check" ]; then
    python "$BUILD_PATH/run-clang-tidy.py" -header-filter="^$ROOTDIR/src/.*" -quiet $FILES \
        && ! (clang-format -output-replacements-xml $FILES | grep "<replacement " >/dev/null)

    if [ $? -ne 0 ]; then
        echo "Run the 'format' target to format the code."
        exit 1
    else
        exit 0
    fi
fi

python "$BUILD_PATH/run-clang-tidy.py" -header-filter="^$ROOTDIR/src/.*" -quiet -fix -format $FILES
