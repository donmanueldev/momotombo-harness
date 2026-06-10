#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${ROOT_DIR}/build/debug"
APP_NAME="RedisForge.app"
APP_PATH="${BUILD_DIR}/${APP_NAME}"
STAGE_DIR="/private/tmp/redisforge-package"
STAGE_APP="${STAGE_DIR}/${APP_NAME}"
DMG_PATH="/private/tmp/RedisForge.dmg"

cmake -S "${ROOT_DIR}" \
  -B "${BUILD_DIR}" \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_PREFIX_PATH="/opt/homebrew/opt/qt;/opt/homebrew/opt/hiredis;/opt/homebrew/opt/googletest"

cmake --build "${BUILD_DIR}"

if [[ ! -d "${APP_PATH}" ]]; then
  echo "App bundle not found at ${APP_PATH}" >&2
  exit 1
fi

rm -rf "${STAGE_DIR}" "${DMG_PATH}"
mkdir -p "${STAGE_DIR}"
cp -R "${APP_PATH}" "${STAGE_APP}"

/opt/homebrew/bin/macdeployqt "${STAGE_APP}" -always-overwrite -no-codesign
chmod -R u+w "${STAGE_APP}"
codesign --force --deep --sign - "${STAGE_APP}"

hdiutil create \
  -volname "RedisForge" \
  -srcfolder "${STAGE_DIR}" \
  -ov \
  -format UDZO \
  "${DMG_PATH}"

echo
echo "Packaged app: ${STAGE_APP}"
echo "DMG created: ${DMG_PATH}"
