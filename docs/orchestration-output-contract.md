# Orchestration Output Contract

This repository uses JSON-only, machine-readable output for orchestration and review agents. It keeps decisions deterministic, reviewable, and safe to append to the audit trail.

## Required envelope

All outputs from `specifier`, `planner`, `orchestrator`, and `reviewer` must be a single JSON object with these top-level fields:

- `agent`: emitter agent name (`specifier`, `planner`, `orchestrator`, `reviewer`)
- `goal`: one-line objective statement
- `status`: `ok` | `blocked` | `failed` | `in_progress`
- `decision`: short human-readable summary of what was decided (`ready` or `needs_work` for `reviewer`)
- `evidence`: list of artifact paths or command references
- `next_step`: optional next action or next owner
- `result`: short outcome phrase

`goal`, `decision`, and `result` must be non-empty strings. `evidence` must be an array. `status` must be one of `ok`, `blocked`, `failed`, or `in_progress`.

## `planner` payload fields

- `specification_id`: ID of the approved specification
- `specification_revision`: approved revision of that specification
- `context`: current request context
- `constraints`: explicit blockers, policies, or limits
- `done_criteria`: measurable completion criteria
- `execution_plan`: ordered agent recommendations
- `risks`: unresolved risks
- `verification`: how the result should be validated
- `requirement_plan`: objects with `requirement_id`, `owner`, `work`, and `verification`; covers every approved requirement exactly once

## `specifier` payload fields

Use this agent before planning a complex product change. Its output is a draft until the user explicitly approves it; planning and implementation must not silently fill in unresolved decisions.

- `approval_status`: `draft` | `needs_clarification` | `approved`
- `specification_id`: stable ID for this product decision, retained across revisions
- `specification_revision`: positive integer revision
- `approval_evidence`: required only when `approval_status` is `approved`; link or reference to the user or product-owner approval
- `scope_in`: behavior and surfaces included
- `scope_out`: explicit non-goals
- `requirements`: objects with stable `id`, `description`, and measurable `acceptance_criteria`
- `non_functional_requirements`: security, performance, accessibility, reliability, observability, or compatibility constraints when applicable
- `contracts`: public API, event, persistence, migration, or integration effects; empty when none apply
- `assumptions`: unverified assumptions that do not block a draft
- `open_questions`: decisions or evidence required before approval
- `verification_matrix`: one or more entries per requirement, each containing `requirement_id`, `method`, and `evidence`
- `risks`: known delivery, data, security, migration, or rollback risks

Best-practice invariants:

- A requirement ID must be unique, appear in the verification matrix, and have at least one acceptance criterion.
- `needs_clarification` requires at least one open question.
- `approved` may only record approval already supplied by the user or an authoritative product owner, with `approval_evidence`; the specifier itself never grants it.
- The planner, orchestrator, and reviewer preserve requirement IDs in their plans, routes, and findings.

Example (JSON):

```json
{
  "agent": "planner",
  "goal": "Add orchestration contract",
  "status": "ok",
  "decision": "Proceed with structured handoff format",
  "specification_id": "SPEC-ORCH-001",
  "specification_revision": 1,
  "evidence": [
    "docs/orchestration-output-contract.md",
    "AGENTS.md"
  ],
  "context": "agent-config repository",
  "constraints": [
    "single-shot edits only",
    "no destructive behavior"
  ],
  "done_criteria": [
    "contract file exists",
    "validation enforces format references"
  ],
  "execution_plan": [
    "update planner/orchestrator outputs",
    "add orchestration logger",
    "add sync validation"
  ],
  "risks": [
    "manual discipline may still break output shape"
  ],
  "verification": [
    "sync.sh validate",
    "manual log append check"
  ],
  "requirement_plan": [
    {
      "requirement_id": "REQ-1",
      "owner": "backend",
      "work": "Add the contract validator",
      "verification": "Bats validation test"
    }
  ],
  "next_step": "Run implementation tasks",
  "result": "contract and logger introduced"
}
```

## `orchestrator` payload fields

- `specification_id`: ID of the approved specification
- `specification_revision`: approved revision of that specification
- `route`: `start`, `plan`, `implement`, `review`, `verify`
- `agent_order`: ordered list of specialist assignments
- `risk_register`: unresolved and known risks
- `verification_plan`: explicit checks before handoff
- `requirement_assignments`: objects with `requirement_id`, `owner`, `scope`, and `verification`; covers every approved requirement exactly once

With `max_depth = 1`, planner and orchestrator return this route to the root agent; they do not create nested agents.

## Local autonomous controller

`scripts/orchestration-controller.py` is the local, deterministic control
plane for a spec-driven run. It does **not** invoke agents, commands, network
services, hooks, Git operations, or destructive actions. Its autonomy is
limited to validating artifacts, state transitions, authorizations, and audit
records. The root agent remains the execution bridge for any approved
specialist work, and host sandbox approval remains authoritative.

Each run is stored under the ignored path
`.codex/orchestration/runs/<run-id>/`:

- `state.json` is atomically replaced with mode `0600` and holds the current
  versioned run state.
- `events.jsonl` is an append-only, mode-`0600` allow-listed audit journal.
  Events contain run ID, monotonic sequence, timestamp, phase/state change,
  actor, decision, result, and next step. Prompts, secrets, tokens, PII, and
  raw logs are not accepted fields.

The lifecycle is:

`intake -> specify -> awaiting_approval -> plan -> route -> execute -> review -> verify -> completed`

At any nonterminal point, a human or controller can stop a run as `blocked`,
`failed`, or `cancelled`, with a reason and next step. Terminal runs reject
further transitions or dispatches. `completed` requires `verify` and reviewer
coverage with every approved requirement marked `verified`.

The root starts a run from an approved specifier artifact, then records the
human approval before planning:

```bash
python3 scripts/orchestration-controller.py start --run-id SPEC-42-r1 --spec /tmp/spec.json
python3 scripts/orchestration-controller.py advance --run-id SPEC-42-r1 --to specify
python3 scripts/orchestration-controller.py advance --run-id SPEC-42-r1 --to awaiting_approval
python3 scripts/orchestration-controller.py approve --run-id SPEC-42-r1 --evidence 'user message reference'
```

`record-output` preserves planner, orchestrator, and reviewer artifacts only
in their respective phase and only when their specification ID/revision and
exact requirement coverage match the run. A reviewer must return `status: ok`,
`decision: ready`, and non-empty evidence for every requirement. An
orchestrator route must name an allowed owner and repository-relative file
scope for every requirement. `advance`, `status`, `validate`, `resume`,
`cancel`, and `stop` are local operations. `resume` first validates the journal
against the durable state; it never invokes or repeats a dispatch.

Before a root agent can mark an execution task ready for a first-layer
specialist, a human must record a durable, scoped authorization. It matches
the run, specification revision, requirement IDs, repository-relative files
or responsibility, evidence reference, and expiry:

```bash
python3 scripts/orchestration-controller.py authorize \
  --run-id SPEC-42-r1 --authorization-id approval-1 --requirements REQ-1 \
  --paths scripts/example.py --evidence 'user message reference' \
  --expires-at 2030-01-01T00:00:00Z
python3 scripts/orchestration-controller.py dispatch \
  --run-id SPEC-42-r1 --assignment-id backend-1 --owner backend \
  --requirements REQ-1 --paths scripts/example.py --effect local_write \
  --authorization-id approval-1
python3 scripts/orchestration-controller.py complete-dispatch \
  --run-id SPEC-42-r1 --assignment-id backend-1
```

Only `backend`, `web`, `infra`, and `mobile` can be named as first-layer
specialists. Each route requirement must have exactly one matching completed
dispatch before a run can enter review. `specifier`, `planner`, `reviewer`, and `orchestrator` stay
read-only. Duplicate requirements/dispatches, nested dispatches, and
overlapping scopes without a declared dependency are rejected. The controller
also rejects commit, push, deploy, publish, network, service, hook, and
destructive effect kinds; it cannot execute effects of any kind.

For recovery, run `validate`, then `resume`; if state and journal disagree,
the controller fails closed. To stop safely, use `cancel` or `stop` with a
reason and next step. Roll back the automation by ceasing use of this
controller and returning to the existing manual root-agent route; retain the
run directory and legacy `.codex/orchestration.jsonl` for audit. The legacy
JSONL helper remains compatible and is intentionally separate from per-run
journals.

## `reviewer` payload fields

- `specification_id`: ID of the approved specification when reviewing spec-driven work
- `specification_revision`: approved revision of that specification when reviewing spec-driven work
- `findings`: list with `severity`, `file`, `summary`, `impact`, and `requirement_ids`
- `decision`: `ready` or `needs_work`
- `blocking_items`: required follow-ups before close
- `requirement_coverage`: objects with `requirement_id`, `status` (`verified` | `unverified` | `not_applicable`), and `evidence`

The root agent must provide the approved requirement IDs to the planner, orchestrator, and reviewer. Their payloads preserve those IDs; validation proves shape and references, while the root compares completeness against the approved spec.

## Logging convention

Append a single JSON object per routing decision to `.codex/orchestration.jsonl` with:

- `timestamp` (ISO-8601 UTC)
- `agent`
- `goal`
- `decision`
- `status`
- `status` must be one of `ok`, `blocked`, `failed`, or `in_progress`.
- `result`
- `next_step`

Recommended command:

```bash
bash scripts/log-orchestration-event.sh \
  --agent orchestrator \
  --goal "Add orchestration output contract" \
  --status <ok|blocked|failed|in_progress> \
  --decision "planner-approved" \
  --result "contract added" \
  --next-step "run sync validate"
```

Recommended validation command:

```bash
bash scripts/validate-orchestration-output.sh \
  --agent planner \
  --input /tmp/route.json
```
