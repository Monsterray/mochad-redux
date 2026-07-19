#!/bin/sh
set -eu

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
cd "$SCRIPT_DIR/.."

CC=${CC:-cc}
CFLAGS=${CFLAGS:-}
BUILD_DIR=${BUILD_DIR:-}
STRICT=0
RUN_CLANG_TIDY=0
RUN_CLANG_FORMAT=0
RUN_CPPCHECK=0
CPPCHECK_STYLE=0
ASAN=0
UBSAN=0

usage() {
    cat <<EOF
Usage: $0 [options]

Compile the non-libusb mochad source files as standalone object files.

Options:
  --strict              Add stricter warning flags and treat warnings as errors.
  --clang-tidy          Run clang-tidy on the libusb-free source files.
  --clang-format-check  Check formatting with clang-format.
  --cppcheck            Run cppcheck on the libusb-free source files.
  --cppcheck-style      Include cppcheck style suggestions.
  --asan                Add AddressSanitizer compile flags.
  --ubsan               Add UndefinedBehaviorSanitizer compile flags.
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
        --clang-tidy)
            RUN_CLANG_TIDY=1
            shift
            ;;
        --clang-format-check)
            RUN_CLANG_FORMAT=1
            shift
            ;;
        --cppcheck)
            RUN_CPPCHECK=1
            shift
            ;;
        --cppcheck-style)
            RUN_CPPCHECK=1
            CPPCHECK_STYLE=1
            shift
            ;;
        --asan)
            ASAN=1
            shift
            ;;
        --ubsan)
            UBSAN=1
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

if [ "$ASAN" -eq 1 ]; then
    CFLAGS="$CFLAGS -fsanitize=address -fno-omit-frame-pointer"
fi

if [ "$UBSAN" -eq 1 ]; then
    CFLAGS="$CFLAGS -fsanitize=undefined -fno-omit-frame-pointer"
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
config.c
diagnostics.c
decode.c
encode.c
global.c
mochad_event.c
socket_io.c
x10state.c
x10_write.c
"

find_tool() {
    tool=$1
    if command -v "$tool" >/dev/null 2>&1; then
        command -v "$tool"
        return 0
    fi
    if [ -x "/usr/local/opt/llvm/bin/$tool" ]; then
        echo "/usr/local/opt/llvm/bin/$tool"
        return 0
    fi
    if [ -x "/opt/homebrew/opt/llvm/bin/$tool" ]; then
        echo "/opt/homebrew/opt/llvm/bin/$tool"
        return 0
    fi
    return 1
}

if [ "$RUN_CLANG_FORMAT" -eq 1 ]; then
    CLANG_FORMAT=$(find_tool clang-format) || {
        echo "clang-format requested but not found" >&2
        exit 127
    }
    for source in $SOURCES; do
        echo "clang-format check $source"
        "$CLANG_FORMAT" --dry-run --Werror "$source"
    done
fi

if [ "$RUN_CLANG_TIDY" -eq 1 ]; then
    CLANG_TIDY=$(find_tool clang-tidy) || {
        echo "clang-tidy requested but not found" >&2
        exit 127
    }
    for source in $SOURCES; do
        echo "clang-tidy $source"
        # Annex K functions such as memcpy_s and snprintf_s are optional in C11
        # and unavailable on many supported Linux targets, so keep this checker
        # out of the default clang-tidy gate.
        # shellcheck disable=SC2086
        "$CLANG_TIDY" "$source" \
            --checks=clang-analyzer-*,-clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling \
            -- -I. $CFLAGS
    done
fi

if [ "$RUN_CPPCHECK" -eq 1 ]; then
    if ! command -v cppcheck >/dev/null 2>&1; then
        echo "cppcheck requested but not found" >&2
        exit 127
    fi
    CPPCHECK_ENABLE=warning,performance,portability
    if [ "$CPPCHECK_STYLE" -eq 1 ]; then
        CPPCHECK_ENABLE="$CPPCHECK_ENABLE,style"
    fi
    # shellcheck disable=SC2086
    cppcheck --check-level=exhaustive --enable="$CPPCHECK_ENABLE" --error-exitcode=1 \
        --inline-suppr --quiet -I. $SOURCES
fi

for source in $SOURCES; do
    object="$BUILD_DIR/${source%.c}.o"
    echo "Compiling $source"
    # shellcheck disable=SC2086
    "$CC" $CFLAGS -I. -c "$source" -o "$object"
done

echo "libusb-free compile passed"
