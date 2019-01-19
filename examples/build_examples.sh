#!/usr/bin/env bash

set -e

delta=${1:-delta}

cd "$(dirname "$0")"

for file in *.delta; do
    output="${file%.delta}.out"
    $delta $file -o$output
done

echo "All examples built successfully."
