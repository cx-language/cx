if [ $# != 2 ]; then
    echo "usage: $0 path-to-delta test-file"
    exit 1
fi

path_to_delta=$1
source_file=$2

compiler_stdout=$($path_to_delta -codegen=c -o=stdout $source_file)

if [ $? -ne 0 ]; then
    echo "FAILED: $compiler_stdout"
    exit 1
fi

echo "$compiler_stdout" | gcc -x c -
./a.out
status=$?
rm a.out
exit $status
