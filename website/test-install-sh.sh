#!/bin/sh
# Offline test for ../install.sh: stubs out curl to serve a fake tarball and
# installs into a fake HOME, then checks the PATH setup. Usage: sh test-install-sh.sh
set -eu

cd "$(dirname "$0")"

tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT INT TERM

os=$(uname -s)
arch=$(uname -m)
case "$os-$arch" in
    Linux-x86_64 | Darwin-arm64) ;;
    *) echo "skip: no prebuilt platform for $os/$arch"; exit 0 ;;
esac

mkdir -p "$tmp/pkg/cx-test" "$tmp/fakebin"
printf '#!/bin/sh\nexit 0\n' > "$tmp/pkg/cx-test/cx"
chmod +x "$tmp/pkg/cx-test/cx"
tar -czf "$tmp/cx.tar.gz" -C "$tmp/pkg" cx-test

# Stub curl: copies the fake tarball to the -o target, ignoring the URL.
cat > "$tmp/fakebin/curl" <<EOF
#!/bin/sh
prev=""
for arg in "\$@"; do
    if [ "\$prev" = "-o" ]; then out="\$arg"; fi
    prev="\$arg"
done
cp "$tmp/cx.tar.gz" "\$out"
EOF
chmod +x "$tmp/fakebin/curl"

run_install() {
    # $1 = SHELL value, $2 = fake home
    HOME="$2" SHELL="$1" PATH="$tmp/fakebin:$PATH" sh ../install.sh
}

# zsh: appends the export line, prints it for the current shell,
# and the second run is a no-op.
mkdir -p "$tmp/home1"
out=$(run_install /bin/zsh "$tmp/home1")
echo "$out" | grep -q "Added $tmp/home1/.cx to your PATH in $tmp/home1/.zshrc" || { echo "FAIL: no PATH confirmation: $out"; exit 1; }
echo "$out" | grep -qF "For the current shell, run: export PATH=\"$tmp/home1/.cx:\$PATH\"" || { echo "FAIL: export not printed: $out"; exit 1; }
grep -qF "export PATH=\"$tmp/home1/.cx:\$PATH\"" "$tmp/home1/.zshrc" || { echo "FAIL: no export in .zshrc"; exit 1; }
[ -x "$tmp/home1/.cx/cx" ] || { echo "FAIL: cx not installed"; exit 1; }
out=$(run_install /bin/zsh "$tmp/home1")
echo "$out" | grep -q "already in $tmp/home1/.zshrc" || { echo "FAIL: rerun not idempotent: $out"; exit 1; }
[ "$(grep -c "export PATH" "$tmp/home1/.zshrc")" -eq 1 ] || { echo "FAIL: duplicate export"; exit 1; }

# fish: writes config.fish.
mkdir -p "$tmp/home2"
run_install /usr/local/bin/fish "$tmp/home2" > /dev/null
grep -qF "fish_add_path \"$tmp/home2/.cx\"" "$tmp/home2/.config/fish/config.fish" || { echo "FAIL: no fish_add_path"; exit 1; }

# Unknown shell: prints manual instructions instead.
mkdir -p "$tmp/home3"
out=$(run_install /bin/tcsh "$tmp/home3")
echo "$out" | grep -q "Put $tmp/home3/.cx on your PATH" || { echo "FAIL: no manual fallback: $out"; exit 1; }

# cx already on PATH: leaves profiles alone.
mkdir -p "$tmp/home4"
PATH="$tmp/home4/.cx:$tmp/fakebin:$PATH" HOME="$tmp/home4" SHELL=/bin/zsh sh ../install.sh > "$tmp/out4"
grep -q "already on your PATH" "$tmp/out4" || { echo "FAIL: no on-PATH shortcut"; exit 1; }
[ ! -e "$tmp/home4/.zshrc" ] || { echo "FAIL: touched .zshrc despite cx on PATH"; exit 1; }

echo "PASS"
