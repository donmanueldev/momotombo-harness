# Orchestration Output Contract

This repository uses JSON-only, machine-readable output for orchestration and review agents. It keeps decisions deterministic, reviewable, and safe to append to the audit trail.

## Required envelope

All outputs from `planner`, `orchestrator`, and `reviewer` must be a single JSON object with these top-level fields:

- `agent`: emitter agent name (`planner`, `orchestrator`, `reviewer`)
- `goal`: one-line objective statement
- `status`: `ok` | `blocked` | `failed` | `in_progress`
- `decision`: short human-readable summary of what was decided
- `evidence`: list of artifact paths or command references
- `next_step`: optional next action or next owner
- `result`: short outcome phrase

`goal`, `decision`, and `result` must be non-empty strings. `evidence` must be an array. `status` must be one of `ok`, `blocked`, `failed`, or `in_progress`.

## `planner` payload fields

- `context`: current request context
- `constraints`: explicit blockers, policies, or limits
- `done_criteria`: measurable completion criteria
- `execution_plan`: ordered agent recommendations
- `risks`: unresolved risks
- `verification`: how the result should be validated

Example (JSON):

```json
{
  "agent": "planner",
  "goal": "Add orchestration contract",
  "status": "ok",
  "decision": "Proceed with structured handoff format",
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
  "next_step": "Run implementation tasks",
  "result": "contract and logger introduced"
}
```

## `orchestrator` payload fields

- `route`: `start`, `plan`, `implement`, `review`, `verify`
- `agent_order`: ordered list of specialist assignments
- `risk_register`: unresolved and known risks
- `verification_plan`: explicit checks before handoff

## `reviewer` payload fields

- `findings`: list with `severity`, `file`, `summary`, `impact`
- `decision`: `ready` or `needs_work`
- `blocking_items`: required follow-ups before close

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
