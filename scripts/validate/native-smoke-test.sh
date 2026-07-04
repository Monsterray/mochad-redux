#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")/../.."

MOCHAD_BIN="${MOCHAD_BIN:-./mochad}"

echo "== mochad-redux validation: native smoke test =="
echo "Working directory: $PWD"
echo "Binary: $MOCHAD_BIN"
echo

if [ ! -x "$MOCHAD_BIN" ]; then
    echo "FAIL: $MOCHAD_BIN is not executable" >&2
    echo "Run scripts/validate/full-libusb-build.sh first, or set MOCHAD_BIN." >&2
    exit 2
fi

echo "+ $MOCHAD_BIN --version"
"$MOCHAD_BIN" --version

echo
echo "+ $MOCHAD_BIN --help"
"$MOCHAD_BIN" --help >/tmp/mochad-redux-help.$$
cat /tmp/mochad-redux-help.$$
rm -f /tmp/mochad-redux-help.$$

echo
echo "+ invalid port should fail"
if "$MOCHAD_BIN" --port 70000 >/tmp/mochad-redux-invalid.$$.out 2>/tmp/mochad-redux-invalid.$$.err; then
    cat /tmp/mochad-redux-invalid.$$.out
    cat /tmp/mochad-redux-invalid.$$.err >&2
    rm -f /tmp/mochad-redux-invalid.$$.out /tmp/mochad-redux-invalid.$$.err
    echo "FAIL: invalid port unexpectedly succeeded" >&2
    exit 1
fi
cat /tmp/mochad-redux-invalid.$$.err
rm -f /tmp/mochad-redux-invalid.$$.out /tmp/mochad-redux-invalid.$$.err

echo
echo "+ invalid bind address should fail"
if "$MOCHAD_BIN" --bind not-an-address >/tmp/mochad-redux-bind.$$.out 2>/tmp/mochad-redux-bind.$$.err; then
    cat /tmp/mochad-redux-bind.$$.out
    cat /tmp/mochad-redux-bind.$$.err >&2
    rm -f /tmp/mochad-redux-bind.$$.out /tmp/mochad-redux-bind.$$.err
    echo "FAIL: invalid bind unexpectedly succeeded" >&2
    exit 1
fi
cat /tmp/mochad-redux-bind.$$.err
rm -f /tmp/mochad-redux-bind.$$.out /tmp/mochad-redux-bind.$$.err

echo
echo "PASS: native command-line smoke test completed"
