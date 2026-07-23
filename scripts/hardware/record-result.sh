#!/usr/bin/env bash
set -euo pipefail

usage() {
    cat <<'EOF'
Usage: scripts/hardware/record-result.sh --check NAME --status STATUS [--note TEXT] [--output FILE]

STATUS must be PASS, FAIL, NOT RUN, NOT APPLICABLE, or HARDWARE REQUIRED.
EOF
}

check_name=
status=
note=
output=
while [ "$#" -gt 0 ]; do
    case "$1" in
        --check)
            check_name=${2:?missing check name}
            shift 2
            ;;
        --status)
            status=${2:?missing status}
            shift 2
            ;;
        --note)
            note=${2:?missing note}
            shift 2
            ;;
        --output)
            output=${2:?missing output}
            shift 2
            ;;
        --help|-h)
            usage
            exit 0
            ;;
        *)
            echo "FAIL: unknown argument: $1" >&2
            usage >&2
            exit 64
            ;;
    esac
done

case "$status" in
    PASS|FAIL|'NOT RUN'|'NOT APPLICABLE'|'HARDWARE REQUIRED') ;;
    *)
        echo "FAIL: invalid status: $status" >&2
        exit 64
        ;;
esac

if [ -z "$check_name" ]; then
    usage >&2
    exit 64
fi

repo_root=$(CDPATH='' cd -- "$(dirname -- "$0")/../.." && pwd)
sha=$(git -C "$repo_root" rev-parse HEAD)
output=${output:-"$repo_root/validation/hardware-$(date -u +%Y%m%dT%H%M%SZ)-${sha:0:12}.md"}
mkdir -p "$(dirname -- "$output")"

if [ ! -e "$output" ]; then
    cat >"$output" <<EOF
# mochad-redux Hardware Lab Evidence

- Commit SHA: $sha
- Started UTC: $(date -u +%Y-%m-%dT%H:%M:%SZ)

| Check | Status | Notes |
| --- | --- | --- |
EOF
fi

printf '| %s | %s | %s |\n' "$check_name" "$status" "${note//|/\\|}" >>"$output"
printf '%s\n' "$output"
