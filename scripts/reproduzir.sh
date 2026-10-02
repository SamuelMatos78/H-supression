#!/usr/bin/env bash
set -euo pipefail
ROOT=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
MODE=${1:-quick}
case "$MODE" in quick|full) ;; *) echo 'Uso: bash scripts/reproduzir.sh [quick|full]' >&2; exit 2;; esac
cd "$ROOT"
export PYTHONDONTWRITEBYTECODE=1
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
mkdir -p execucoes
OUT="execucoes/${MODE}-$(date -u +%Y%m%dT%H%M%SZ)-$$"
if [ "$MODE" = full ]; then
    python3 codigo/tools/futures.py --binary build/agents/hs_arena --output "$OUT"
else
    python3 scripts/rapido.py --output "$OUT"
fi
printf 'Resultados novos: %s\n' "$ROOT/$OUT"
