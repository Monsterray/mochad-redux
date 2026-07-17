#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")/../.."

next_release="${1:-}"
if ! [[ "$next_release" =~ ^[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
    echo "Usage: $0 <next-release-version>" >&2
    echo "Example: $0 0.5.0" >&2
    exit 64
fi

if ! git diff --quiet || ! git diff --cached --quiet; then
    echo "FAIL: next-development preparation requires a clean working tree" >&2
    exit 1
fi

version="${next_release}-dev"
today="$(date +%F)"
evidence="validation/releases/v${next_release}.md"

printf '%s\n' "$version" > VERSION
scripts/release/update-version-files.sh

if ! grep -Fq "## [$version]" CHANGELOG.md; then
    temporary="$(mktemp "${TMPDIR:-/tmp}/mochad-changelog.XXXXXX")"
    awk -v heading="## [$version] - Unreleased" '
        !inserted && /^## / {
            print ""
            print heading
            print ""
            print "- Development begins."
            inserted = 1
        }
        { print }
        END {
            if (!inserted) {
                print ""
                print heading
                print ""
                print "- Development begins."
            }
        }
    ' CHANGELOG.md > "$temporary"
    mv "$temporary" CHANGELOG.md
fi

if [ ! -f "$evidence" ]; then
    sed \
        -e "s/^# Release Evidence: vX.Y.Z/# Release Evidence: v$next_release/" \
        -e "s/^- Release:$/- Release: v$next_release/" \
        -e "s/^- Branch:$/- Branch: $(git branch --show-current)/" \
        -e "s/^- Date:$/- Date: $today/" \
        validation/release-evidence-template.md > "$evidence"
fi

scripts/validate/version-consistency.sh

cat <<EOF

Development preparation complete for $version.

Next commands:
  git diff --check
  scripts/validate/version-consistency.sh
  git add VERSION version.h CHANGELOG.md "$evidence"
  git commit -m "Start $version development"

This script does not commit, tag, push, publish an image, or create a release.
EOF
