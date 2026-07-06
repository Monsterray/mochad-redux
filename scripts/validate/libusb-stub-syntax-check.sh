#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")/../.."

echo "== mochad-redux validation: libusb stub syntax check =="
echo "Working directory: $PWD"
echo

if [ ! -f tools/stubs/libusb-1.0/libusb.h ]; then
    echo "FAIL: tools/stubs/libusb-1.0/libusb.h is missing" >&2
    exit 2
fi

if [ ! -x ./configure ] || [ ! -f Makefile.in ]; then
    echo "+ ./autogen.sh"
    ./autogen.sh
fi

echo "+ ./configure CPPFLAGS=-Itools/stubs"
./configure CPPFLAGS="-Itools/stubs"

echo "+ make -B mochad.o CPPFLAGS=-Itools/stubs"
make -B mochad.o CPPFLAGS="-Itools/stubs"

echo
echo "PASS: mochad.c syntax check completed with the development libusb stub"
echo "NOTE: this is not a runtime test and does not replace a real libusb build."
