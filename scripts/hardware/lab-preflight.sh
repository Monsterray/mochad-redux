#!/usr/bin/env bash
set -euo pipefail

repo_root=$(CDPATH='' cd -- "$(dirname -- "$0")/../.." && pwd)
lock_path=${X10_HARDWARE_LOCK_PATH:-/run/lock/x10-hardware.lock}
lab_port=${MOCHAD_LAB_PORT:-19099}
test_housecode=${X10_TEST_HOUSECODE:-D}

usage() {
    cat <<'EOF'
Usage: scripts/hardware/lab-preflight.sh

Non-transmitting checks for the restricted CM19A hardware lab.
EOF
}

fail() {
    echo "FAIL: $*" >&2
    exit 1
}

case "${1:-}" in
    --help|-h)
        usage
        exit 0
        ;;
    '')
        ;;
    *)
        usage >&2
        exit 64
        ;;
esac

case "$lab_port" in
    19[0-9][0-9][0-9]) ;;
    *) fail "MOCHAD_LAB_PORT must be in reserved range 19000-19999" ;;
esac

[ "$test_housecode" = D ] ||
    fail "X10_TEST_HOUSECODE must be D; every other housecode is outside the lab boundary"

git -C "$repo_root" diff --quiet || fail "working tree has unstaged changes"
git -C "$repo_root" diff --cached --quiet || fail "working tree has staged changes"
[ -z "$(git -C "$repo_root" status --porcelain --untracked-files=normal)" ] ||
    fail "working tree has untracked files"

command -v flock >/dev/null 2>&1 || fail "flock is required"
id -nG | tr ' ' '\n' | grep -Fx x10 >/dev/null ||
    fail "current account is not in x10"
id -nG | tr ' ' '\n' | grep -Fx x10dev >/dev/null ||
    fail "current account is not in x10dev"
[ -e "$lock_path" ] || fail "hardware lock is missing: $lock_path"
[ -w "$lock_path" ] || fail "hardware lock is not writable: $lock_path"

device_node=
for device in /sys/bus/usb/devices/*; do
    [ -r "$device/idVendor" ] && [ -r "$device/idProduct" ] || continue
    [ "$(cat "$device/idVendor")" = 0bc7 ] || continue
    [ "$(cat "$device/idProduct")" = 0002 ] || continue
    bus=$(cat "$device/busnum")
    dev=$(cat "$device/devnum")
    device_node=$(printf '/dev/bus/usb/%03d/%03d' "$bus" "$dev")
    break
done

[ -n "$device_node" ] || fail "CM19A USB device 0bc7:0002 was not found"
[ -e "$device_node" ] || fail "CM19A node is missing: $device_node"
[ -r "$device_node" ] && [ -w "$device_node" ] ||
    fail "no read/write access to $device_node"

echo "PASS: lab preflight"
echo "repository_sha=$(git -C "$repo_root" rev-parse HEAD)"
echo "controller_node=$device_node"
echo "controller_permissions=$(stat -c '%U:%G %a' "$device_node")"
echo "lock=$lock_path"
echo "port=$lab_port"
echo "housecode=$test_housecode"
