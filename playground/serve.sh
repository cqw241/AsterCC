#!/usr/bin/env bash
set -euo pipefail
PLAYGROUND_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname -- "$PLAYGROUND_DIR")"
cd "$PROJECT_DIR"
export UV_LINK_MODE=copy
uv sync --project "$PLAYGROUND_DIR" --frozen
npm --prefix "$PLAYGROUND_DIR/web" ci --no-audit --no-fund
npm --prefix "$PLAYGROUND_DIR/web" run build
printf '\nAsterCC Playground: http://127.0.0.1:%s\n\n' "${PORT:-8000}"
exec uv run --project "$PLAYGROUND_DIR" --frozen uvicorn playground.server.app:app --host 127.0.0.1 --port "${PORT:-8000}"
