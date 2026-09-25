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

for file in ../docs/*.md .generated/*.md .generated/std/*.md .generated/std/*/*.md index.html bench.html; do
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
        bench.html)
            relpath="bench"
            ;;
    esac

    case $relpath in
        index|bench)
            toc=""
            ;;
        *)
            toc=--include-before-body=".generated/toc.html"
            ;;
    esac

    # bench.js loads via an include because pandoc strips script elements
    # from the page source.
    case $relpath in
        bench)
            extra=--include-after-body="bench-script.html"
            ;;
        *)
            extra=""
            ;;
    esac

    if [ "$relpath" = "index" ]; then
        title="cx Programming Language"
        body_class="frontpage"
    elif [ "$relpath" = "bench" ]; then
        title="cx - Benchmarks"
        body_class=""
    else
        # The first '# ' heading is the section name (unwrap links: '# [List](...)' -> 'List').
        title="cx - $(sed -n 's/^# //p' "$file" | head -n 1 | sed 's/^\[\(.*\)\](.*/\1/')"
        case "$relpath" in
            std*) body_class="std" ;;
            *) body_class="" ;;
        esac
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
    # ellipsis, and em/en dashes into the generated HTML. The front page is
    # raw HTML, not Markdown: forcing the Markdown reader on it escapes its
    # indented blocks into code listings.
    case "$file" in
        *.md) from="markdown-smart" ;;
        *) from="html" ;;
    esac
    pandoc -f "$from" "$file" -o "build/$outpath.html" -s --template="template.html" --include-before-body="top-nav.html" $toc --include-after-body="footer.html" $extra --metadata pagetitle="$title" --metadata body-class="$body_class"

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
import os
import re
import sys

REPO_URL = "https://github.com/cx-language/cx"

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


def page_links(outpath):
    """Footer links for one page: edit the source, report an issue.

    Top-level pages come from docs/, stdlib file pages from std/; the
    generated std index and category pages aggregate many files and get
    only the issue link. The front page gets neither.
    """
    if outpath == "index":
        return ""
    if "/" not in outpath:
        # bench.html has no docs/ source; skip its edit link instead of
        # pointing at a nonexistent file.
        if not os.path.isfile("../docs/%s.md" % outpath):
            source = None
        else:
            source = "docs/%s.md" % outpath
    elif os.path.isfile("../std/%s.cx" % outpath[4:]):
        source = "std/%s.cx" % outpath[4:]
    else:
        source = None
    links = []
    if source is not None:
        links.append('<a href="%s/edit/main/%s" target="_blank">Edit this page</a>' % (REPO_URL, source))
    links.append('<a href="%s/issues/new" target="_blank">Report an issue</a>' % REPO_URL)
    return '<span class="page-links">%s</span>' % "".join(links)


if "##PAGELINKS##" in template:
    template = template.replace("##PAGELINKS##", page_links(outpath))


def docs_order():
    """Guide reading order: sidebar links backed by a docs/ source file."""
    toc = open("toc.html").read()
    ids = re.findall(r'href="\./([^"#]+)"', toc)
    return [page for page in ids if os.path.isfile("../docs/%s.md" % page)]


def page_title(page):
    with open("../docs/%s.md" % page) as file:
        for line in file:
            if line.startswith("# "):
                return re.sub(r"^\[(.*)\]\(.*", r"\1", line[2:].strip())
    return page


def next_page(outpath):
    """Next-button for guide pages; API docs and the front page get none."""
    if "/" in outpath or outpath == "index":
        return ""
    order = docs_order()
    if outpath not in order:
        return ""
    following = order[order.index(outpath) + 1 :]
    if not following:
        return ""
    target = following[0]
    return '<nav class="next-page"><a class="button" href="./%s">Next: %s →</a></nav>' % (
        target,
        html.escape(page_title(target)),
    )


if "##NEXTPAGE##" in template:
    template = template.replace("##NEXTPAGE##", next_page(outpath))

showcase = [
    ("Filter and map", "filter-map.cx"),
    ("Tagged unions", "tagged-union.cx"),
    ("Null safety", "null-safety.cx"),
    ("String interpolation", "interpolation.cx"),
    ("Result", "result.cx"),
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

# Search index over the docs and generated reference sources.
python3 generate_search_index.py || exit

cp -r *.css *.js lib build

# Local graph preview: drop a bench-data.json next to this script (e.g. saved
# from the deployed site) and it is served with the preview build.
if [ -f "bench-data.json" ]; then
    cp "bench-data.json" build/
fi

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
