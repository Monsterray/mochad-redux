#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")/../.."

echo "== mochad-redux validation: libusb stub syntax check =="
echo "Working directory: $PWD"
echo

# This check compiles src/core/mochad.o against the development stub rather
# than the real libusb headers, and make writes it into the source tree.  The
# stub declares libusb_fill_interrupt_transfer() out of line where the real
# header defines it "static inline", so an object left behind here does not
# link against the real library -- and a later "make" sees it as newer than
# mochad.c and reuses it instead of rebuilding.  The result is that a
# subsequent full libusb build fails with an undefined reference that has
# nothing to do with the code under test.  Never leave the artifact behind.
cleanup() {
    rm -f src/core/mochad.o src/core/.deps/mochad.Po
}
trap cleanup EXIT INT HUP TERM

if [ ! -f tests/support/libusb-1.0/libusb.h ]; then
    echo "FAIL: tests/support/libusb-1.0/libusb.h is missing" >&2
    exit 2
fi

if [ ! -x ./configure ] || [ ! -f Makefile.in ]; then
    echo "+ ./autogen.sh"
    ./autogen.sh
fi

echo "+ ./configure CPPFLAGS=-Itests/support"
./configure CPPFLAGS="-Itests/support"

echo "+ make -B src/core/mochad.o CPPFLAGS=-Itests/support"
make -B src/core/mochad.o CPPFLAGS="-Itests/support"

echo "+ verify src/x10/decode.c is linked into mochad"
if ! awk '
    /^am_mochad_OBJECTS =/ { printing = 1 }
    printing { print }
    printing && $0 !~ /\\$/ { exit }
' Makefile | grep -F 'src/x10/decode.$(OBJEXT)' >/dev/null; then
    echo "FAIL: generated mochad link object list does not include decode.o" >&2
    exit 1
fi

echo
echo "PASS: src/core/mochad.c syntax check completed with the development libusb stub"
echo "NOTE: this is not a runtime test and does not replace a real libusb build."
