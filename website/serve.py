#!/usr/bin/env python3
"""Dev server for the website: serves build/, rebuilds on source change,
and reloads open pages. Run via: ./build-website.sh --serve"""

import argparse
import functools
import http.server
import os
import subprocess
import sys
import threading
import webbrowser
from pathlib import Path

WEBSITE_DIR = Path(__file__).resolve().parent

# Pages poll this counter and reload when a rebuild lands it.
_VERSION = [1]
_VERSION_LOCK = threading.Lock()

LIVERELOAD_JS = """(async () => {
    const current = await (await fetch("/__version")).text();
    for (;;) {
        await new Promise((r) => setTimeout(r, 1000));
        try {
            if ((await (await fetch("/__version")).text()) !== current) location.reload();
        } catch (e) { /* build in progress; retry */ }
    }
})();
"""

SNIPPET = '<script src="/__livereload.js"></script>'


def inject_livereload(html):
    """Insert the reload snippet before </body>, or append it."""
    marker = html.lower().rfind("</body>")
    if marker < 0:
        return html + SNIPPET
    return html[:marker] + SNIPPET + html[marker:]


class Handler(http.server.SimpleHTTPRequestHandler):
    # The site links pages without the .html suffix (./introduction), which
    # GitHub Pages resolves to introduction.html. Plain 'http.server' does a
    # literal lookup and would 404, so resolve 'path' to 'path.html' as well.
    def translate_path(self, path):
        translated = super().translate_path(path)
        if not os.path.exists(translated):
            html_path = translated + ".html"
            if os.path.isfile(html_path):
                return html_path
        return translated

    def do_GET(self):
        if self.path.split("?", 1)[0] == "/__version":
            with _VERSION_LOCK:
                body = str(_VERSION[0]).encode()
            self.send_response(200)
            self.send_header("Content-Type", "text/plain")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
        elif self.path.split("?", 1)[0] == "/__livereload.js":
            body = LIVERELOAD_JS.encode()
            self.send_response(200)
            self.send_header("Content-Type", "text/javascript")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
        else:
            target = self.translate_path(self.path)
            if os.path.isdir(target):
                target = os.path.join(target, "index.html")
            if target.endswith(".html") and os.path.isfile(target):
                with open(target, encoding="utf-8") as file:
                    body = inject_livereload(file.read()).encode()
                self.send_response(200)
                self.send_header("Content-Type", "text/html; charset=utf-8")
                self.send_header("Content-Length", str(len(body)))
                self.end_headers()
                self.wfile.write(body)
            else:
                super().do_GET()

    def log_message(self, *args):
        pass


def collect_files(website_dir):
    """Every source a website build reads. Build outputs excluded, or the
    watcher would rebuild in a loop off its own writes."""
    website_dir = Path(website_dir)
    repo = website_dir.parent
    files = []
    for path in website_dir.rglob("*"):
        if not path.is_file() or path.suffix not in (".html", ".css", ".js", ".md", ".py", ".sh", ".json"):
            continue
        if any(part in ("build", ".generated", "__pycache__") for part in path.parts):
            continue
        files.append(path)
    files.extend((repo / "docs").glob("*.md"))
    files.extend((repo / "examples").glob("*.cx"))
    files.extend((repo / "std").rglob("*.cx"))
    installer = repo / "install.sh"
    if installer.is_file():
        files.append(installer)
    return files


def snapshot(files):
    return {str(path): path.stat().st_mtime_ns for path in files}


def watch(website_dir, on_change, interval=0.5, settle=0.4):
    """Poll mtimes; call on_change once the tree goes quiet after a change."""
    last = snapshot(collect_files(website_dir))
    while True:
        threading.Event().wait(interval)
        current = snapshot(collect_files(website_dir))
        if current != last:
            threading.Event().wait(settle)
            current = snapshot(collect_files(website_dir))
            last = current
            on_change()


def rebuild(website_dir):
    build = subprocess.run(
        ["sh", "build-website.sh"], cwd=website_dir,
        stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
    if build.returncode == 0:
        with _VERSION_LOCK:
            _VERSION[0] += 1
        print("rebuilt ok, reloading pages", flush=True)
    else:
        print(build.stdout, end="", flush=True)
        print("rebuild failed, keeping previous build", flush=True)


def main(argv):
    parser = argparse.ArgumentParser(description="cx website dev server")
    parser.add_argument("--port", "-p", type=int, default=8000)
    parser.add_argument("--no-open", action="store_true")
    args = parser.parse_args(argv)

    print(f"Serving build/ at http://localhost:{args.port}", flush=True)
    print("Rebuilding on source changes; pages reload automatically.", flush=True)
    threading.Thread(
        target=watch, args=(WEBSITE_DIR, lambda: rebuild(WEBSITE_DIR)),
        daemon=True).start()
    if not args.no_open:
        threading.Timer(
            1.0,
            lambda: webbrowser.open(f"http://localhost:{args.port}/")).start()
    os.chdir(WEBSITE_DIR)
    http.server.ThreadingHTTPServer(
        ("", args.port),
        functools.partial(Handler, directory="build"),
    ).serve_forever()


if __name__ == "__main__":
    main(sys.argv[1:])
