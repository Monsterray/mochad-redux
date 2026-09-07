#!/usr/bin/env bash
# Validate the inactive macOS launchd template: rendering completes with no
# leftover placeholders and the rendered property list keeps its launchd
# contract (foreground supervision, no --background).
set -euo pipefail

cd "$(dirname "$0")/../.."

TEMPLATE="packaging/macos/LaunchDaemons/com.mochad-redux.mochad.plist.in"
LABEL="com.mochad-redux.mochad"

echo "== mochad-redux validation: macOS launchd plist =="
echo "Working directory: $PWD"
echo "Template: $TEMPLATE"
echo

fail() {
    echo "FAIL: $*" >&2
    exit 1
}

[ -f "$TEMPLATE" ] || fail "template not found: $TEMPLATE"

rendered="$(mktemp "${TMPDIR:-/tmp}/mochad-redux-launchd.XXXXXX")"
trap 'rm -f "$rendered"' EXIT HUP INT TERM

sed "s|@MOCHAD_BINARY@|/usr/local/bin/mochad|g" "$TEMPLATE" >"$rendered"

if grep -q '@[A-Za-z_]*@' "$rendered"; then
    grep '@[A-Za-z_]*@' "$rendered" >&2
    fail "unrendered placeholders remain"
fi

if command -v plutil >/dev/null 2>&1; then
    echo "+ plutil -lint $rendered"
    plutil -lint "$rendered" || fail "plutil rejected the rendered plist"
else
    echo "(plutil not available; skipping Apple lint)"
fi

command -v python3 >/dev/null 2>&1 || fail "python3 is required for plist contract checks"

echo "+ python3 plist contract checks"
TEMPLATE_RENDERED="$rendered" TEMPLATE_LABEL="$LABEL" python3 - <<'EOF'
import os
import plistlib

path = os.environ["TEMPLATE_RENDERED"]
with open(path, "rb") as handle:
    data = plistlib.load(handle)

failures = []


def check(condition, message):
    if not condition:
        failures.append(message)


check(data.get("Label") == os.environ["TEMPLATE_LABEL"], "Label mismatch")
args = data.get("ProgramArguments", [])
check(isinstance(args, list) and len(args) >= 2, "ProgramArguments must list binary and flags")
if isinstance(args, list) and args:
    check(os.path.isabs(args[0]), "ProgramArguments binary must be an absolute path")
check("-d" in args, "ProgramArguments must run the daemon in the foreground (-d)")
check("--background" not in args, "ProgramArguments must not self-detach (--background)")
check(data.get("RunAtLoad") is True, "RunAtLoad must be true")
keepalive = data.get("KeepAlive", None)
check(isinstance(keepalive, dict) and keepalive.get("SuccessfulExit") is False,
      "KeepAlive must be {SuccessfulExit: false} so clean shutdown stays stopped")
check(isinstance(data.get("ThrottleInterval"), int), "ThrottleInterval must be an integer")
for key in ("StandardOutPath", "StandardErrorPath"):
    check(isinstance(data.get(key), str) and os.path.isabs(data[key]),
          "%s must be an absolute path" % key)
check(data.get("WorkingDirectory") == "/", "WorkingDirectory must be /")
check(data.get("ProcessType") == "Background", "ProcessType must be Background")

if failures:
    for failure in failures:
        print("FAIL: %s" % failure)
    raise SystemExit(1)
print("plist contract ok: Label=%s args=%s" % (data["Label"], " ".join(args[1:])))
EOF

echo
echo "PASS: macOS launchd plist completed"
