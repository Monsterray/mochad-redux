#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")/../.."

echo "== mochad-redux validation: repository hygiene =="
echo "Working directory: $PWD"
echo

fail() {
    echo "FAIL: $*" >&2
    exit 1
}

if ! git diff --quiet; then
    git status --short >&2
    fail "working tree has unstaged changes"
fi

if ! git diff --cached --quiet; then
    git status --short >&2
    fail "working tree has staged changes"
fi

untracked="$(git ls-files --others --exclude-standard)"
if [ -n "$untracked" ]; then
    printf '%s\n' "$untracked" >&2
    fail "working tree has untracked files"
fi

if ! git diff --check --quiet; then
    git diff --check >&2
    fail "whitespace errors detected"
fi

forbidden_patterns='(^|/)(Makefile\.in|Makefile|configure|config\.guess|config\.sub|compile|depcomp|install-sh|missing|aclocal\.m4)$|(^|/)autom4te\.cache/|\.o$|\.a$|\.so$|\.dylib$|\.gcda$|\.gcno$|^mochad$'
forbidden="$(git ls-files | grep -E "$forbidden_patterns" || true)"
if [ -n "$forbidden" ]; then
    printf '%s\n' "$forbidden" >&2
    fail "tracked generated files or build artifacts detected"
fi

echo "PASS: repository hygiene completed"
