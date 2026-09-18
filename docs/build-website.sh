# To develop the website locally, run this script after each change,
# and serve the generated HTML from the build directory using e.g. 'npx serve'.
# Or run with --serve to build and serve in one step:
#   docs/build-website.sh --serve [--port <port>]

SERVE=0
PORT=8000

while [ $# -gt 0 ]; do
    case "$1" in
        --serve)
            SERVE=1
            shift
            ;;
        --port|-p)
            PORT="$2"
            shift 2
            ;;
        --port=*)
            PORT="${1#--port=}"
            shift
            ;;
        -h|--help)
            echo "Usage: $(basename "$0") [--serve] [--port <port>]"
            exit 0
            ;;
        *)
            echo "unknown option: $1" >&2
            exit 1
            ;;
    esac
done

cd "$(dirname "$0")" || exit

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

    if [ "$basename" = "index" ]; then
        title="C* Programming Language"
    else
        # The first '# ' heading is the section name.
        title="C* - $(sed -n 's/^# //p' "$file" | head -n 1)"
    fi

    pandoc "$file" -o "build/$basename.html" -s --template="template.html" --include-before-body="top-nav.html" $toc --include-after-body="footer.html" --metadata pagetitle="$title"

    # Substitute the front-page example code. This must be HTML-escaped:
    # browsers would otherwise parse e.g. List<bool> as an HTML tag, corrupting
    # both the displayed code and the source handed to the playground editor.
    # Python is used for the substitution itself as well because the bash
    # replacement would mangle backslashes in the example code.
    #
    # The showcase examples must all compile warning-free (enforced by
    # check_examples) and run in the browser playground: no C header imports,
    # no file system access, and no float-to-int conversions of unbounded
    # values (those trap on WebAssembly). The first entry is shown by default.
    python3 - "$basename" <<'EOF'
import html
import json
import sys

basename = sys.argv[1]
path = "build/" + basename + ".html"
with open(path) as file:
    template = file.read()

showcase = [
    ("Prime sieve", "sieve.cx"),
    ("Hello world", "hello.cx"),
    ("Fibonacci", "fibonacci.cx"),
    ("FizzBuzz", "fizzbuzz.cx"),
    ("Complex numbers", "complex.cx"),
]

if "##EXAMPLECODE##" in template:
    with open("../examples/" + showcase[0][1]) as file:
        example = html.escape(file.read(), quote=False)
    template = template.replace("##EXAMPLECODE##", example)

if "##EXAMPLESELECTOR##" in template:
    options = "".join(
        '<option value="{}"{}>{}</option>'.format(index, " selected" if index == 0 else "", html.escape(name))
        for index, (name, _) in enumerate(showcase)
    )
    template = template.replace(
        "##EXAMPLESELECTOR##", '<select id="example-selector" aria-label="Example">' + options + "</select>"
    )

with open(path, "w") as file:
    file.write(template)

if basename == "index":
    examples = [{"name": name, "code": open("../examples/" + filename).read()} for name, filename in showcase]
    for example in examples:
        assert "</script" not in example["code"], "example breaks out of playground-examples.js: " + example["name"]
    with open("build/playground-examples.js", "w") as file:
        file.write("var CxExamples = " + json.dumps(examples) + ";\n")
EOF
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

if [ "$SERVE" = 1 ]; then
    echo "Serving build/ at http://localhost:$PORT"
    # The site links pages without the .html suffix (./introduction), which
    # GitHub Pages resolves to introduction.html. Plain 'http.server' does a
    # literal lookup and would 404, so resolve 'path' to 'path.html' as well.
    exec python3 - "$PORT" <<'EOF'
import functools
import http.server
import os
import sys


class Handler(http.server.SimpleHTTPRequestHandler):
    def translate_path(self, path):
        translated = super().translate_path(path)
        if not os.path.exists(translated):
            html_path = translated + ".html"
            if os.path.isfile(html_path):
                return html_path
        return translated


http.server.ThreadingHTTPServer(
    ("", int(sys.argv[1])),
    functools.partial(Handler, directory="build"),
).serve_forever()
EOF
fi
