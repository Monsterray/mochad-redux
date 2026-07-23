#!/usr/bin/env bash
set -euo pipefail

# The POSIX C locale is available on every supported validation host. Using it
# keeps Autotools and tar output deterministic on systems without C.UTF-8.
export LC_ALL=C
export LANG=C

source_dir="$(CDPATH='' cd -- "$(dirname -- "$0")/../.." && pwd)"
cd "$source_dir"

echo "== mochad-redux validation: source archive =="
echo "Working directory: $PWD"
echo

archive_dir="$(mktemp -d "${TMPDIR:-/tmp}/mochad-source-archive.XXXXXX")"

cleanup() {
    rm -rf "$archive_dir"
}
trap cleanup EXIT INT HUP TERM

if command -v git >/dev/null 2>&1 &&
        git -C "$source_dir" rev-parse --is-inside-work-tree >/dev/null 2>&1 &&
        [ "$(git -C "$source_dir" rev-parse --show-toplevel)" = "$source_dir" ]; then
    echo "+ export source candidates from worktree"
    (
        cd "$source_dir"
        git ls-files -z --cached --others --exclude-standard |
            while IFS= read -r -d '' path; do
                if [ -e "$path" ] || [ -L "$path" ]; then
                    printf '%s\0' "$path"
                fi
            done |
            tar --null -T - -cf -
    ) | tar -x -C "$archive_dir"
else
    echo "+ export source tree (Git metadata not available)"
    tar --exclude='./.git' -C "$source_dir" -cf - . | tar -x -C "$archive_dir"
fi

cd "$archive_dir"

echo "+ forbidden artifact scan inside exported source"
forbidden="$(find . \
    \( -name Makefile.in -o -name Makefile -o -name configure \
       -o -name config.guess -o -name config.sub -o -name compile \
       -o -name depcomp -o -name install-sh -o -name missing \
       -o -name aclocal.m4 -o -name autom4te.cache \
       -o -name '*.o' -o -name '*.a' -o -name '*.so' -o -name '*.dylib' \
       -o -name '*.gcda' -o -name '*.gcno' \) -print)"
if [ -x ./mochad ]; then
    forbidden="${forbidden}"$'\n'"./mochad"
fi
if [ -n "$forbidden" ]; then
    printf '%s\n' "$forbidden" >&2
    echo "FAIL: source archive contains generated files or build artifacts" >&2
    exit 1
fi

echo "+ strict libusb-free compile inside exported source"
scripts/validate/strict-libusb-free-compile.sh

if [ -x scripts/validate/staged-install-contract.sh ]; then
    echo "+ staged install contract inside exported source"
    scripts/validate/staged-install-contract.sh
else
    echo "NOT APPLICABLE: staged install contract script is not present in this branch"
fi

echo "PASS: source archive validation completed"
