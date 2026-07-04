#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")/../.."

echo "== mochad-redux validation: full libusb build =="
echo "Working directory: $PWD"
echo

echo "This check requires libusb development headers and the autotools toolchain."
echo

echo "+ ./autogen.sh"
./autogen.sh

echo
echo "+ ./configure"
./configure

echo
echo "+ make"
make

echo
echo "PASS: full libusb build completed"
