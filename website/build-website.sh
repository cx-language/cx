#!/bin/sh
# To develop the website locally, run this script after each change,
# and serve the generated HTML from the build directory using e.g. 'npx serve'.
# Or run with --serve to build and serve in one step:
#   website/build-website.sh --serve [--port <port>]

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

# Reject smart typography in prose sources (see check_prose.py).
python3 check_prose.py || exit

pandoc --version >/dev/null || exit

# Pandoc 2 emits different default styles (notably 'html { font-size: 20px }'),
# so refuse to generate a subtly wrong-looking site with it.
major=$(pandoc --version | head -n 1 | cut -d' ' -f2 | cut -d. -f1)
if [ "${major:-0}" -lt 3 ]; then
    echo "error: pandoc 3 or newer is required" >&2
    exit 1
fi

rm -rf build
mkdir build

# Regenerate the standard library reference from the stdlib sources into a
# staging directory. The generated pages are build products, not versioned.
rm -rf .generated
python3 generate_std_docs.py || exit

for file in ../docs/*.md .generated/*.md .generated/std/*.md .generated/std/*/*.md index.html; do
    case $file in
        ../docs/*)
            relpath="${file#../docs/}"
            relpath="${relpath%.md}"
            ;;
        .generated/*)
            relpath="${file#.generated/}"
            relpath="${relpath%.md}"
            ;;
        index.html)
            relpath="index"
            ;;
    esac

    case $relpath in
        index)
            toc=""
            ;;
        *)
            toc=--include-before-body=".generated/toc.html"
            ;;
    esac

    if [ "$relpath" = "index" ]; then
        title="cx Programming Language"
    else
        # The first '# ' heading is the section name (unwrap links: '# [List](...)' -> 'List').
        title="cx - $(sed -n 's/^# //p' "$file" | head -n 1 | sed 's/^\[\(.*\)\](.*/\1/')"
    fi

    # The std index page lives at std/index.html: build/std.html would be
    # shadowed by the build/std/ subpage directory.
    if [ "$relpath" = "std" ]; then
        outpath="std/index"
    else
        outpath="$relpath"
    fi

    mkdir -p "build/$(dirname "$outpath")"
    # markdown-smart: pandoc would otherwise reintroduce curly quotes,
    # ellipsis, and em/en dashes into the generated HTML.
    pandoc -f markdown-smart "$file" -o "build/$outpath.html" -s --template="template.html" --include-before-body="top-nav.html" $toc --include-after-body="footer.html" --metadata pagetitle="$title"

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
    python3 - "$outpath" <<'EOF'
import html
import json
import re
import sys

outpath = sys.argv[1]
path = "build/" + outpath + ".html"
with open(path) as file:
    template = file.read()

# Pages below the site root resolve sibling links and assets relatively,
# so prefix them back up to the root (anchors and absolute URLs excluded).
depth = outpath.count("/")
if depth:
    prefix = "../" * depth
    template = re.sub(r'(href|src)="(\./)?(?!#|/|[a-zA-Z][a-zA-Z0-9+.-]*:)', r'\1="' + prefix, template)

showcase = [
    ("Filter and map", "filter-map.cx"),
    ("Tagged unions", "tagged-union.cx"),
    ("Null safety", "null-safety.cx"),
    ("Printable", "printable.cx"),
    ("Structs", "structs.cx"),
    ("Ranges and loops", "ranges.cx"),
    ("Operator overloading", "operator-overloading.cx"),
]

if "##EXAMPLECODE##" in template:
    with open("../examples/" + showcase[0][1]) as file:
        example = html.escape(file.read().rstrip("\n"), quote=False)
    template = template.replace("##EXAMPLECODE##", example)

if "##EXAMPLESELECTOR##" in template:
    options = "".join(
        '<option value="{}"{}>{}</option>'.format(index, " selected" if index == 0 else "", html.escape(name))
        for index, (name, _) in enumerate(showcase)
    ) + '<option value="more">More examples...</option>'
    template = template.replace(
        "##EXAMPLESELECTOR##", '<select id="example-selector" aria-label="Example">' + options + "</select>"
    )

with open(path, "w") as file:
    file.write(template)

if outpath == "index":
    examples = [{"name": name, "code": open("../examples/" + filename).read().rstrip("\n")} for name, filename in showcase]
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
import threading
import webbrowser


class Handler(http.server.SimpleHTTPRequestHandler):
    def translate_path(self, path):
        translated = super().translate_path(path)
        if not os.path.exists(translated):
            html_path = translated + ".html"
            if os.path.isfile(html_path):
                return html_path
        return translated


threading.Timer(1.0, lambda: webbrowser.open("http://localhost:" + sys.argv[1] + "/")).start()
http.server.ThreadingHTTPServer(
    ("", int(sys.argv[1])),
    functools.partial(Handler, directory="build"),
).serve_forever()
EOF
fi
