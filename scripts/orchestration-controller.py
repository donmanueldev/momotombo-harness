#!/usr/bin/env python3
"""Local, effect-free orchestration state controller.

This controller deliberately records and validates work; it never invokes an
agent, shell command, network request, hook, or Git operation.  A root agent
can use its durable dispatch records as the only bridge to an approved human
write, while host permissions remain authoritative.
"""
from __future__ import annotations

import argparse
import fcntl
import hashlib
import json
import os
import re
import sys
import tempfile
from datetime import datetime, timezone
from pathlib import Path

from orchestration_contract import ContractError, validate_payload

STATES = ("intake", "specify", "awaiting_approval", "plan", "route", "execute", "review", "verify", "completed", "blocked", "failed", "cancelled")
TERMINAL = {"completed", "blocked", "failed", "cancelled"}
NEXT = {"intake": {"specify"}, "specify": {"awaiting_approval"}, "awaiting_approval": {"plan"}, "plan": {"route", "blocked", "failed", "cancelled"}, "route": {"execute", "blocked", "failed", "cancelled"}, "execute": {"review", "blocked", "failed", "cancelled"}, "review": {"verify", "blocked", "failed", "cancelled"}, "verify": {"completed", "blocked", "failed", "cancelled"}}
READ_ONLY = {"specifier", "planner", "reviewer", "orchestrator"}
FORBIDDEN_EFFECTS = {"commit", "push", "deploy", "publish", "network", "service", "hook", "destructive"}
RUN_RE = re.compile(r"^[A-Za-z0-9][A-Za-z0-9._-]{0,127}$")
EVENT_FIELDS = {
    "schema_version", "run_id", "sequence", "timestamp", "phase",
    "previous_state", "state", "actor", "decision", "evidence", "result",
    "next_step", "state_digest",
}
FORBIDDEN_JOURNAL_FIELDS = {"prompt", "secret", "token", "pii", "raw_log"}
EVENT_TEXT = {
    "start": ("run initialized", "advance to specify"),
    "stop": None,
    "transition": None,
    "authorize": ("material scope recorded", "dispatch approved work"),
    "dispatch_ready": ("dispatch recorded", "root invokes approved specialist"),
    "dispatch_completed": ("dispatch completed", "advance when all assignments complete"),
    "record_output": ("validated output reference", "advance phase"),
    "complete": ("all requirements verified", "none"),
}


class ControllerError(Exception):
    pass


def utcnow():
    return datetime.now(timezone.utc).replace(microsecond=0).isoformat().replace("+00:00", "Z")


def load_json(path):
    try:
        with open(path, encoding="utf-8") as handle:
            value = json.load(handle)
    except (OSError, json.JSONDecodeError) as error:
        raise ControllerError(f"cannot read JSON: {path}") from error
    if not isinstance(value, dict):
        raise ControllerError("JSON artifact must be an object")
    return value


def require_string(value, field):
    if not isinstance(value, str) or not value.strip():
        raise ControllerError(f"{field} must be a non-empty string")
    return value


def run_dir(root, run_id):
    if not RUN_RE.fullmatch(run_id):
        raise ControllerError("run_id contains unsupported characters")
    return Path(root) / ".codex" / "orchestration" / "runs" / run_id


def state_path(directory): return directory / "state.json"
def journal_path(directory): return directory / "events.jsonl"


def atomic_json(path, value):
    path.parent.mkdir(mode=0o700, parents=True, exist_ok=True)
    fd, temporary = tempfile.mkstemp(prefix=".state-", dir=path.parent)
    try:
        with os.fdopen(fd, "w", encoding="utf-8") as handle:
            json.dump(value, handle, ensure_ascii=False, sort_keys=True, separators=(",", ":"))
            handle.write("\n")
            handle.flush(); os.fsync(handle.fileno())
        os.chmod(temporary, 0o600)
        os.replace(temporary, path)
        os.chmod(path, 0o600)
    finally:
        if os.path.exists(temporary): os.unlink(temporary)


def state_digest(state):
    encoded = json.dumps(
        state,
        ensure_ascii=False,
        sort_keys=True,
        separators=(",", ":"),
    ).encode()
    return hashlib.sha256(encoded).hexdigest()


def contains_forbidden_journal_field(value):
    if isinstance(value, dict):
        return any(
            str(key).lower() in FORBIDDEN_JOURNAL_FIELDS
            or contains_forbidden_journal_field(child)
            for key, child in value.items()
        )
    if isinstance(value, list):
        return any(contains_forbidden_journal_field(child) for child in value)
    return False


def expected_event_text(event):
    decision = event["decision"]
    if decision == "transition":
        return f"advanced to {event['state']}", "continue from durable state"
    if decision == "stop":
        return f"run marked {event['state']}", "inspect durable state"
    return EVENT_TEXT.get(decision)


def append_event(directory, state, actor, decision, previous=None, result="", next_step=""):
    state["sequence"] += 1
    event = {"schema_version": 2, "run_id": state["run_id"], "sequence": state["sequence"], "timestamp": utcnow(), "phase": state["state"], "previous_state": previous, "state": state["state"], "actor": actor, "decision": decision, "evidence": [], "result": result, "next_step": next_step, "state_digest": state_digest(state)}
    path = journal_path(directory); path.parent.mkdir(mode=0o700, parents=True, exist_ok=True)
    descriptor = os.open(path, os.O_APPEND | os.O_CREAT | os.O_WRONLY, 0o600)
    with os.fdopen(descriptor, "a", encoding="utf-8") as handle:
        handle.write(json.dumps(event, ensure_ascii=False, sort_keys=True, separators=(",", ":")) + "\n")
        handle.flush(); os.fsync(handle.fileno())
    os.chmod(path, 0o600)
    return event


def read_state(directory):
    path = state_path(directory)
    if not path.is_file(): raise ControllerError("run state not found")
    state = load_json(path)
    if state.get("state") not in STATES: raise ControllerError("stored state is invalid")
    return state


def ensure_active(state):
    if state["state"] in TERMINAL:
        raise ControllerError("terminal runs cannot mutate")


def write_state(directory, state): atomic_json(state_path(directory), state)


def requirements_from_spec(spec):
    try:
        validate_payload(spec, "specifier")
    except ContractError as error:
        raise ControllerError(f"invalid specifier artifact: {error}") from error
    if spec["approval_status"] != "approved":
        raise ControllerError("start requires an approved specifier artifact")
    identifiers = [item["id"] for item in spec["requirements"]]
    return spec["specification_id"], spec["specification_revision"], identifiers


def transition(directory, state, target, actor="controller", reason="", next_step=""):
    current = state["state"]
    ensure_active(state)
    if target not in STATES or target not in NEXT.get(current, set()): raise ControllerError(f"illegal transition: {current} -> {target}")
    if target in {"plan", "route", "execute", "review", "verify"} and not state.get("specification_approved"):
        raise ControllerError("approved specification is required before planning")
    required_output = {"route": "planner", "execute": "orchestrator", "verify": "reviewer"}.get(target)
    if required_output and required_output not in state["outputs"]:
        raise ControllerError(f"validated {required_output} output is required before {target}")
    if target == "review":
        assignments = state.get("route_assignments", {})
        dispatches = state.get("dispatches", {})
        if set(assignments) != set(state["requirements"]): raise ControllerError("validated route assignments are required before review")
        completed = {req for item in dispatches.values() if item["status"] == "completed" for req in item["requirements"]}
        if completed != set(state["requirements"]): raise ControllerError("every routed requirement needs one completed dispatch before review")
    if target == "completed":
        raise ControllerError("completed is only reachable from verify")
    state["state"] = target; state["updated_at"] = utcnow()
    append_event(directory, state, actor, "transition", current, f"advanced to {target}", "continue from durable state")
    write_state(directory, state)


def stop(directory, state, target, reason, next_step):
    if target not in {"blocked", "failed", "cancelled"}: raise ControllerError("invalid terminal state")
    require_string(reason, "reason"); require_string(next_step, "next_step")
    if state["state"] in TERMINAL: raise ControllerError("terminal runs cannot transition")
    previous = state["state"]; state.update({"state": target, "terminal_reason": reason, "next_step": next_step, "updated_at": utcnow()})
    append_event(directory, state, "controller", "stop", previous, f"run marked {target}", "inspect durable state"); write_state(directory, state)


def parse_requirements(value, known):
    identifiers = [x for x in value.split(",") if x]
    if not identifiers or len(set(identifiers)) != len(identifiers) or set(identifiers) - set(known): raise ControllerError("requirements must be unique approved requirement IDs")
    return identifiers


def command_start(args):
    directory = run_dir(args.root, args.run_id)
    if state_path(directory).exists(): raise ControllerError("run already exists")
    spec = load_json(args.spec); spec_id, revision, requirements = requirements_from_spec(spec)
    state = {"schema_version": 1, "run_id": args.run_id, "specification_id": spec_id, "specification_revision": revision, "requirements": requirements, "state": "intake", "specification_approved": False, "sequence": 0, "authorizations": [], "dispatches": {}, "outputs": {}, "created_at": utcnow(), "updated_at": utcnow()}
    append_event(directory, state, "controller", "start", None, "run initialized", "advance to specify")
    write_state(directory, state); print(json.dumps(state, sort_keys=True))


def command_advance(args):
    directory = run_dir(args.root, args.run_id); state = read_verified_state(directory)
    if args.to == "completed":
        if state["state"] != "verify": raise ControllerError("completed requires verify")
        coverage = state.get("review_coverage", {})
        if set(coverage) != set(state["requirements"]) or any(v != "verified" for v in coverage.values()): raise ControllerError("completed requires verified coverage for every requirement")
        previous = state["state"]; state.update({"state": "completed", "updated_at": utcnow(), "terminal_reason": "verified", "next_step": "none"})
        append_event(directory, state, "controller", "complete", previous, "all requirements verified", "none"); write_state(directory, state); return
    transition(directory, state, args.to, reason=args.reason, next_step=args.next_step)


def command_approve(args):
    directory = run_dir(args.root, args.run_id); state = read_verified_state(directory)
    if state["state"] != "awaiting_approval": raise ControllerError("specification approval is only accepted while awaiting_approval")
    require_string(args.evidence, "evidence")
    state["specification_approved"] = True; state["approval_evidence"] = args.evidence
    transition(directory, state, "plan", actor="human", reason="specification approved", next_step="record planner output")


def command_authorize(args):
    directory = run_dir(args.root, args.run_id); state = read_verified_state(directory)
    ensure_active(state)
    require_string(args.evidence, "evidence"); requirements = parse_requirements(args.requirements, state["requirements"])
    if not args.paths and not args.responsibility: raise ControllerError("authorization requires paths or responsibility")
    paths = [p for p in args.paths.split(",") if p] if args.paths else []
    if any(Path(p).is_absolute() or ".." in Path(p).parts for p in paths): raise ControllerError("authorization paths must be repository-relative")
    authorization = {"id": args.authorization_id, "requirements": requirements, "paths": paths, "responsibility": args.responsibility or "", "evidence": args.evidence, "expires_at": args.expires_at, "revision": state["specification_revision"]}
    if any(item["id"] == args.authorization_id for item in state["authorizations"]): raise ControllerError("authorization already exists")
    state["authorizations"].append(authorization); state["updated_at"] = utcnow(); append_event(directory, state, "human", "authorize", result="material scope recorded", next_step="dispatch approved work"); write_state(directory, state)


def valid_authorization(state, args, requirements, paths):
    now = datetime.now(timezone.utc)
    for item in state["authorizations"]:
        if item["id"] != args.authorization_id or item["revision"] != state["specification_revision"]: continue
        try: expiry = datetime.fromisoformat(item["expires_at"].replace("Z", "+00:00"))
        except ValueError: continue
        if expiry <= now or not set(requirements).issubset(item["requirements"]): continue
        if item["paths"] and not set(paths).issubset(item["paths"]): continue
        if not item["paths"] and item["responsibility"] != args.responsibility: continue
        if args.responsibility and item["responsibility"] != args.responsibility: continue
        if item["evidence"]: return True
    return False


def command_dispatch(args):
    directory = run_dir(args.root, args.run_id); state = read_verified_state(directory)
    if state["state"] != "execute": raise ControllerError("dispatch is only accepted in execute")
    if args.owner in READ_ONLY or args.owner not in {"backend", "web", "infra", "mobile"}: raise ControllerError("owner is not an allowed first-layer specialist")
    if args.parent: raise ControllerError("nested dispatch is prohibited")
    if args.effect in FORBIDDEN_EFFECTS: raise ControllerError("effect kind is prohibited")
    if args.effect != "local_write": raise ControllerError("controller only records local_write dispatches")
    requirements = parse_requirements(args.requirements, state["requirements"]); paths = [p for p in args.paths.split(",") if p]
    if len(requirements) != 1: raise ControllerError("a dispatch must own exactly one requirement")
    if (not paths and not args.responsibility) or any(Path(p).is_absolute() or ".." in Path(p).parts for p in paths): raise ControllerError("dispatch requires repository-relative paths or responsibility")
    if args.assignment_id in state["dispatches"]: raise ControllerError("duplicate dispatch")
    assigned = {req for item in state["dispatches"].values() for req in item["requirements"]}
    if assigned.intersection(requirements): raise ControllerError("requirements already have an owner")
    route_assignment = state.get("route_assignments", {}).get(requirements[0])
    if not route_assignment or route_assignment["owner"] != args.owner or not set(paths).issubset(route_assignment["paths"]) or (route_assignment.get("responsibility", "") and route_assignment["responsibility"] != args.responsibility):
        raise ControllerError("dispatch does not match the validated route assignment")
    for item in state["dispatches"].values():
        overlap = any(p == q or p.startswith(q + "/") or q.startswith(p + "/") for p in paths for q in item["paths"])
        if overlap and not args.depends_on: raise ControllerError("overlapping scope requires declared dependency")
    if not valid_authorization(state, args, requirements, paths): raise ControllerError("no valid scoped authorization")
    state["dispatches"][args.assignment_id] = {"owner": args.owner, "requirements": requirements, "paths": paths, "responsibility": args.responsibility or "", "status": "ready", "depends_on": args.depends_on or ""}; state["updated_at"] = utcnow()
    append_event(directory, state, "controller", "dispatch_ready", result="dispatch recorded", next_step="root invokes approved specialist"); write_state(directory, state)


def command_complete_dispatch(args):
    directory = run_dir(args.root, args.run_id); state = read_verified_state(directory); ensure_active(state)
    item = state["dispatches"].get(args.assignment_id)
    if not item or item["status"] != "ready": raise ControllerError("dispatch is not ready for completion")
    item["status"] = "completed"; state["updated_at"] = utcnow()
    append_event(directory, state, "root", "dispatch_completed", result="dispatch completed", next_step="advance when all assignments complete"); write_state(directory, state)


def command_record_output(args):
    directory = run_dir(args.root, args.run_id); state = read_verified_state(directory); payload = load_json(args.input)
    ensure_active(state)
    expected_state = {"planner": "plan", "orchestrator": "route", "reviewer": "review"}[args.agent]
    if state["state"] != expected_state: raise ControllerError(f"{args.agent} output is only accepted in {expected_state}")
    try:
        validate_payload(payload, args.agent, require_spec=True)
    except ContractError as error:
        raise ControllerError(f"invalid {args.agent} artifact: {error}") from error
    if payload.get("specification_id") != state["specification_id"] or payload.get("specification_revision") != state["specification_revision"]: raise ControllerError("output specification does not match run")
    if args.agent == "reviewer":
        if payload.get("status") != "ok" or payload.get("decision") != "ready": raise ControllerError("reviewer output must be ready")
        coverage = payload.get("requirement_coverage")
        if not isinstance(coverage, list): raise ControllerError("reviewer output requires requirement_coverage")
        if any(not isinstance(entry, dict) or not isinstance(entry.get("evidence"), str) or not entry["evidence"].strip() for entry in coverage): raise ControllerError("reviewer coverage requires evidence")
        mapped = {entry.get("requirement_id"): entry.get("status") for entry in coverage if isinstance(entry, dict)}
        if len(mapped) != len(coverage) or set(mapped) != set(state["requirements"]): raise ControllerError("reviewer coverage must exactly match requirements")
        referenced = {
            requirement_id
            for finding in payload["findings"]
            for requirement_id in finding["requirement_ids"]
        }
        if referenced - set(state["requirements"]):
            raise ControllerError("reviewer findings must reference approved requirements")
        state["review_coverage"] = mapped
    elif args.agent in {"planner", "orchestrator"}:
        key = "requirement_plan" if args.agent == "planner" else "requirement_assignments"; entries = payload.get(key)
        mapped = [entry.get("requirement_id") for entry in entries] if isinstance(entries, list) else []
        if set(mapped) != set(state["requirements"]) or len(mapped) != len(set(mapped)): raise ControllerError("phase requirement coverage must exactly match requirements")
        if args.agent == "orchestrator":
            assignments = {}
            for entry in entries:
                owner = entry.get("owner"); scope = entry.get("scope")
                if owner not in {"backend", "web", "infra", "mobile"} or not isinstance(scope, str) or not scope.strip(): raise ControllerError("route assignment requires an allowed owner and file scope")
                if scope.startswith("responsibility:"):
                    responsibility = scope.removeprefix("responsibility:").strip()
                    if not responsibility: raise ControllerError("route responsibility must not be empty")
                    paths = []
                else:
                    responsibility = ""; paths = [path for path in scope.split(",") if path]
                    if not paths or any(Path(path).is_absolute() or ".." in Path(path).parts for path in paths): raise ControllerError("route scope must contain repository-relative files")
                assignments[entry["requirement_id"]] = {"owner": owner, "paths": paths, "responsibility": responsibility}
            state["route_assignments"] = assignments
    state["outputs"][args.agent] = str(Path(args.input)); state["updated_at"] = utcnow(); append_event(directory, state, args.agent, "record_output", result="validated output reference", next_step="advance phase"); write_state(directory, state)


def replay_journal(directory, state):
    expected = 1
    try: lines = journal_path(directory).read_text(encoding="utf-8").splitlines()
    except OSError as error: raise ControllerError("journal not found") from error
    last_digest = None
    for line in lines:
        try:
            event = json.loads(line)
        except json.JSONDecodeError as error:
            raise ControllerError("journal contains invalid JSON") from error
        if not isinstance(event, dict) or set(event) != EVENT_FIELDS:
            raise ControllerError("journal event fields are invalid")
        if event.get("schema_version") != 2:
            raise ControllerError("journal schema version is unsupported")
        if event.get("run_id") != state["run_id"] or event.get("sequence") != expected: raise ControllerError("journal sequence is invalid")
        if not RUN_RE.fullmatch(event["run_id"]):
            raise ControllerError("journal run ID is invalid")
        if not isinstance(event["sequence"], int) or isinstance(event["sequence"], bool):
            raise ControllerError("journal sequence is invalid")
        if not re.fullmatch(r"\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}Z", event.get("timestamp", "")):
            raise ControllerError("journal timestamp is invalid")
        if event.get("state") not in STATES or event.get("phase") != event.get("state"):
            raise ControllerError("journal state is invalid")
        if event.get("previous_state") is not None and event["previous_state"] not in STATES:
            raise ControllerError("journal previous state is invalid")
        if event.get("evidence") != []:
            raise ControllerError("journal evidence must be empty")
        for field in ("timestamp", "actor", "decision", "result", "next_step"):
            if not isinstance(event.get(field), str):
                raise ControllerError(f"journal {field} must be a string")
        if event["actor"] not in READ_ONLY | {"controller", "human", "root"}:
            raise ControllerError("journal actor is invalid")
        expected_text = expected_event_text(event)
        if expected_text is None or (event["result"], event["next_step"]) != expected_text:
            raise ControllerError("journal metadata is invalid")
        if not isinstance(event.get("state_digest"), str) or not re.fullmatch(r"[0-9a-f]{64}", event["state_digest"]):
            raise ControllerError("journal state digest is invalid")
        if contains_forbidden_journal_field(event):
            raise ControllerError("journal contains forbidden fields")
        last_digest = event["state_digest"]
        expected += 1
    if state["sequence"] != expected - 1:
        raise ControllerError("state and journal sequences differ")
    if last_digest != state_digest(state):
        raise ControllerError("state does not match the journal digest")
    return state


def read_verified_state(directory):
    return replay_journal(directory, read_state(directory))


def command_validate(args):
    directory = run_dir(args.root, args.run_id); state = read_state(directory)
    replay_journal(directory, state)
    print("valid")


def command_status(args): print(json.dumps(read_state(run_dir(args.root, args.run_id)), sort_keys=True))
def command_resume(args):
    directory = run_dir(args.root, args.run_id); state = replay_journal(directory, read_state(directory))
    print(json.dumps({"run_id": state["run_id"], "state": state["state"], "next_step": state.get("next_step", "continue from current state")}, sort_keys=True))
def command_cancel(args): stop(run_dir(args.root, args.run_id), read_verified_state(run_dir(args.root, args.run_id)), "cancelled", args.reason, args.next_step)
def command_stop(args): stop(run_dir(args.root, args.run_id), read_verified_state(run_dir(args.root, args.run_id)), args.to, args.reason, args.next_step)


def parser():
    root = argparse.ArgumentParser(description=__doc__); root.add_argument("--root", default="."); sub = root.add_subparsers(dest="command", required=True)
    p = sub.add_parser("start"); p.add_argument("--run-id", required=True); p.add_argument("--spec", required=True); p.set_defaults(func=command_start)
    p = sub.add_parser("advance"); p.add_argument("--run-id", required=True); p.add_argument("--to", required=True, choices=STATES); p.add_argument("--reason", default=""); p.add_argument("--next-step", default=""); p.set_defaults(func=command_advance)
    p = sub.add_parser("approve"); p.add_argument("--run-id", required=True); p.add_argument("--evidence", required=True); p.set_defaults(func=command_approve)
    p = sub.add_parser("authorize"); p.add_argument("--run-id", required=True); p.add_argument("--authorization-id", required=True); p.add_argument("--requirements", required=True); p.add_argument("--paths", default=""); p.add_argument("--responsibility", default=""); p.add_argument("--evidence", required=True); p.add_argument("--expires-at", required=True); p.set_defaults(func=command_authorize)
    p = sub.add_parser("dispatch"); p.add_argument("--run-id", required=True); p.add_argument("--assignment-id", required=True); p.add_argument("--owner", required=True); p.add_argument("--requirements", required=True); p.add_argument("--paths", default=""); p.add_argument("--responsibility", default=""); p.add_argument("--authorization-id", required=True); p.add_argument("--effect", required=True); p.add_argument("--depends-on", default=""); p.add_argument("--parent", default=""); p.set_defaults(func=command_dispatch)
    p = sub.add_parser("complete-dispatch"); p.add_argument("--run-id", required=True); p.add_argument("--assignment-id", required=True); p.set_defaults(func=command_complete_dispatch)
    p = sub.add_parser("record-output"); p.add_argument("--run-id", required=True); p.add_argument("--agent", required=True, choices=("planner", "orchestrator", "reviewer")); p.add_argument("--input", required=True); p.set_defaults(func=command_record_output)
    for name, fn in (("status", command_status), ("resume", command_resume), ("validate", command_validate)):
        p = sub.add_parser(name); p.add_argument("--run-id", required=True); p.set_defaults(func=fn)
    p = sub.add_parser("cancel"); p.add_argument("--run-id", required=True); p.add_argument("--reason", required=True); p.add_argument("--next-step", required=True); p.set_defaults(func=command_cancel)
    p = sub.add_parser("stop"); p.add_argument("--run-id", required=True); p.add_argument("--to", required=True, choices=("blocked", "failed")); p.add_argument("--reason", required=True); p.add_argument("--next-step", required=True); p.set_defaults(func=command_stop)
    return root


if __name__ == "__main__":
    try:
        args = parser().parse_args()
        directory = run_dir(args.root, args.run_id)
        directory.parent.mkdir(mode=0o700, parents=True, exist_ok=True)
        with open(directory.parent / ".lock", "a", encoding="utf-8") as lock:
            os.chmod(lock.name, 0o600)
            fcntl.flock(lock.fileno(), fcntl.LOCK_EX)
            args.func(args)
    except ControllerError as error:
        print(f"error: {error}", file=sys.stderr); sys.exit(1)
