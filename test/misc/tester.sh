if [ $# != 1 ]; then
    echo "usage: $0 path-to-delta"
    exit 1
fi

path_to_delta=$1

compile() {
    source_file=$1

    $path_to_delta -fsyntax-only inputs/$source_file

    if [ $? -ne 0 ]; then
        exit 1
    fi
}

compile function-call.delta
compile function-call-before-declaration.delta
