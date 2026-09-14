# To develop the website locally, run this script after each change,
# and serve the generated HTML from the build directory using e.g. 'npx serve'.

pandoc --version >/dev/null || exit

cd spec || exit
rm -rf ../build
mkdir ../build
# Disabled for now
# pandoc spec.tex -o ../build/spec.html -s --toc --include-before-body="../top-nav.html" --include-before-body="../toc.html"
cd ..

for file in book/*.md index.html; do
    basename=$(basename "$file")

    case $basename in
        index.html)
            toc=""
            ;;
        *)
            toc=--include-before-body="toc.html"
            ;;
    esac

    basename="${basename%.*}"
    pandoc "$file" -o "build/$basename.html" -s --template="template.html" --include-before-body="top-nav.html" $toc --metadata pagetitle="C*"

    template=$(<"build/$basename.html")
    example=$(<../examples/tree.cx)
    echo "${template//##EXAMPLECODE##/$example}" > "build/$basename.html"
done

cp -r *.css *.js lib build

# Playground WebAssembly artifacts, if built (see wasm/README.md). Without
# them the site still works, but the Run buttons report that the playground
# is unavailable until the artifacts are deployed.
for artifact in cx-wasm.js cx-wasm.wasm cx-wasm.data cc.wasm wcc-files.zip; do
    if [ -f "../wasm/dist/$artifact" ]; then
        cp "../wasm/dist/$artifact" build/
    else
        echo "warning: ../wasm/dist/$artifact not found, the playground will be unavailable" >&2
    fi
done
