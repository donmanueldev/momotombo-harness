#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${ROOT_DIR}/build/debug"

cmake -S "${ROOT_DIR}" \
  -B "${BUILD_DIR}" \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_PREFIX_PATH="/opt/homebrew/opt/qt;/opt/homebrew/opt/hiredis;/opt/homebrew/opt/googletest"

cmake --build "${BUILD_DIR}"

echo
echo "Build completed: ${BUILD_DIR}/src/RedisForge"
