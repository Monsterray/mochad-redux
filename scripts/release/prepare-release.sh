#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")/../.."

release="${1:-}"
if ! [[ "$release" =~ ^[0-9]+\.[0-9]+\.[0-9]+(-rc[1-9][0-9]*)?$ ]]; then
    echo "Usage: $0 <release-version>" >&2
    echo "Example: $0 0.5.0 or $0 0.5.0-rc1" >&2
    exit 64
fi

if ! git diff --quiet || ! git diff --cached --quiet; then
    echo "FAIL: release preparation requires a clean working tree" >&2
    exit 1
fi

today="$(date +%F)"
evidence="validation/releases/v${release}.md"

printf '%s\n' "$release" > VERSION
scripts/release/update-version-files.sh

if ! grep -Fq "## [$release]" CHANGELOG.md; then
    temporary="$(mktemp "${TMPDIR:-/tmp}/mochad-changelog.XXXXXX")"
    awk -v heading="## [$release] - $today" '
        !inserted && /^## / {
            print ""
            print heading
            print ""
            print "- Release preparation in progress."
            inserted = 1
        }
        { print }
        END {
            if (!inserted) {
                print ""
                print heading
                print ""
                print "- Release preparation in progress."
            }
        }
    ' CHANGELOG.md > "$temporary"
    mv "$temporary" CHANGELOG.md
fi

if [ ! -f "$evidence" ]; then
    sed \
        -e "s/^# Release Evidence: vX.Y.Z/# Release Evidence: v$release/" \
        -e "s/^- Release:$/- Release: v$release/" \
        -e "s/^- Branch:$/- Branch: $(git branch --show-current)/" \
        -e "s/^- Date:$/- Date: $today/" \
        validation/release-evidence-template.md > "$evidence"
fi

scripts/validate/version-consistency.sh

cat <<EOF

Release preparation complete for $release.

Next commands:
  git diff --check
  scripts/validate/version-consistency.sh
  # Run the documented build, test, and hardware evidence checks.
  git add VERSION src/config/version.h CHANGELOG.md "$evidence"
  git commit -m "Prepare v$release"
  # After review and merge: git tag -a v$release -m "v$release"

This script does not commit, tag, push, publish an image, or create a release.
EOF
