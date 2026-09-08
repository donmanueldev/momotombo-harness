#!/usr/bin/env bash
set -euo pipefail

DRY_RUN=false

usage() {
    echo "Usage: $0 [--dry-run]"
    exit 1
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --dry-run|-n) DRY_RUN=true; shift ;;
        -h|--help) usage ;;
        *) usage ;;
    esac
done

command -v npx >/dev/null 2>&1 || {
    echo "npx is required to synchronize Kotlin skills." >&2
    exit 1
}

if $DRY_RUN; then
    echo "[dry-run] Would reconcile all official Kotlin skills for Codex."
    exit 0
fi

npx skills@latest add Kotlin/kotlin-agent-skills --global --agent codex --skill '*' --yes
echo "Official Kotlin skills reconciled for Codex."
