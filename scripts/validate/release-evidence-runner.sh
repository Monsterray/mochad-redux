#!/usr/bin/env bash
set -euo pipefail

usage() {
    cat <<'EOF'
Usage:
  scripts/validate/release-evidence-runner.sh init [output]
  scripts/validate/release-evidence-runner.sh record <output> <check> <status> [note]

Statuses: PASS, FAIL, NOT RUN, NOT APPLICABLE, HARDWARE REQUIRED.
EOF
}

repo_root=$(CDPATH='' cd -- "$(dirname -- "$0")/../.." && pwd)
cd "$repo_root"

validate_status() {
    case "$1" in
        PASS|FAIL|'NOT RUN'|'NOT APPLICABLE'|'HARDWARE REQUIRED') ;;
        *)
            echo "FAIL: invalid evidence status: $1" >&2
            exit 64
            ;;
    esac
}

command=${1:-}
case "$command" in
    init)
        output=${2:-"validation/releases/evidence-$(git rev-parse HEAD).md"}
        mkdir -p "$(dirname -- "$output")"
        cat >"$output" <<EOF
# mochad-redux Release Validation Evidence

- Commit SHA: $(git rev-parse HEAD)
- Recorded UTC: $(date -u +%Y-%m-%dT%H:%M:%SZ)

| Check | Status | Notes |
| --- | --- | --- |
EOF
        ;;
    record)
        output=${2:-}
        check=${3:-}
        status=${4:-}
        note=${5:-}
        if [ -z "$output" ] || [ -z "$check" ] || [ -z "$status" ]; then
            usage >&2
            exit 64
        fi
        validate_status "$status"
        if [ ! -f "$output" ]; then
            echo "FAIL: evidence file does not exist: $output" >&2
            exit 66
        fi
        printf '| %s | %s | %s |\n' "$check" "$status" "${note//|/\\|}" >>"$output"
        ;;
    --help|-h)
        usage
        ;;
    *)
        usage >&2
        exit 64
        ;;
esac
