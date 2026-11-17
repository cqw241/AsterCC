#!/usr/bin/env bash
set -euo pipefail
cd -- "$(dirname -- "$0")/.."
export ASTERCC_ROOT="$PWD"
export LD_LIBRARY_PATH="$PWD/.toolchain/usr/lib/x86_64-linux-gnu:$PWD/.toolchain/usr/lib/llvm-18/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
python3 scripts/toolchain.py
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j 2
ctest --test-dir build --output-on-failure
python3 scripts/bootstrap.py
python3 scripts/test.py --compiler build/bootstrap/O2/stage3 --reference build/astercc0 --output artifacts/tests-stage3.json
python3 scripts/benchmark.py
python3 scripts/sanitize.py
python3 scripts/report.py
python3 scripts/check_delivery.py
printf '\nAsterCC reproduced successfully. Report: 实验报告.md\n'
