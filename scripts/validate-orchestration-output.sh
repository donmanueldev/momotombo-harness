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

def positive_integer(key):
    value = payload.get(key)
    if not isinstance(value, int) or isinstance(value, bool) or value < 1:
        raise SystemExit(f"{key} must be a positive integer")

def traceable_entries(key, required_keys, allowed_ids=None):
    entries = payload[key]
    if not entries:
        raise SystemExit(f"{key} must not be empty")
    referenced_ids = set()
    for index, entry in enumerate(entries):
        if not isinstance(entry, dict):
            raise SystemExit(f"{key}[{index}] must be an object")
        for required_key in required_keys:
            value = entry.get(required_key)
            if not isinstance(value, str) or not value.strip():
                raise SystemExit(f"{key}[{index}].{required_key} must be a non-empty string")
        requirement_id = entry["requirement_id"]
        if allowed_ids is not None and requirement_id not in allowed_ids:
            raise SystemExit(f"{key}[{index}].requirement_id must reference a requirement")
        if requirement_id in referenced_ids:
            raise SystemExit(f"{key}[{index}].requirement_id must be unique")
        referenced_ids.add(requirement_id)
    return referenced_ids

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
    "planner": ("constraints", "done_criteria", "execution_plan", "risks", "verification", "requirement_plan"),
    "orchestrator": ("agent_order", "risk_register", "verification_plan", "requirement_assignments"),
    "reviewer": ("findings", "blocking_items", "requirement_coverage"),
}

for key in required_arrays[agent]:
    array(key)

if agent == "specifier":
    nonempty_string("specification_id")
    positive_integer("specification_revision")
    if payload.get("approval_status") not in {"draft", "needs_clarification", "approved"}:
        raise SystemExit("approval_status must be one of draft, needs_clarification, approved")
    if payload["approval_status"] == "needs_clarification" and not payload["open_questions"]:
        raise SystemExit("needs_clarification requires at least one open question")
    if payload["approval_status"] == "approved":
        nonempty_string("approval_evidence")
    if payload["approval_status"] == "approved" and not payload["requirements"]:
        raise SystemExit("approved specifications must define at least one requirement")
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
    nonempty_string("specification_id")
    positive_integer("specification_revision")
    traceable_entries("requirement_plan", ("requirement_id", "owner", "work", "verification"))
elif agent == "orchestrator":
    nonempty_string("specification_id")
    positive_integer("specification_revision")
    if payload.get("route") not in {"start", "plan", "implement", "review", "verify"}:
        raise SystemExit("route must be one of start, plan, implement, review, verify")
    traceable_entries("requirement_assignments", ("requirement_id", "owner", "scope", "verification"))
elif payload.get("decision") not in {"ready", "needs_work"}:
    raise SystemExit("reviewer decision must be ready or needs_work")

if agent == "reviewer":
    nonempty_string("specification_id")
    positive_integer("specification_revision")
    traceable_entries("requirement_coverage", ("requirement_id", "status", "evidence"))
    if any(entry["status"] not in {"verified", "unverified", "not_applicable"} for entry in payload["requirement_coverage"]):
        raise SystemExit("requirement_coverage status must be verified, unverified, or not_applicable")
    for index, finding in enumerate(payload["findings"]):
        if not isinstance(finding, dict):
            raise SystemExit(f"findings[{index}] must be an object")
        for key in ("severity", "file", "summary", "impact"):
            value = finding.get(key)
            if not isinstance(value, str) or not value.strip():
                raise SystemExit(f"findings[{index}].{key} must be a non-empty string")
        requirement_ids = finding.get("requirement_ids")
        if not isinstance(requirement_ids, list) or not requirement_ids or any(not isinstance(requirement_id, str) or not requirement_id.strip() for requirement_id in requirement_ids):
            raise SystemExit(f"findings[{index}].requirement_ids must be a non-empty array of strings")

print("valid")
PY
