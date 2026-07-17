#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")/../.."

echo "== mochad-redux validation: version consistency =="
echo "Working directory: $PWD"
echo

version="$(tr -d '\n' < VERSION)"

case "$version" in
    v*)
        echo "FAIL: VERSION must not start with v: $version" >&2
        exit 1
        ;;
    ""|*[!0-9A-Za-z.-]*|*..*|.*|*.)
        echo "FAIL: VERSION must be a semantic version such as 0.4.0, 0.4.0-dev, or 0.4.0-rc1: $version" >&2
        exit 1
        ;;
esac

if ! [[ "$version" =~ ^[0-9]+\.[0-9]+\.[0-9]+(-dev|-rc[1-9][0-9]*)?$ ]]; then
    echo "FAIL: VERSION must be a semantic version such as 0.4.0, 0.4.0-dev, or 0.4.0-rc1: $version" >&2
    exit 1
fi

require_contains() {
    file="$1"
    needle="$2"
    description="$3"
    if ! grep -Fq -- "$needle" "$file"; then
        echo "FAIL: $description missing from $file: $needle" >&2
        exit 1
    fi
}

require_not_contains() {
    file="$1"
    needle="$2"
    description="$3"
    if grep -Fq -- "$needle" "$file"; then
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
require_contains CHANGELOG.md "## [$version]" "changelog heading"
require_contains diagnostics.c 'MOCHAD_REDUX_VERSION' \
    "diagnostic plain version macro"
require_not_contains diagnostics.c 'MOCHAD_REDUX_DISPLAY_VERSION' \
    "display version in machine-readable diagnostics"
require_contains diagnostics.c '\"name\":\"mochad-redux\"' \
    "diagnostic name identity"

evidence_version="$version"
if [[ "$version" == *-dev ]]; then
    evidence_version="${version%-dev}"
fi
evidence="validation/releases/v${evidence_version}.md"
if [ ! -f "$evidence" ]; then
    echo "FAIL: missing release evidence template or record: $evidence" >&2
    exit 1
fi
require_contains "$evidence" "- Release: v$evidence_version" \
    "release evidence version"

if [ -n "${MOCHAD_BIN:-}" ]; then
    if [ ! -x "$MOCHAD_BIN" ]; then
        echo "FAIL: MOCHAD_BIN is not executable: $MOCHAD_BIN" >&2
        exit 1
    fi
    binary_version="$("$MOCHAD_BIN" --version | sed -n '1p')"
    if [ "$binary_version" != "$version" ]; then
        echo "FAIL: $MOCHAD_BIN --version is $binary_version, expected $version" >&2
        exit 1
    fi
fi

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
