#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")/../.."

MOCHAD_BIN="${MOCHAD_BIN:-./mochad}"
MOCHAD_PORT="${MOCHAD_PORT:-1099}"
LOG_FILE="${LOG_FILE:-validation/cm19a-$(date +%Y%m%d-%H%M%S).log}"

echo "== mochad-redux validation: interactive CM19A hardware validation =="
echo "Working directory: $PWD"
echo "Binary: $MOCHAD_BIN"
echo "Port: $MOCHAD_PORT"
echo "Log file: $LOG_FILE"
echo

if [ ! -x "$MOCHAD_BIN" ]; then
    echo "FAIL: $MOCHAD_BIN is not executable" >&2
    echo "Run scripts/validate/full-libusb-build.sh first, or set MOCHAD_BIN." >&2
    exit 2
fi

if ! command -v nc >/dev/null 2>&1; then
    echo "FAIL: nc command not found" >&2
    exit 127
fi

mkdir -p "$(dirname "$LOG_FILE")"

cat <<EOF
This interactive check expects:

1. A CM19A attached to this machine.
2. Permission to access the USB device.
3. An X10 RF remote ready for button presses.

The script starts mochad in the foreground, captures logs, opens a TCP
connection, and asks you to press remote buttons. Stop the capture with Ctrl+C.
EOF

echo
read -r -p "Press Enter to start mochad and begin validation..."

: >"$LOG_FILE"

cleanup() {
    if [ "${MOCHAD_PID:-}" ]; then
        echo
        echo "Stopping mochad pid $MOCHAD_PID"
        kill "$MOCHAD_PID" 2>/dev/null || true
        wait "$MOCHAD_PID" 2>/dev/null || true
    fi
}
trap cleanup EXIT INT HUP TERM

echo "+ $MOCHAD_BIN -d --port $MOCHAD_PORT"
"$MOCHAD_BIN" -d --port "$MOCHAD_PORT" 2>&1 | tee -a "$LOG_FILE" &
MOCHAD_PID=$!

echo "Waiting for TCP listener on localhost:$MOCHAD_PORT..."
for _ in 1 2 3 4 5 6 7 8 9 10; do
    if nc -z localhost "$MOCHAD_PORT" >/dev/null 2>&1; then
        break
    fi
    sleep 1
done

if ! nc -z localhost "$MOCHAD_PORT" >/dev/null 2>&1; then
    echo "FAIL: TCP listener did not open on localhost:$MOCHAD_PORT" >&2
    exit 1
fi

echo
echo "Connected. Press CM19A/X10 RF remote buttons now."
echo "The next command prints mochad TCP output. Stop with Ctrl+C when enough evidence is captured."
echo

nc localhost "$MOCHAD_PORT" | tee -a "$LOG_FILE"
