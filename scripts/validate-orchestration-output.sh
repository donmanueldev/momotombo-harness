#!/usr/bin/env bash
set -euo pipefail

AGENT=""
INPUT=""

usage() {
    echo "Usage: $0 --agent <specifier|planner|orchestrator|reviewer> --input <path-to-json>"
    exit 1
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --agent) AGENT="$2"; shift 2 ;;
        --input) INPUT="$2"; shift 2 ;;
        -h|--help) usage ;;
        *) usage ;;
    esac
done

[[ -n "$AGENT" && -n "$INPUT" ]] || usage
[[ -f "$INPUT" ]] || { echo "input file not found: $INPUT"; exit 1; }

python3 - "$INPUT" "$AGENT" <<'PY'
import json
import sys

path, agent = sys.argv[1:]
allowed_agents = {"specifier", "planner", "orchestrator", "reviewer"}
statuses = {"ok", "blocked", "failed", "in_progress"}

if agent not in allowed_agents:
    raise SystemExit("agent must be one of: specifier, planner, orchestrator, reviewer")

try:
    with open(path, encoding="utf-8") as handle:
        payload = json.load(handle)
except json.JSONDecodeError as error:
    raise SystemExit(f"input must be valid JSON: {error.msg}") from error

if not isinstance(payload, dict):
    raise SystemExit("payload must be a JSON object")

def nonempty_string(key):
    value = payload.get(key)
    if not isinstance(value, str) or not value.strip():
        raise SystemExit(f"{key} must be a non-empty string")

def array(key):
    if not isinstance(payload.get(key), list):
        raise SystemExit(f"{key} must be an array")

if payload.get("agent") != agent:
    raise SystemExit(f"agent field must be '{agent}'")
if payload.get("status") not in statuses:
    raise SystemExit("status must be one of ok, blocked, failed, in_progress")

for key in ("goal", "decision", "result"):
    nonempty_string(key)
array("evidence")

if "next_step" in payload:
    nonempty_string("next_step")

required_arrays = {
    "specifier": ("scope_in", "scope_out", "requirements", "non_functional_requirements", "contracts", "assumptions", "open_questions", "verification_matrix", "risks"),
    "planner": ("constraints", "done_criteria", "execution_plan", "risks", "verification"),
    "orchestrator": ("agent_order", "risk_register", "verification_plan"),
    "reviewer": ("findings", "blocking_items"),
}

for key in required_arrays[agent]:
    array(key)

if agent == "specifier":
    if payload.get("approval_status") not in {"draft", "needs_clarification", "approved"}:
        raise SystemExit("approval_status must be one of draft, needs_clarification, approved")
    if payload["approval_status"] == "needs_clarification" and not payload["open_questions"]:
        raise SystemExit("needs_clarification requires at least one open question")
    if payload["approval_status"] == "approved":
        nonempty_string("approval_evidence")
    requirement_ids = set()
    for index, requirement in enumerate(payload["requirements"]):
        if not isinstance(requirement, dict):
            raise SystemExit(f"requirements[{index}] must be an object")
        for key in ("id", "description"):
            value = requirement.get(key)
            if not isinstance(value, str) or not value.strip():
                raise SystemExit(f"requirements[{index}].{key} must be a non-empty string")
        acceptance_criteria = requirement.get("acceptance_criteria")
        if not isinstance(acceptance_criteria, list) or not acceptance_criteria:
            raise SystemExit(f"requirements[{index}].acceptance_criteria must be a non-empty array")
        if any(not isinstance(criterion, str) or not criterion.strip() for criterion in acceptance_criteria):
            raise SystemExit(f"requirements[{index}].acceptance_criteria must contain non-empty strings")
        if requirement["id"] in requirement_ids:
            raise SystemExit(f"requirements[{index}].id must be unique")
        requirement_ids.add(requirement["id"])
    verified_requirement_ids = set()
    for index, verification in enumerate(payload["verification_matrix"]):
        if not isinstance(verification, dict):
            raise SystemExit(f"verification_matrix[{index}] must be an object")
        for key in ("requirement_id", "method", "evidence"):
            value = verification.get(key)
            if not isinstance(value, str) or not value.strip():
                raise SystemExit(f"verification_matrix[{index}].{key} must be a non-empty string")
        if verification["requirement_id"] not in requirement_ids:
            raise SystemExit(f"verification_matrix[{index}].requirement_id must reference a requirement")
        verified_requirement_ids.add(verification["requirement_id"])
    if verified_requirement_ids != requirement_ids:
        raise SystemExit("verification_matrix must cover every requirement")
elif agent == "planner":
    nonempty_string("context")
elif agent == "orchestrator":
    if payload.get("route") not in {"start", "plan", "implement", "review", "verify"}:
        raise SystemExit("route must be one of start, plan, implement, review, verify")
elif payload.get("decision") not in {"ready", "needs_work"}:
    raise SystemExit("reviewer decision must be ready or needs_work")

if agent == "reviewer":
    for index, finding in enumerate(payload["findings"]):
        if not isinstance(finding, dict):
            raise SystemExit(f"findings[{index}] must be an object")
        for key in ("severity", "file", "summary", "impact"):
            value = finding.get(key)
            if not isinstance(value, str) or not value.strip():
                raise SystemExit(f"findings[{index}].{key} must be a non-empty string")

print("valid")
PY
