#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${ROOT_DIR}/build/debug"
APP_NAME="RedisForge.app"
APP_PATH="${BUILD_DIR}/${APP_NAME}"
STAGE_DIR="/private/tmp/redisforge-package"
STAGE_APP="${STAGE_DIR}/${APP_NAME}"
DMG_PATH="/private/tmp/RedisForge.dmg"
APP_BINARY="${STAGE_APP}/Contents/MacOS/RedisForge"
FRAMEWORKS_DIR="${STAGE_APP}/Contents/Frameworks"

bundle_hiredis() {
  local linked_hiredis
  linked_hiredis="$(otool -L "${APP_BINARY}" | awk '/libhiredis/ { print $1; exit }')"

  if [[ -z "${linked_hiredis}" ]]; then
    echo "Unable to locate linked hiredis library in ${APP_BINARY}" >&2
    exit 1
  fi

  mkdir -p "${FRAMEWORKS_DIR}"

  local hiredis_filename
  hiredis_filename="$(basename "${linked_hiredis}")"

  local bundled_hiredis
  bundled_hiredis="@executable_path/../Frameworks/${hiredis_filename}"

  local bundled_hiredis_path
  bundled_hiredis_path="${FRAMEWORKS_DIR}/${hiredis_filename}"

  if [[ ! -f "${bundled_hiredis_path}" ]]; then
    local source_hiredis
    source_hiredis="$(otool -L "${APP_PATH}/Contents/MacOS/RedisForge" | awk '/libhiredis/ { print $1; exit }')"

    if [[ -z "${source_hiredis}" || ! -f "${source_hiredis}" ]]; then
      echo "Unable to locate source hiredis library for bundling" >&2
      exit 1
    fi

    cp -L "${source_hiredis}" "${bundled_hiredis_path}"
  fi

  chmod u+w "${bundled_hiredis_path}"
  install_name_tool -id "${bundled_hiredis}" "${bundled_hiredis_path}"

  if [[ "${linked_hiredis}" != "${bundled_hiredis}" ]]; then
    install_name_tool -change "${linked_hiredis}" "${bundled_hiredis}" "${APP_BINARY}"
  fi
}

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
bundle_hiredis
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
