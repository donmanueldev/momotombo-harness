#!/usr/bin/env python3
"""Validate machine-readable orchestration artifacts."""

from __future__ import annotations

import argparse
import json
from pathlib import Path


ALLOWED_AGENTS = {"specifier", "planner", "orchestrator", "reviewer"}
STATUSES = {"ok", "blocked", "failed", "in_progress"}


class ContractError(ValueError):
    """Raised when an orchestration artifact violates the contract."""


def _nonempty_string(payload: dict, key: str) -> None:
    value = payload.get(key)
    if not isinstance(value, str) or not value.strip():
        raise ContractError(f"{key} must be a non-empty string")


def _array(payload: dict, key: str) -> None:
    if not isinstance(payload.get(key), list):
        raise ContractError(f"{key} must be an array")


def _positive_integer(payload: dict, key: str) -> None:
    value = payload.get(key)
    if not isinstance(value, int) or isinstance(value, bool) or value < 1:
        raise ContractError(f"{key} must be a positive integer")


def _traceable_entries(
    payload: dict,
    key: str,
    required_keys: tuple[str, ...],
    *,
    allow_empty: bool = False,
) -> set[str]:
    entries = payload[key]
    if not entries and not allow_empty:
        raise ContractError(f"{key} must not be empty")
    referenced_ids: set[str] = set()
    for index, entry in enumerate(entries):
        if not isinstance(entry, dict):
            raise ContractError(f"{key}[{index}] must be an object")
        for required_key in required_keys:
            value = entry.get(required_key)
            if not isinstance(value, str) or not value.strip():
                raise ContractError(
                    f"{key}[{index}].{required_key} must be a non-empty string"
                )
        requirement_id = entry["requirement_id"]
        if requirement_id in referenced_ids:
            raise ContractError(f"{key}[{index}].requirement_id must be unique")
        referenced_ids.add(requirement_id)
    return referenced_ids


def validate_payload(payload: dict, agent: str, *, require_spec: bool | None = None) -> None:
    """Validate an artifact for ``agent``.

    Reviewer artifacts may omit specification metadata only for reviews that are
    explicitly outside a spec-driven workflow. The controller always passes
    ``require_spec=True`` because every controlled run starts from an approved spec.
    """
    if not isinstance(payload, dict):
        raise ContractError("payload must be a JSON object")
    if agent not in ALLOWED_AGENTS:
        raise ContractError("agent must be one of: specifier, planner, orchestrator, reviewer")
    if payload.get("agent") != agent:
        raise ContractError(f"agent field must be '{agent}'")
    if payload.get("status") not in STATUSES:
        raise ContractError("status must be one of ok, blocked, failed, in_progress")

    for key in ("goal", "decision", "result"):
        _nonempty_string(payload, key)
    _array(payload, "evidence")
    if "next_step" in payload:
        _nonempty_string(payload, "next_step")

    required_arrays = {
        "specifier": (
            "scope_in", "scope_out", "requirements", "non_functional_requirements",
            "contracts", "assumptions", "open_questions", "verification_matrix", "risks",
        ),
        "planner": (
            "constraints", "done_criteria", "execution_plan", "risks", "verification",
            "requirement_plan",
        ),
        "orchestrator": (
            "agent_order", "risk_register", "verification_plan", "requirement_assignments",
        ),
        "reviewer": ("findings", "blocking_items", "requirement_coverage"),
    }
    for key in required_arrays[agent]:
        _array(payload, key)

    if agent == "specifier":
        _validate_specifier(payload)
    elif agent == "planner":
        _nonempty_string(payload, "context")
        _validate_spec_metadata(payload)
        _traceable_entries(
            payload,
            "requirement_plan",
            ("requirement_id", "owner", "work", "verification"),
        )
    elif agent == "orchestrator":
        _validate_spec_metadata(payload)
        if payload.get("route") not in {"start", "plan", "implement", "review", "verify"}:
            raise ContractError("route must be one of start, plan, implement, review, verify")
        _traceable_entries(
            payload,
            "requirement_assignments",
            ("requirement_id", "owner", "scope", "verification"),
        )
    else:
        _validate_reviewer(payload, require_spec=require_spec)


def _validate_spec_metadata(payload: dict) -> None:
    _nonempty_string(payload, "specification_id")
    _positive_integer(payload, "specification_revision")


def _validate_specifier(payload: dict) -> None:
    _validate_spec_metadata(payload)
    if payload.get("approval_status") not in {"draft", "needs_clarification", "approved"}:
        raise ContractError("approval_status must be one of draft, needs_clarification, approved")
    if payload["approval_status"] == "needs_clarification" and not payload["open_questions"]:
        raise ContractError("needs_clarification requires at least one open question")
    if payload["approval_status"] == "approved":
        _nonempty_string(payload, "approval_evidence")
        if not payload["requirements"]:
            raise ContractError("approved specifications must define at least one requirement")

    requirement_ids: set[str] = set()
    for index, requirement in enumerate(payload["requirements"]):
        if not isinstance(requirement, dict):
            raise ContractError(f"requirements[{index}] must be an object")
        for key in ("id", "description"):
            value = requirement.get(key)
            if not isinstance(value, str) or not value.strip():
                raise ContractError(f"requirements[{index}].{key} must be a non-empty string")
        acceptance_criteria = requirement.get("acceptance_criteria")
        if not isinstance(acceptance_criteria, list) or not acceptance_criteria:
            raise ContractError(
                f"requirements[{index}].acceptance_criteria must be a non-empty array"
            )
        if any(not isinstance(item, str) or not item.strip() for item in acceptance_criteria):
            raise ContractError(
                f"requirements[{index}].acceptance_criteria must contain non-empty strings"
            )
        if requirement["id"] in requirement_ids:
            raise ContractError(f"requirements[{index}].id must be unique")
        requirement_ids.add(requirement["id"])

    verified_ids: set[str] = set()
    for index, verification in enumerate(payload["verification_matrix"]):
        if not isinstance(verification, dict):
            raise ContractError(f"verification_matrix[{index}] must be an object")
        for key in ("requirement_id", "method", "evidence"):
            value = verification.get(key)
            if not isinstance(value, str) or not value.strip():
                raise ContractError(
                    f"verification_matrix[{index}].{key} must be a non-empty string"
                )
        if verification["requirement_id"] not in requirement_ids:
            raise ContractError(
                f"verification_matrix[{index}].requirement_id must reference a requirement"
            )
        verified_ids.add(verification["requirement_id"])
    if verified_ids != requirement_ids:
        raise ContractError("verification_matrix must cover every requirement")


def _validate_reviewer(payload: dict, *, require_spec: bool | None) -> None:
    if payload.get("decision") not in {"ready", "needs_work"}:
        raise ContractError("reviewer decision must be ready or needs_work")

    has_spec_id = "specification_id" in payload
    has_spec_revision = "specification_revision" in payload
    if has_spec_id != has_spec_revision:
        raise ContractError(
            "reviewer specification_id and specification_revision must be provided together"
        )
    spec_driven = has_spec_id and has_spec_revision
    if require_spec is True and not spec_driven:
        raise ContractError("controlled reviewer output requires specification metadata")
    if spec_driven:
        _validate_spec_metadata(payload)

    coverage_ids = _traceable_entries(
        payload,
        "requirement_coverage",
        ("requirement_id", "status", "evidence"),
        allow_empty=not spec_driven,
    )
    if spec_driven and not coverage_ids:
        raise ContractError("spec-driven reviewer output requires requirement coverage")
    if not spec_driven and coverage_ids:
        raise ContractError("reviewer output without a specification must use empty coverage")
    if any(
        entry["status"] not in {"verified", "unverified", "not_applicable"}
        for entry in payload["requirement_coverage"]
    ):
        raise ContractError(
            "requirement_coverage status must be verified, unverified, or not_applicable"
        )

    for index, finding in enumerate(payload["findings"]):
        if not isinstance(finding, dict):
            raise ContractError(f"findings[{index}] must be an object")
        for key in ("severity", "file", "summary", "impact"):
            value = finding.get(key)
            if not isinstance(value, str) or not value.strip():
                raise ContractError(f"findings[{index}].{key} must be a non-empty string")
        requirement_ids = finding.get("requirement_ids")
        if not isinstance(requirement_ids, list) or any(
            not isinstance(item, str) or not item.strip() for item in requirement_ids
        ):
            raise ContractError(
                f"findings[{index}].requirement_ids must be an array of non-empty strings"
            )
        if spec_driven and not requirement_ids:
            raise ContractError(
                f"findings[{index}].requirement_ids must not be empty for spec-driven reviews"
            )
        if not spec_driven and requirement_ids:
            raise ContractError(
                f"findings[{index}].requirement_ids must be empty without a specification"
            )


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--agent", required=True, choices=sorted(ALLOWED_AGENTS))
    parser.add_argument("--input", required=True, type=Path)
    args = parser.parse_args()
    try:
        with args.input.open(encoding="utf-8") as handle:
            payload = json.load(handle)
        validate_payload(payload, args.agent)
    except (OSError, json.JSONDecodeError) as error:
        message = error.msg if isinstance(error, json.JSONDecodeError) else str(error)
        raise SystemExit(f"input must be valid JSON: {message}") from error
    except ContractError as error:
        raise SystemExit(str(error)) from error
    print("valid")


if __name__ == "__main__":
    main()
