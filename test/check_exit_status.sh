#!/bin/sh

expectedExitStatus=$1
shift
"$@"
actualExitStatus=$?

if [ $actualExitStatus != $expectedExitStatus ]; then
    echo "FAIL: expected exit status: $expectedExitStatus, actual exit status: $actualExitStatus"
    exit 1
fi
