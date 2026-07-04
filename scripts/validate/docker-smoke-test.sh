#!/usr/bin/env bash
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
MOCHAD_DOCKER_DIR="${MOCHAD_DOCKER_DIR:-$REPO_ROOT/../mochad-docker}"
MOCHAD_SERVICE="${MOCHAD_SERVICE:-mochad}"
MOCHAD_PORT="${MOCHAD_PORT:-1099}"
WAIT_SECONDS="${WAIT_SECONDS:-30}"

echo "== mochad-redux validation: Docker smoke test =="
echo "mochad-redux: $REPO_ROOT"
echo "mochad-docker: $MOCHAD_DOCKER_DIR"
echo "service: $MOCHAD_SERVICE"
echo "port: $MOCHAD_PORT"
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

echo "+ docker compose up -d $MOCHAD_SERVICE"
docker compose up -d "$MOCHAD_SERVICE"

echo
echo "Waiting up to $WAIT_SECONDS seconds for container health..."
deadline=$((SECONDS + WAIT_SECONDS))
while [ "$SECONDS" -lt "$deadline" ]; do
    status=$(docker inspect --format '{{if .State.Health}}{{.State.Health.Status}}{{else}}{{.State.Status}}{{end}}' "$MOCHAD_SERVICE" 2>/dev/null || true)
    echo "health/status: ${status:-unknown}"
    if [ "$status" = "healthy" ] || [ "$status" = "running" ]; then
        break
    fi
    sleep 2
done

echo
echo "+ docker compose logs --tail=80 $MOCHAD_SERVICE"
docker compose logs --tail=80 "$MOCHAD_SERVICE"

echo
echo "+ docker compose exec -T $MOCHAD_SERVICE nc -z localhost $MOCHAD_PORT"
docker compose exec -T "$MOCHAD_SERVICE" nc -z localhost "$MOCHAD_PORT"

echo
echo "PASS: Docker smoke test completed"
