#!/usr/bin/env bash
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
MOCHAD_DOCKER_DIR="${MOCHAD_DOCKER_DIR:-$REPO_ROOT/../mochad-docker}"

echo "== mochad-redux validation: Docker build =="
echo "mochad-redux: $REPO_ROOT"
echo "mochad-docker: $MOCHAD_DOCKER_DIR"
echo

if ! command -v docker >/dev/null 2>&1; then
    echo "FAIL: docker command not found" >&2
    exit 127
fi

if [ ! -f "$MOCHAD_DOCKER_DIR/docker-compose.yml" ]; then
    echo "FAIL: docker-compose.yml not found in $MOCHAD_DOCKER_DIR" >&2
    echo "Set MOCHAD_DOCKER_DIR to the standalone mochad-docker project path." >&2
    exit 2
fi

cd "$MOCHAD_DOCKER_DIR"

echo "+ docker compose build"
docker compose build

echo
echo "PASS: Docker image build completed"
