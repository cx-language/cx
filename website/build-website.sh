#!/bin/sh
# To develop the website locally, run with --serve: it builds once, serves
# the result, and rebuilds and reloads open pages whenever sources change:
#   website/build-website.sh --serve [--port <port>]
# Without --serve it performs a single build (as CI does for deploys).

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

build_page() {
    file=$1
    case $file in
        ../docs/*)
            relpath="${file#../docs/}"
            relpath="${relpath%.md}"
            # Strip the ordering prefix: 010-foo.md lives at ./foo.
            relpath="$(printf '%s' "$relpath" | sed 's/^[0-9][0-9]*-//')"
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

    # The guide sidebar covers the language documentation; API reference
    # pages get the generated stdlib sidebar instead.
    case $relpath in
        index|bench)
            toc=""
            ;;
        std*)
            toc=--include-before-body=".generated/api-toc.html"
            ;;
        *)
            toc=--include-before-body="toc.html"
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

    # A page owning a subpage directory (the std index, the std category
    # pages) renders as that directory's index: build/std/containers.html
    # would be shadowed by the build/std/containers/ file-page directory,
    # serving a redirect to a listing instead of the page.
    if [ "$relpath" = "std" ]; then
        outpath="std/index"
    elif [ -d ".generated/$relpath" ]; then
        outpath="$relpath/index"
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
    pandoc -f "$from" "$file" -o "build/$outpath.html" -s --template="template.html" --include-before-body="top-nav.html" $toc --include-after-body="footer.html" $extra --metadata pagetitle="$title" --metadata body-class="$body_class" || return 1

    # Substitute the front-page example code. This must be HTML-escaped:
    # browsers would otherwise parse e.g. List<bool> as an HTML tag, corrupting
    # both the displayed code and the source handed to the playground editor.
    # Python is used for the substitution itself as well because the bash
    # replacement would mangle backslashes in the example code.
    #
    # The showcase examples must all compile warning-free (enforced by
    # check_examples) and run in the browser playground: no C header imports,
    # no file system access, and no float-to-int conversions of unbounded
    # values (those trap on WebAssembly). Keep lines short: the hero window
    # fits about 60 columns. The first entry is shown by default.
    python3 - "$outpath" <<'EOF' || return 1
import glob
import html
import json
import os
import re
import sys

REPO_URL = "https://github.com/cx-language/cx"

outpath = sys.argv[1]


def docs_source(page):
    """Source file for a guide page id, with or without an ordering prefix."""
    direct = "../docs/%s.md" % page
    if os.path.isfile(direct):
        return direct
    matches = glob.glob("../docs/[0-9]*-%s.md" % page)
    return matches[0] if matches else None
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
        source_file = docs_source(outpath)
        if source_file is None:
            source = None
        else:
            source = "docs/" + os.path.basename(source_file)
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
    return [page for page in ids if docs_source(page) is not None]


def page_title(page):
    with open(docs_source(page)) as file:
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

showcase = ["unions.cx", "null-safety.cx", "vectors.cx", "lambdas.cx", "cleanup.cx", "errors.cx"]

if "##EXAMPLECODE##" in template:
    with open("../examples/" + showcase[0]) as file:
        example = html.escape(file.read().rstrip("\n"), quote=False)
    template = template.replace("##EXAMPLECODE##", example)

if "##EXAMPLETABS##" in template:
    # Tab labels drop the .cx suffix ("Null-safety", not "null-safety.cx")
    # so more tabs fit the row.
    buttons = "".join(
        '<button aria-pressed="{}" data-tab="{}">{}</button>'.format(
            "true" if index == 0 else "false", index, html.escape(name[:-3].capitalize())
        )
        for index, name in enumerate(showcase)
    )
    # Plain link, not a tab: editor.js only binds button elements, so this
    # navigates without touching the playground.
    more = '<a class="more" href="https://github.com/cx-language/cx/tree/main/examples" target="_blank">more...</a>'
    template = template.replace(
        "##EXAMPLETABS##", '<div class="example-tabs">' + buttons + more + "</div>"
    )

# Inline markup inside code does not survive pandoc either, so grey the
# shell prompts here. Only line-leading "$ "/"&gt; " match, which cx code
# never has (interpolation is always mid-line).
template = re.sub(
    r"<pre ([^>]*class=\"snippet[^\"]*\"[^>]*)><code>(.*?)</code></pre>",
    lambda m: "<pre " + m.group(1) + "><code>" + re.sub(
        r"(?m)^(\$|&gt;)(?= )", r'<span class="prompt">\1</span>', m.group(2)
    ) + "</code></pre>",
    template,
    flags=re.S,
)

with open(path, "w") as file:
    file.write(template)

if outpath == "index":
    examples = [{"name": name, "code": open("../examples/" + name).read().rstrip("\n")} for name in showcase]
    for example in examples:
        assert "</script" not in example["code"], "example breaks out of playground-examples.js: " + example["name"]
    with open("build/playground-examples.js", "w") as file:
        file.write("var CxExamples = " + json.dumps(examples) + ";\n")
EOF
}

# Pages build independently (one pandoc + one post-processing step each, all
# writing distinct files), so build them in parallel batches: a pandoc
# invocation costs ~60ms of startup, which dominates the build sequentially.
max_jobs=$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 4)
pids=""
running=0
failed=0
for file in ../docs/*.md .generated/*.md .generated/std/*.md .generated/std/*/*.md index.html bench.html; do
    build_page "$file" &
    pids="$pids $!"
    running=$((running + 1))
    if [ "$running" -ge "$max_jobs" ]; then
        for pid in $pids; do
            wait "$pid" || failed=1
        done
        pids=""
        running=0
    fi
done
for pid in $pids; do
    wait "$pid" || failed=1
done
[ "$failed" -eq 0 ] || exit 1

# Search index over the docs and generated reference sources.
python3 generate_search_index.py || exit

cp -r *.css *.js lib build

# Gallery screenshots served from the site root.
cp ../examples/fractal/screenshot.jpg build/fractal-screenshot.jpg
cp ../examples/voxel-game/screenshot.jpg build/voxel-screenshot.jpg
cp ../examples/raytracer/screenshot.png build/raytracer-screenshot.png
cp ../examples/boids/screenshot.png build/boids-screenshot.png

# Local graph preview: drop a bench-data.json or langs-data.json next to
# this script (e.g. saved from the deployed site) and it is served with
# the preview build.
if [ -f "bench-data.json" ]; then
    cp "bench-data.json" build/
fi
if [ -f "langs-data.json" ]; then
    cp "langs-data.json" build/
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
    exec python3 serve.py --port "$PORT"
fi
