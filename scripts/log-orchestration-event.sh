#!/usr/bin/env bash
set -euo pipefail

REPO_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
LOG_FILE="$REPO_DIR/.codex/orchestration.jsonl"
AGENT=""
GOAL=""
STATUS="ok"
DECISION=""
RESULT=""
NEXT_STEP=""

usage() {
    echo "Usage: $0 --agent <planner|orchestrator|reviewer> --goal <goal> --status <ok|blocked|failed|in_progress> --decision <text> --result <text> [--next-step <text>]"
    exit 1
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --agent) AGENT="$2"; shift 2 ;;
        --goal) GOAL="$2"; shift 2 ;;
        --status) STATUS="$2"; shift 2 ;;
        --decision) DECISION="$2"; shift 2 ;;
        --result) RESULT="$2"; shift 2 ;;
        --next-step) NEXT_STEP="$2"; shift 2 ;;
        -h|--help) usage ;;
        *) usage ;;
    esac
done

case "$AGENT" in planner|orchestrator|reviewer) ;; *) echo "Invalid agent: $AGENT"; usage ;; esac
case "$STATUS" in ok|blocked|failed|in_progress) ;; *) echo "Invalid status: $STATUS"; usage ;; esac
[[ -n "$GOAL" && -n "$DECISION" && -n "$RESULT" ]] || usage

python3 - "$LOG_FILE" "$AGENT" "$GOAL" "$STATUS" "$DECISION" "$RESULT" "$NEXT_STEP" <<'PY'
import json
import os
import sys
from datetime import datetime, timezone

path, agent, goal, status, decision, result, next_step = sys.argv[1:]
entry = {
    "timestamp": datetime.now(timezone.utc).replace(microsecond=0).isoformat().replace("+00:00", "Z"),
    "agent": agent,
    "goal": goal,
    "status": status,
    "decision": decision,
    "result": result,
}
if next_step:
    entry["next_step"] = next_step

os.makedirs(os.path.dirname(path), exist_ok=True)
flags = os.O_APPEND | os.O_CREAT | os.O_WRONLY
descriptor = os.open(path, flags, 0o600)
try:
    os.write(descriptor, (json.dumps(entry, ensure_ascii=False, separators=(",", ":")) + "\n").encode())
finally:
    os.close(descriptor)
os.chmod(path, 0o600)
PY

echo "logged: $LOG_FILE"
