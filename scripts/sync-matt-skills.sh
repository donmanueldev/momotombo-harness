#!/usr/bin/env bash
set -euo pipefail

DRY_RUN=false
CONFIG_DIR="$(cd "$(dirname "$0")/.." && pwd)"
CODEX_SKILLS_DIR="$HOME/.codex/skills"

# These names used to be linked from this repository. Keep the migration list
# narrow so unrelated user-managed skills are never touched.
LEGACY_REPO_SKILLS=(
    ask-matt code-review codebase-design diagnosing-bugs domain-modeling
    grill-me grill-with-docs grilling handoff implement
    improve-codebase-architecture prototype research resolving-merge-conflicts
    setup-matt-pocock-skills tdd teach to-spec to-tickets triage wayfinder
    workflow writing-great-skills
)

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
    echo "npx is required to synchronize Matt Pocock skills." >&2
    exit 1
}

if $DRY_RUN; then
    echo "[dry-run] Would remove only legacy agent-config Matt skill symlinks."
    echo "[dry-run] Would reconcile the complete Matt Pocock skill set for Codex."
    exit 0
fi

for skill in "${LEGACY_REPO_SKILLS[@]}"; do
    path="$CODEX_SKILLS_DIR/$skill"
    [ -L "$path" ] || continue

    target="$(readlink "$path")"
    if [[ "$target" == "$CONFIG_DIR/skills/$skill" ]]; then
        rm -f "$path"
        echo "Migrated legacy agent-config skill link: $skill"
    fi
done

npx skills@latest add mattpocock/skills --global --agent codex --skill '*' --yes
echo "Matt Pocock skills reconciled for Codex."
