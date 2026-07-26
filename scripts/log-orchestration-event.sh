#!/usr/bin/env bash
set -euo pipefail

REPO_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
LOG_FILE="$REPO_DIR/.codex/orchestration.log"
AGENT=""
GOAL=""
STATUS="ok"
DECISION=""
RESULT=""
NEXT_STEP=""

usage() {
    echo "Usage: $0 --agent <name> --goal <goal> --status <ok|blocked|failed|in_progress> --decision <text> --result <text> [--next-step <text>]"
    echo "Optional flags:"
    echo "  --log-file <path>   Override destination log file (default: <repo-root>/.codex/orchestration.log)"
    exit 1
}

sanitize() {
    local value="${1:-}"
    value="${value//$'\n'/ }"
    value="${value//$'\r'/ }"
    value="${value//$'\t'/ }"
    printf '%s' "$value"
}

validate_status() {
    local status_value="$1"
    case "$status_value" in
        ok|blocked|failed|in_progress)
            return 0
            ;;
        *)
            echo "Invalid status: $status_value"
            echo "Allowed statuses: ok | blocked | failed | in_progress"
            return 1
            ;;
    esac
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --agent)
            AGENT="$(sanitize "$2")"
            shift 2
            ;;
        --goal)
            GOAL="$(sanitize "$2")"
            shift 2
            ;;
        --status)
            STATUS="$(sanitize "$2")"
            shift 2
            ;;
        --decision)
            DECISION="$(sanitize "$2")"
            shift 2
            ;;
        --result)
            RESULT="$(sanitize "$2")"
            shift 2
            ;;
        --next-step)
            NEXT_STEP="$(sanitize "$2")"
            shift 2
            ;;
        --log-file)
            LOG_FILE="$2"
            shift 2
            ;;
        -h|--help)
            usage
            ;;
        *)
            usage
            ;;
    esac
done

if ! validate_status "$STATUS"; then
    exit 1
fi

if [[ -z "$AGENT" || -z "$GOAL" || -z "$DECISION" || -z "$RESULT" ]]; then
    usage
fi

mkdir -p "$(dirname "$LOG_FILE")"
ts="$(date -u +%Y-%m-%dT%H:%M:%SZ)"

printf '%s | agent=%s | goal=%s | status=%s | decision=%s | result=%s | next_step=%s\n' \
    "$ts" "$AGENT" "$GOAL" "$STATUS" "$DECISION" "$RESULT" "$NEXT_STEP" >> "$LOG_FILE"

echo "logged: $LOG_FILE"
