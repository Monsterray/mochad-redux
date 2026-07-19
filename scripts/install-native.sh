#!/usr/bin/env bash
set -euo pipefail

# This is deliberately thin: native host changes live in the installed setup
# tool, so the source-tree convenience path cannot drift from package installs.
repo_root="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
cd "$repo_root"

if [ ! -x ./mochad ]; then
    echo "mochad is not built; run ./configure and make first" >&2
    exit 69
fi

make install
setup_path="$(make -s print-sbindir)/mochad-redux-setup"
if [ ! -x "$setup_path" ]; then
    echo "installed setup tool not found: $setup_path" >&2
    exit 69
fi

exec "$setup_path" install "$@"
