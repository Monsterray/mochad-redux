#!/usr/bin/env bash
set -euo pipefail

# The POSIX C locale is available on every supported validation host. Using it
# keeps Autotools and tar output deterministic on systems without C.UTF-8.
export LC_ALL=C
export LANG=C

source_dir="$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)"
cd "$source_dir"

echo "== mochad-redux validation: staged install contract =="
echo "Working directory: $PWD"
echo

build_dir="$(mktemp -d "${TMPDIR:-/tmp}/mochad-staged-install.XXXXXX")"
dest_dir="$(mktemp -d "${TMPDIR:-/tmp}/mochad-destdir.XXXXXX")"

cleanup() {
    rm -rf "$build_dir" "$dest_dir"
}
trap cleanup EXIT INT HUP TERM

# Prefer version-controlled source candidates so maintainers validate
# uncommitted edits, additions, and renames without copying ignored build
# output. Exported archives have no Git metadata, so copy their complete source
# tree instead.
if command -v git >/dev/null 2>&1 &&
        git -C "$source_dir" rev-parse --is-inside-work-tree >/dev/null 2>&1 &&
        [ "$(git -C "$source_dir" rev-parse --show-toplevel)" = "$source_dir" ]; then
    (
        cd "$source_dir"
        git ls-files -z --cached --others --exclude-standard |
            while IFS= read -r -d '' path; do
                if [ -e "$path" ] || [ -L "$path" ]; then
                    printf '%s\0' "$path"
                fi
            done |
            tar --null -T - -cf -
    ) | tar -x -C "$build_dir"
else
    tar --exclude='./.git' -C "$source_dir" -cf - . | tar -x -C "$build_dir"
fi
cd "$build_dir"

./autogen.sh >/dev/null
./configure --prefix=/usr >/dev/null

make -n DESTDIR="$dest_dir" install >"$build_dir/install.plan"
if grep -Eq '(^|[[:space:]])(systemctl|udevadm|useradd|groupadd|usermod|adduser|addgroup|lsusb)([[:space:]]|$)' "$build_dir/install.plan"; then
    echo "FAIL: make install plan contains live host mutation commands" >&2
    grep -En '(systemctl|udevadm|useradd|groupadd|usermod|adduser|addgroup|lsusb)' "$build_dir/install.plan" >&2
    exit 1
fi

if grep -Eq '(^|[[:space:]])/etc/' "$build_dir/install.plan"; then
    echo "FAIL: make install plan writes directly to host /etc" >&2
    grep -En '/etc/' "$build_dir/install.plan" >&2
    exit 1
fi

# make install normally depends on a compiled binary.  A disposable executable
# lets this contract test execute the install rules without requiring libusb.
printf '#!/bin/sh\nexit 0\n' > mochad
chmod 0755 mochad
make -o mochad DESTDIR="$dest_dir" install-binPROGRAMS install-sbinSCRIPTS install-data >/dev/null

test -f "$dest_dir/usr/bin/mochad" || {
    echo "FAIL: staged executable missing" >&2
    exit 1
}
test -x "$dest_dir/usr/sbin/mochad-redux-setup" || {
    echo "FAIL: staged administration tool missing" >&2
    exit 1
}
test -f "$dest_dir/usr/share/mochad-redux/templates/mochad.service.in" || {
    echo "FAIL: staged systemd template missing" >&2
    exit 1
}
test -f "$dest_dir/usr/share/mochad-redux/templates/91-usb-x10-controllers.rules.in" || {
    echo "FAIL: staged udev template missing" >&2
    exit 1
}
test -f "$dest_dir/usr/share/mochad-redux/templates/mochad.conf.example" || {
    echo "FAIL: staged config template missing" >&2
    exit 1
}
test -f "$dest_dir/usr/share/man/man1/mochad.1" || {
    echo "FAIL: staged manual page missing" >&2
    exit 1
}
test -f "$dest_dir/usr/share/man/man8/mochad-redux-setup.8" || {
    echo "FAIL: staged setup manual page missing" >&2
    exit 1
}

if [ -e /etc/mochad-redux-staged-install-sentinel ]; then
    echo "FAIL: staged install touched host /etc" >&2
    exit 1
fi

make DESTDIR="$dest_dir" uninstall >/dev/null

if find "$dest_dir/usr" -type f | grep -q .; then
    echo "FAIL: staged uninstall left files behind:" >&2
    find "$dest_dir/usr" -type f >&2
    exit 1
fi

if [ -e /etc/mochad-redux-staged-install-sentinel ]; then
    echo "FAIL: staged uninstall touched host /etc" >&2
    exit 1
fi

echo "PASS: staged install contract completed"
