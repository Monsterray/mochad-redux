#!/usr/bin/env bash
set -euo pipefail

repo_root=$(CDPATH='' cd -- "$(dirname -- "$0")/../.." && pwd)
MOCHAD_BIN=${MOCHAD_BIN:-"$repo_root/mochad"}
MOCHAD_LAB_PORT=${MOCHAD_LAB_PORT:-19099}
X10_HARDWARE_LOCK_PATH=${X10_HARDWARE_LOCK_PATH:-/run/lock/x10-hardware.lock}
X10_TEST_HOUSECODE=${X10_TEST_HOUSECODE:-D}
X10_TEST_ADDRESS=${X10_TEST_ADDRESS:-D1}
EVIDENCE_FILE=${EVIDENCE_FILE:-"$repo_root/validation/cm19a-lab-$(date -u +%Y%m%dT%H%M%SZ).md"}
transmit=false

usage() {
    cat <<'EOF'
Usage: scripts/hardware/run-cm19a-validation.sh [--transmit]

Without --transmit, runs startup, diagnostics, receive observation, and idle
shutdown only. --transmit still requires an exact interactive approval phrase.
Only development housecode D is accepted.
EOF
}

case "${1:-}" in
    --transmit)
        transmit=true
        ;;
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

case "$X10_TEST_ADDRESS" in
    D[1-9]|D1[0-6]) ;;
    *)
        echo "FAIL: X10_TEST_ADDRESS must be D1-D16" >&2
        exit 64
        ;;
esac

export MOCHAD_LAB_PORT X10_HARDWARE_LOCK_PATH X10_TEST_HOUSECODE

record() {
    "$repo_root/scripts/hardware/record-result.sh" --output "$EVIDENCE_FILE" "$@" >/dev/null
}

"$repo_root/scripts/hardware/lab-preflight.sh"
[ -x "$MOCHAD_BIN" ] || {
    echo "FAIL: build mochad first or set MOCHAD_BIN" >&2
    exit 2
}
command -v nc >/dev/null 2>&1 || {
    echo "FAIL: nc is required" >&2
    exit 127
}

exec 9>"$X10_HARDWARE_LOCK_PATH"
flock -n 9 || {
    echo "FAIL: X10 controller is already claimed" >&2
    exit 75
}

log_file="${EVIDENCE_FILE%.md}.log"
mkdir -p "$(dirname -- "$EVIDENCE_FILE")"

mochad_pid=
monitor_pid=
cleanup() {
    if [ -n "$monitor_pid" ] && kill -0 "$monitor_pid" 2>/dev/null; then
        kill -TERM "$monitor_pid" 2>/dev/null || true
        wait "$monitor_pid" 2>/dev/null || true
    fi
    if [ -n "$mochad_pid" ] && kill -0 "$mochad_pid" 2>/dev/null; then
        kill -TERM "$mochad_pid" 2>/dev/null || true
        wait "$mochad_pid" 2>/dev/null || true
    fi
}
trap cleanup EXIT INT TERM HUP

"$MOCHAD_BIN" -d --bind 127.0.0.1 --port "$MOCHAD_LAB_PORT" \
    --disable-xml --disable-openremote >"$log_file" 2>&1 &
mochad_pid=$!

listener_ready=false
for _ in $(seq 1 30); do
    if nc -z 127.0.0.1 "$MOCHAD_LAB_PORT" >/dev/null 2>&1; then
        listener_ready=true
        break
    fi
    sleep 1
done

if [ "$listener_ready" != true ]; then
    record --check startup --status FAIL --note "TCP listener did not open"
    exit 1
fi
record --check startup --status PASS \
    --note "mochad listened on 127.0.0.1:$MOCHAD_LAB_PORT"

for command in hello health; do
    response=$(printf '%s\n' "$command" | nc -w 3 127.0.0.1 "$MOCHAD_LAB_PORT" || true)
    if printf '%s' "$response" | grep -q '^{' ; then
        record --check "$command diagnostic" --status PASS \
            --note "single-line JSON response received"
    else
        record --check "$command diagnostic" --status FAIL \
            --note "expected JSON response was not received"
        exit 1
    fi
done

tcp_capture_file="${EVIDENCE_FILE%.md}.tcp.log"
nc 127.0.0.1 "$MOCHAD_LAB_PORT" >"$tcp_capture_file" &
monitor_pid=$!
echo "Press one RF remote button on development housecode D, then press Enter."
read -r _ || true
sleep 1
kill -TERM "$monitor_pid" 2>/dev/null || true
wait "$monitor_pid" 2>/dev/null || true
monitor_pid=

if grep -Eq 'Rx RF House(Unit)?: D' "$tcp_capture_file"; then
    record --check "RF receive" --status PASS \
        --note "A housecode D RF event was captured in $tcp_capture_file."
else
    record --check "RF receive" --status "HARDWARE REQUIRED" \
        --note "No housecode D RF event was captured; attach human observation and $tcp_capture_file."
fi

if [ "$transmit" = true ]; then
    cat <<EOF
Proposed transmitting commands:
  rf $X10_TEST_ADDRESS on
  rf $X10_TEST_ADDRESS on

These commands use development housecode D. They may activate real X10 loads.
Type exactly: APPROVE RF $X10_TEST_ADDRESS ON
EOF
    approval=
    read -r approval || true
    if [ "$approval" != "APPROVE RF $X10_TEST_ADDRESS ON" ]; then
        record --check "RF transmit and repeated command" --status "NOT RUN" \
            --note "Exact human approval phrase was not provided."
    else
        printf 'rf %s on\nrf %s on\n' "$X10_TEST_ADDRESS" "$X10_TEST_ADDRESS" |
            nc -w 3 127.0.0.1 "$MOCHAD_LAB_PORT" || true
        record --check "RF transmit and repeated command" --status "HARDWARE REQUIRED" \
            --note "Human approved two RF ON submissions; physical reception requires confirmation."
        record --check "shutdown while transmitting" --status "HARDWARE REQUIRED" \
            --note "Review $log_file and record controller/load observations."
    fi
else
    record --check "RF transmit and repeated command" --status "NOT RUN" \
        --note "Run again with --transmit and provide explicit human approval."
    record --check "shutdown while transmitting" --status "NOT RUN" \
        --note "No transmitting command was approved."
fi

kill -TERM "$mochad_pid"
if wait "$mochad_pid"; then
    record --check "shutdown while idle" --status PASS \
        --note "SIGTERM completed after diagnostic validation."
else
    record --check "shutdown while idle" --status FAIL \
        --note "mochad returned a failure status during SIGTERM shutdown."
    mochad_pid=
    exit 1
fi
mochad_pid=

echo "PASS: automated CM19A lab checks completed"
echo "Evidence: $EVIDENCE_FILE"
echo "Full log: $log_file"
