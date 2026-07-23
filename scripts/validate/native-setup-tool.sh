#!/usr/bin/env bash
set -euo pipefail

source_dir="$(CDPATH='' cd -- "$(dirname -- "$0")/../.." && pwd)"
cd "$source_dir"

root="$(mktemp -d "${TMPDIR:-/tmp}/mochad-native-setup.XXXXXX")"
prefix=/opt/mochad-redux
tool="$source_dir/scripts/mochad-redux-setup"
cleanup() { rm -rf "$root"; }
trap cleanup EXIT INT HUP TERM

./autogen.sh >/dev/null
./configure >/dev/null

install_fixture() {
    fixture_root="$1"
    mkdir -p "$fixture_root$prefix/share/mochad-redux/templates" "$fixture_root$prefix/bin"
    cp packaging/linux/config/mochad.conf.example \
        packaging/linux/systemd/mochad.service.in \
        packaging/linux/udev/91-usb-x10-controllers.rules.in \
        "$fixture_root$prefix/share/mochad-redux/templates/"
    : >"$fixture_root$prefix/bin/mochad"
    chmod 0755 "$fixture_root$prefix/bin/mochad"
}

setup() { bash "$tool" "$@" --root "$root" --prefix "$prefix"; }
expect_fail() { if "$@" >/dev/null 2>&1; then echo "FAIL: expected command to fail: $*" >&2; exit 1; fi; }

echo "== mochad-redux validation: native setup tool =="
install_fixture "$root"

# Fresh install, custom identities, and idempotent reinstall.
setup install --service-user house --service-group automation --usb-group x10lab
test -f "$root/etc/systemd/system/mochad.service"
test -f "$root/etc/udev/rules.d/91-usb-x10-controllers.rules"
test -f "$root/etc/mochad-redux/mochad.conf"
grep -q "User=house" "$root/etc/systemd/system/mochad.service"
grep -q 'GROUP="x10lab"' "$root/etc/udev/rules.d/91-usb-x10-controllers.rules"
verify_output="$(setup verify)"
printf '%s\n' "$verify_output" | grep -q "OK $root/etc/systemd/system/mochad.service"
status_output="$(setup status)"
printf '%s\n' "$status_output" | grep -q "binary=$root$prefix/bin/mochad"
setup install --service-user house --service-group automation --usb-group x10lab

# Dry runs cannot create a second fake host tree.
dry_root="$root/dry"
install_fixture "$dry_root"
bash "$tool" install --root "$dry_root" --prefix "$prefix" --dry-run >/dev/null
test ! -e "$dry_root/etc"

# Unknown and locally modified integration files require force.
printf 'local unit\n' >"$root/etc/systemd/system/mochad.service"
expect_fail setup install
setup install --force
printf '# local edit\n' >>"$root/etc/udev/rules.d/91-usb-x10-controllers.rules"
expect_fail setup install
setup install --force

# Normal updates preserve configuration and never restart implicitly in a
# staged root. Explicit service controls are rejected with --no-systemd.
printf '# keep me\n' >>"$root/etc/mochad-redux/mochad.conf"
setup install
grep -q 'keep me' "$root/etc/mochad-redux/mochad.conf"
expect_fail setup install --no-systemd --restart

# Activation options are accepted in an isolated root and rejected when they
# conflict with --no-systemd. Actual systemd invocation is host-only evidence,
# never part of this fake-root validator.
setup install --restart

# Feature switches support non-systemd and non-udev installations.
mode_root="$root/modes"
install_fixture "$mode_root"
bash "$tool" install --root "$mode_root" --prefix "$prefix" --no-systemd --no-udev
test ! -e "$mode_root/etc/systemd/system/mochad.service"
test ! -e "$mode_root/etc/udev/rules.d/91-usb-x10-controllers.rules"

# Remove only managed integration, preserving config. Purge is explicit and
# protected; account deletion is intentionally refused pending host review.
setup remove
test ! -e "$root/etc/systemd/system/mochad.service"
test ! -e "$root/etc/udev/rules.d/91-usb-x10-controllers.rules"
test -f "$root/etc/mochad-redux/mochad.conf"
expect_fail setup purge
expect_fail setup purge --force --purge-accounts

echo "PASS: native setup tool contract completed"
