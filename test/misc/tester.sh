#!/bin/bash

if [ $# != 1 ]; then
    echo "usage: $0 path-to-delta"
    exit 1
fi

path_to_delta=$1

source "../helpers.sh"

test_all inputs/*.delta
test_all inputs/*/*.delta
