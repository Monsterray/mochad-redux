#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")/../.."

echo "== mochad-redux validation: source archive =="
echo "Working directory: $PWD"
echo

archive_dir="$(mktemp -d "${TMPDIR:-/tmp}/mochad-source-archive.XXXXXX")"

cleanup() {
    rm -rf "$archive_dir"
}
trap cleanup EXIT INT HUP TERM

git archive HEAD | tar -x -C "$archive_dir"

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
