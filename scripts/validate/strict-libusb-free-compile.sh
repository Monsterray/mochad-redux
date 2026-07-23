#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")/../.."

echo "== mochad-redux validation: strict libusb-free compile =="
echo "Working directory: $PWD"
echo

echo "+ sh scripts/build/compile-without-libusb.sh --strict --asan --ubsan"
sh scripts/build/compile-without-libusb.sh --strict --asan --ubsan

echo
echo "PASS: strict libusb-free compile completed"
