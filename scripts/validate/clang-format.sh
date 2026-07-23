#!/usr/bin/env bash
set -euo pipefail

root="$(CDPATH='' cd -- "$(dirname -- "${BASH_SOURCE[0]}")/../.." && pwd)"
cd "$root"
formatter="${CLANG_FORMAT:-clang-format}"
command -v "$formatter" >/dev/null 2>&1 || { echo "clang-format is required" >&2; exit 127; }

files="$(git ls-files '*.[ch]' | grep -v -F -f .clang-format-ignore || true)"
[ -n "$files" ] || exit 0
while IFS= read -r file; do
    "$formatter" --dry-run --Werror "$file"
done <<EOF
$files
EOF
