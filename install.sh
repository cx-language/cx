#!/bin/sh
# Installs cx from GitHub Releases into ~/.cx (override with CX_HOME).
# Usage: curl https://cx-language.github.io/cx/install.sh | bash
# Pin a version with CX_VERSION=v0.1 (default: latest).
set -eu

REPO="cx-language/cx"
VERSION="${CX_VERSION:-latest}"
DEST="${CX_HOME:-$HOME/.cx}"

fail() {
    echo "install.sh: $1" >&2
    exit 1
}

detect_platform() {
    os=$(uname -s)
    arch=$(uname -m)
    case "$os" in
        Linux)
            case "$arch" in
                x86_64) platform="linux-x64" ;;
                *) fail "no prebuilt binary for Linux/$arch (only x64); build from source: https://cx-language.github.io/cx/hello-world" ;;
            esac
            ;;
        Darwin)
            case "$arch" in
                arm64) platform="macos-arm64" ;;
                *) fail "no prebuilt binary for Intel Macs; build from source: https://cx-language.github.io/cx/hello-world" ;;
            esac
            ;;
        MINGW* | MSYS* | CYGWIN*)
            fail "Windows: download cx-windows-x64.zip from https://github.com/$REPO/releases/latest and unpack it"
            ;;
        *)
            fail "unsupported platform $os/$arch; build from source: https://cx-language.github.io/cx/hello-world"
            ;;
    esac
}

release_url() {
    if [ "$VERSION" = "latest" ]; then
        echo "https://github.com/$REPO/releases/latest/download/cx-$platform.tar.gz"
    else
        echo "https://github.com/$REPO/releases/download/$VERSION/cx-$platform.tar.gz"
    fi
}

add_to_path() {
    case ":$PATH:" in
        *":$DEST:"*) echo "$DEST is already on your PATH."; return ;;
    esac
    line="export PATH=\"$DEST:\$PATH\""
    case "$(basename "${SHELL:-sh}")" in
        bash)
            if [ "$os" = "Darwin" ]; then rc="$HOME/.bash_profile"; else rc="$HOME/.bashrc"; fi
            ;;
        zsh) rc="$HOME/.zshrc" ;;
        fish)
            rc="$HOME/.config/fish/config.fish"
            line="fish_add_path \"$DEST\""
            ;;
        *)
            echo "Put $DEST on your PATH, e.g.:"
            echo "  $line"
            return
            ;;
    esac
    mkdir -p "${rc%/*}"
    if grep -qF "$DEST" "$rc" 2> /dev/null; then
        echo "$DEST is already in $rc."
    else
        echo "$line" >> "$rc"
        echo "Added $DEST to your PATH in $rc (takes effect in new shells)."
        echo "For the current shell, run: $line"
    fi
}

command -v curl > /dev/null 2>&1 || fail "curl is required"
command -v tar > /dev/null 2>&1 || fail "tar is required"

detect_platform
url=$(release_url)

tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT INT TERM

echo "Downloading $url"
curl -fsSL -o "$tmp/cx.tar.gz" "$url" || fail "download failed: $url"

mkdir -p "$DEST"
tar -xzf "$tmp/cx.tar.gz" -C "$DEST" --strip-components=1

if [ "$os" = "Darwin" ]; then
    # Curl-downloaded files are quarantined; the ad-hoc signed binary
    # would be blocked on first run without this.
    xattr -dr com.apple.quarantine "$DEST" 2> /dev/null || true
fi

printf 'void main() {\n    println("install ok");\n}\n' > "$tmp/hello.cx"
"$DEST/cx" run "$tmp/hello.cx" > /dev/null || fail "smoke test failed"

echo "Installed cx to $DEST."
add_to_path
