#!/bin/sh
set -eu

CC=${CC:-cc}
CFLAGS=${CFLAGS:-}
BUILD_DIR=${BUILD_DIR:-}
STRICT=0

usage() {
    cat <<EOF
Usage: $0 [options]

Compile the non-libusb mochad source files as standalone object files.

Options:
  --strict              Add stricter warning flags and treat warnings as errors.
  --cc COMMAND          Compiler command to use. Defaults to CC or cc.
  --cflags FLAGS        Replace CFLAGS for this run.
  --extra-cflags FLAGS  Append additional compiler flags.
  --build-dir DIR       Keep object files in DIR instead of a temporary dir.
  -h, --help            Show this help.
EOF
}

require_arg() {
    option=$1
    value=${2:-}
    if [ -z "$value" ]; then
        echo "$option requires an argument" >&2
        usage >&2
        exit 2
    fi
}

while [ "$#" -gt 0 ]; do
    case "$1" in
        --strict)
            STRICT=1
            shift
            ;;
        --cc)
            require_arg "$1" "${2:-}"
            CC=$2
            shift 2
            ;;
        --cflags)
            require_arg "$1" "${2:-}"
            CFLAGS=$2
            shift 2
            ;;
        --extra-cflags)
            require_arg "$1" "${2:-}"
            CFLAGS="$CFLAGS $2"
            shift 2
            ;;
        --build-dir)
            require_arg "$1" "${2:-}"
            BUILD_DIR=$2
            shift 2
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            echo "Unknown option: $1" >&2
            usage >&2
            exit 2
            ;;
    esac
done

if [ "$STRICT" -eq 1 ]; then
    CFLAGS="$CFLAGS -Wall -Wextra -Werror -Wformat -Wformat-security -Wshadow -Wpointer-arith -Wcast-align -Wwrite-strings -Wmissing-prototypes -Wstrict-prototypes"
fi

if [ -z "$BUILD_DIR" ]; then
    BUILD_DIR=$(mktemp -d "${TMPDIR:-/tmp}/mochad-no-libusb.XXXXXX")
    CLEAN_BUILD_DIR=1
else
    mkdir -p "$BUILD_DIR"
    CLEAN_BUILD_DIR=0
fi

cleanup() {
    if [ "$CLEAN_BUILD_DIR" -eq 1 ]; then
        rm -rf "$BUILD_DIR"
    fi
}
trap cleanup EXIT INT HUP TERM

SOURCES="
decode.c
encode.c
global.c
x10state.c
x10_write.c
"

for source in $SOURCES; do
    object="$BUILD_DIR/${source%.c}.o"
    echo "Compiling $source"
    # shellcheck disable=SC2086
    "$CC" $CFLAGS -I. -c "$source" -o "$object"
done

echo "libusb-free compile passed"
