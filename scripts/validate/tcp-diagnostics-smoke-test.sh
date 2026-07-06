#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")/../.."

CC=${CC:-cc}
BUILD_DIR=$(mktemp -d "${TMPDIR:-/tmp}/mochad-tcp-diagnostics.XXXXXX")
OUTPUT="$BUILD_DIR/diagnostics.jsonl"

cleanup() {
    rm -rf "$BUILD_DIR"
}
trap cleanup EXIT INT HUP TERM

CFLAGS="${CFLAGS:-} -std=c11 -D_POSIX_C_SOURCE=200809L -I. -Wall -Wextra -Werror -Wformat -Wformat-security -Wshadow -Wpointer-arith -Wcast-align -Wwrite-strings -Wmissing-prototypes -Wstrict-prototypes -fsanitize=address,undefined -fno-omit-frame-pointer"
LDFLAGS="${LDFLAGS:-} -fsanitize=address,undefined"

echo "== mochad-redux validation: TCP diagnostics smoke test =="
echo "Working directory: $PWD"
echo

echo "+ build TCP diagnostics harness"
# shellcheck disable=SC2086
"$CC" $CFLAGS tests/test_tcp_diagnostics.c diagnostics.c config.c socket_io.c \
    -o "$BUILD_DIR/test_tcp_diagnostics" $LDFLAGS

echo "+ run TCP diagnostics harness"
"$BUILD_DIR/test_tcp_diagnostics" > "$OUTPUT"

echo "+ validate JSON lines"
python3 - "$OUTPUT" <<'PY'
import json
import sys

path = sys.argv[1]
with open(path, "r", encoding="utf-8") as handle:
    lines = [line.strip() for line in handle if line.strip()]

if len(lines) != 5:
    raise SystemExit(f"expected 5 JSON diagnostic lines, got {len(lines)}")

objects = [json.loads(line) for line in lines]

for index, obj in enumerate(objects, 1):
    if obj.get("ok") is not True:
        raise SystemExit(f"diagnostic line {index} did not report ok=true")

if objects[0].get("daemon") != "mochad-redux":
    raise SystemExit("hello response missing daemon identity")
if "health" not in objects[1].get("commands", []):
    raise SystemExit("capabilities response missing health command")
if "listeners" not in objects[2]:
    raise SystemExit("health response missing listeners")
if objects[3].get("listeners", {}).get("main", {}).get("port") != 1099:
    raise SystemExit("config response missing default main port")
if "version" not in objects[4]:
    raise SystemExit("version response missing version")
PY

echo
echo "PASS: TCP diagnostics JSON smoke test completed"
