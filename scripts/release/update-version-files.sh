#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")/../.."

version="$(tr -d '\n' < VERSION)"

if ! [[ "$version" =~ ^[0-9]+\.[0-9]+\.[0-9]+(-dev|-rc[1-9][0-9]*)?$ ]]; then
    echo "FAIL: VERSION is not a supported semantic version: $version" >&2
    exit 1
fi

cat > src/config/version.h <<EOF
/*
 * Generated from VERSION by scripts/release/update-version-files.sh.
 * Do not edit the version strings here by hand.
 */

#ifndef MOCHAD_REDUX_VERSION_H
#define MOCHAD_REDUX_VERSION_H

#define MOCHAD_REDUX_VERSION "$version"
#define MOCHAD_REDUX_DISPLAY_VERSION "mochad-redux $version"
#define MOCHAD_UPSTREAM_BASE "mochad 0.1.18"

#endif
EOF

echo "Updated src/config/version.h from VERSION ($version)"
