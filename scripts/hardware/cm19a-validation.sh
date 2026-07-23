#!/usr/bin/env bash
set -euo pipefail

repo_root=$(CDPATH='' cd -- "$(dirname -- "$0")/../.." && pwd)

echo "This command now uses the isolated, approval-gated hardware lab runner." >&2
echo "Use --transmit only after reviewing the proposed housecode D commands." >&2
exec "$repo_root/scripts/hardware/run-cm19a-validation.sh" "$@"
