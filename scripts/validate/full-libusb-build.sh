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

# Discard any objects already in the tree.  Other validation scripts compile
# into this same directory with different headers and CPPFLAGS, and make would
# happily reuse those, which would make this check a statement about whatever
# ran last rather than about a full libusb build.
echo
echo "+ make clean"
make clean

echo
echo "+ make"
make

echo
echo "PASS: full libusb build completed"
