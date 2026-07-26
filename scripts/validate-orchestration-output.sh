#!/usr/bin/env bash
set -euo pipefail

AGENT=""
INPUT=""

usage() {
    echo "Usage: $0 --agent <planner|workflow|reviewer> --input <path-to-json-or-yaml>"
    exit 1
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --agent)
            AGENT="$2"
            shift 2
            ;;
        --input)
            INPUT="$2"
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

if [[ -z "$AGENT" || -z "$INPUT" ]]; then
    usage
fi

if [[ "$AGENT" != "planner" && "$AGENT" != "workflow" && "$AGENT" != "reviewer" ]]; then
    echo "agent must be one of: planner, workflow, reviewer"
    exit 1
fi

if [[ ! -f "$INPUT" ]]; then
    echo "input file not found: $INPUT"
    exit 1
fi

first_char="$(sed -n '1s/^[[:space:]]*//' "$INPUT" | head -c 1)"

validate_json() {
    local file="$1"
    local agent="$2"

    python3 - "$file" "$agent" <<'PY'
import json
import sys

path = sys.argv[1]
agent_name = sys.argv[2]

with open(path, "r", encoding="utf-8") as fh:
    payload = json.load(fh)

required_common = ["agent", "goal", "status", "decision", "evidence", "result"]
required_by_agent = {
    "planner": ["context", "constraints", "done_criteria", "execution_plan", "risks", "verification"],
    "workflow": ["route", "agent_order", "risk_register", "verification_plan"],
    "reviewer": ["findings", "blocking_items"],
}

status_set = {"ok", "blocked", "failed", "in_progress"}

if not isinstance(payload, dict):
    raise SystemExit("Payload must be a JSON object")

if payload.get("agent") != agent_name:
    raise SystemExit(f"agent field must be '{agent_name}'")

if payload.get("status") not in status_set:
    raise SystemExit("status must be one of ok, blocked, failed, in_progress")

missing = [key for key in required_common if key not in payload]
if missing:
    raise SystemExit("Missing required keys: " + ", ".join(missing))

if agent_name not in required_by_agent:
    raise SystemExit(f"Unknown agent '{agent_name}'")

missing = [key for key in required_by_agent[agent_name] if key not in payload]
if missing:
    raise SystemExit("Missing required keys for agent " + agent_name + ": " + ", ".join(missing))

if agent_name == "reviewer" and payload.get("decision") not in {"ready", "needs_work"}:
    raise SystemExit("reviewer decision must be ready or needs_work")

print("valid")
PY
}

validate_yaml() {
    local file="$1"
    local agent="$2"
    local decision

    if ! grep -Eq "^agent:[[:space:]]*$agent$|^agent:[[:space:]]*['\"]?$agent['\"]?$" "$file"; then
        echo "agent field must be '$agent'"
        exit 1
    fi

    if ! grep -Eq "^status:[[:space:]]*(ok|blocked|failed|in_progress)" "$file"; then
        echo "status must be one of ok, blocked, failed, in_progress"
        exit 1
    fi

    for key in goal status decision evidence result; do
        if ! grep -Eq "^${key}:[[:space:]]*" "$file"; then
            echo "Missing required key: $key"
            exit 1
        fi
    done

    case "$agent" in
        planner)
            for key in context constraints done_criteria execution_plan risks verification; do
                if ! grep -Eq "^${key}:[[:space:]]*" "$file"; then
                    echo "Missing required key for planner: $key"
                    exit 1
                fi
            done
            ;;
        workflow)
            for key in route agent_order risk_register verification_plan; do
                if ! grep -Eq "^${key}:[[:space:]]*" "$file"; then
                    echo "Missing required key for workflow: $key"
                    exit 1
                fi
            done
            ;;
        reviewer)
            decision="$(grep -E '^decision:' "$file" | head -n 1 | sed -e 's/^decision:[[:space:]]*//' -e 's/[[:space:]]*$//' )"
            decision="${decision#\"}"
            decision="${decision#\'}"
            decision="${decision%\"}"
            decision="${decision%\'}"
            if [[ "$decision" != "ready" && "$decision" != "needs_work" ]]; then
                echo "reviewer decision must be ready or needs_work"
                exit 1
            fi
            for key in findings blocking_items; do
                if ! grep -Eq "^${key}:[[:space:]]*" "$file"; then
                    echo "Missing required key for reviewer: $key"
                    exit 1
                fi
            done
            ;;
        *)
            echo "Unknown agent: $agent"
            exit 1
            ;;
    esac

    echo "valid"
}

if [[ "$first_char" == "{" ]]; then
    validate_json "$INPUT" "$AGENT"
else
    validate_yaml "$INPUT" "$AGENT"
fi

exit 0
