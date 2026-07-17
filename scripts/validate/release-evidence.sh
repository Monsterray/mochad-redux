#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")/../.."

release="${1:-}"
if [ -z "$release" ]; then
    if git describe --tags --exact-match >/dev/null 2>&1; then
        release="$(git describe --tags --exact-match)"
    else
        echo "Usage: $0 <release-or-candidate>" >&2
        exit 64
    fi
fi

evidence="validation/releases/${release}.md"

echo "== mochad-redux validation: release evidence =="
echo "Release: $release"
echo "Evidence: $evidence"
echo

if [ ! -f "$evidence" ]; then
    echo "FAIL: missing release evidence record: $evidence" >&2
    exit 1
fi

if ! grep -Eq "^- Release:[[:space:]]+${release}$" "$evidence"; then
    echo "FAIL: evidence record does not declare release ${release}" >&2
    exit 1
fi

if grep -Eq '\|[[:space:]]*(TODO|TBD|UNKNOWN|SKIPPED)[[:space:]]*\|' "$evidence"; then
    echo "FAIL: evidence record contains non-standard status words" >&2
    exit 1
fi

echo "PASS: release evidence record exists and uses standard status terms"
