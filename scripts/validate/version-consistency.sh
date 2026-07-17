#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")/../.."

echo "== mochad-redux validation: version consistency =="
echo "Working directory: $PWD"
echo

version="$(cat VERSION)"

case "$version" in
    v*)
        echo "FAIL: VERSION must not start with v: $version" >&2
        exit 1
        ;;
    "")
        echo "FAIL: VERSION is empty" >&2
        exit 1
        ;;
esac

require_contains() {
    file="$1"
    needle="$2"
    description="$3"
    if ! grep -Fq "$needle" "$file"; then
        echo "FAIL: $description missing from $file: $needle" >&2
        exit 1
    fi
}

require_not_contains() {
    file="$1"
    needle="$2"
    description="$3"
    if grep -Fq "$needle" "$file"; then
        echo "FAIL: $description unexpectedly present in $file: $needle" >&2
        exit 1
    fi
}

require_contains configure.ac 'AC_INIT([mochad-redux], m4_esyscmd_s([cat VERSION])' \
    "Autotools VERSION source"
require_contains version.h "#define MOCHAD_REDUX_VERSION \"$version\"" \
    "C plain version"
require_contains version.h "#define MOCHAD_REDUX_DISPLAY_VERSION \"mochad-redux $version\"" \
    "C display version"
require_contains version.h '#define MOCHAD_UPSTREAM_BASE "mochad 0.1.18"' \
    "upstream base"
require_contains CHANGELOG.md "## v$version" \
    "changelog release candidate heading"
require_contains diagnostics.c 'MOCHAD_REDUX_VERSION' \
    "diagnostic plain version macro"
require_not_contains diagnostics.c 'MOCHAD_REDUX_DISPLAY_VERSION' \
    "display version in machine-readable diagnostics"

if git describe --tags --exact-match >/dev/null 2>&1; then
    tag="$(git describe --tags --exact-match)"
    if [ "$tag" != "v$version" ]; then
        echo "FAIL: checked-out tag $tag does not match VERSION v$version" >&2
        exit 1
    fi
else
    echo "NOTE: current commit is not exactly tagged; skipping release tag match"
fi

echo "PASS: version consistency completed for $version"
