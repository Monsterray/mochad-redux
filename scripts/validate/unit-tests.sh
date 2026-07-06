#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")/../.."

CC=${CC:-cc}
BUILD_DIR=$(mktemp -d "${TMPDIR:-/tmp}/mochad-unit-tests.XXXXXX")

cleanup() {
    rm -rf "$BUILD_DIR"
}
trap cleanup EXIT INT HUP TERM

CFLAGS="${CFLAGS:-} -std=c11 -D_POSIX_C_SOURCE=200809L -DMOCHAD_TESTING -I. -Wall -Wextra -Werror -Wformat -Wformat-security -Wshadow -Wpointer-arith -Wcast-align -Wwrite-strings -Wmissing-prototypes -Wstrict-prototypes -fsanitize=address,undefined -fno-omit-frame-pointer"
LDFLAGS="${LDFLAGS:-} -fsanitize=address,undefined"

echo "== mochad-redux validation: unit tests =="
echo "Working directory: $PWD"
echo

echo "+ socket_io"
# shellcheck disable=SC2086
"$CC" $CFLAGS tests/test_socket_io.c socket_io.c -o "$BUILD_DIR/test_socket_io" $LDFLAGS
"$BUILD_DIR/test_socket_io"

echo
echo "+ config"
# shellcheck disable=SC2086
"$CC" $CFLAGS tests/test_config.c config.c -o "$BUILD_DIR/test_config" $LDFLAGS
"$BUILD_DIR/test_config"

echo
echo "+ diagnostics"
# shellcheck disable=SC2086
"$CC" $CFLAGS tests/test_diagnostics.c diagnostics.c config.c -o "$BUILD_DIR/test_diagnostics" $LDFLAGS
"$BUILD_DIR/test_diagnostics" >/dev/null
echo "PASS: diagnostics"

echo
echo "PASS: unit tests completed"
