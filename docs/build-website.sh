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
