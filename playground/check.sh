#!/usr/bin/env bash
set -euo pipefail
PLAYGROUND_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
cd "$(dirname -- "$PLAYGROUND_DIR")"
uv run --project "$PLAYGROUND_DIR" --frozen python -m pytest "$PLAYGROUND_DIR/tests" -q
npm --prefix "$PLAYGROUND_DIR/web" test
npm --prefix "$PLAYGROUND_DIR/web" run build
