#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")/../.."

RUN_LIBUSB_FREE=1
RUN_FULL_BUILD=1
RUN_NATIVE_SMOKE=1
DRY_RUN=0

usage() {
    cat <<EOF
Usage: $0 [options]

Remove ignored build artifacts, then run mochad-redux build validation from a
clean workspace state.

Options:
  --libusb-free-only    Run only the strict libusb-free compile after cleaning.
  --skip-libusb-free    Skip the strict libusb-free compile.
  --skip-full-build     Skip ./autogen.sh, ./configure, and make.
  --skip-smoke          Skip the native command-line smoke test.
  --dry-run             Show ignored files that would be removed, then exit.
  -h, --help            Show this help.

Notes:
  The clean step uses 'git clean -fdX', which removes ignored files only. It is
  intended to delete stale Autotools outputs, object files, binaries, and build
  directories without touching tracked source files.
EOF
}

while [ "$#" -gt 0 ]; do
    case "$1" in
        --libusb-free-only)
            RUN_LIBUSB_FREE=1
            RUN_FULL_BUILD=0
            RUN_NATIVE_SMOKE=0
            shift
            ;;
        --skip-libusb-free)
            RUN_LIBUSB_FREE=0
            shift
            ;;
        --skip-full-build)
            RUN_FULL_BUILD=0
            shift
            ;;
        --skip-smoke)
            RUN_NATIVE_SMOKE=0
            shift
            ;;
        --dry-run)
            DRY_RUN=1
            shift
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

if ! git rev-parse --is-inside-work-tree >/dev/null 2>&1; then
    echo "FAIL: clean-build-test.sh must be run from a git checkout" >&2
    exit 2
fi

echo "== mochad-redux validation: clean build and test =="
echo "Working directory: $PWD"
echo

echo "Ignored build artifacts that will be removed:"
git clean -fdX -n
echo

if [ "$DRY_RUN" -eq 1 ]; then
    echo "DRY RUN: no files removed"
    exit 0
fi

echo "+ git clean -fdX"
git clean -fdX
echo

if [ "$RUN_LIBUSB_FREE" -eq 1 ]; then
    echo "+ scripts/validate/strict-libusb-free-compile.sh"
    scripts/validate/strict-libusb-free-compile.sh
    echo
fi

if [ "$RUN_FULL_BUILD" -eq 1 ]; then
    echo "+ scripts/validate/full-libusb-build.sh"
    scripts/validate/full-libusb-build.sh
    echo
fi

if [ "$RUN_NATIVE_SMOKE" -eq 1 ]; then
    echo "+ scripts/validate/native-smoke-test.sh"
    scripts/validate/native-smoke-test.sh
    echo
fi

echo "PASS: clean build validation completed"
