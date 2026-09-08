# Project memory

Use this document to minimize context while retaining the information needed
to start work correctly. It is a versioned project index, not a transcript and
not a replacement for repository instructions.

## Start a task

1. Read `AGENTS.md` and this document.
2. Identify the requested outcome, affected repository/module, constraints,
   and observable success criteria. Ask one focused question only if one of
   those facts is materially unknown.
3. Read the context file for the affected module when one exists. Inspect code
   and current evidence before selecting skills.
4. Load only the skills that alter the task's decisions. Do not load domain
   skills during this intake unless the task itself requires their guidance.
5. For material changes, continue through specification, approval, planning,
   routing, execution, review, and verification as defined in
   `docs/orchestration-task-requests.md`.

Completion criterion: before planning or dispatching work, the task has a
stated objective, bounded scope, applicable constraints, success criteria,
and a justified list of skills to load.

## Generate the factual map

Run this from a repository root on its first intake or after a structural
change:

```bash
bash scripts/project-memory.sh initialize
```

It creates `docs/project-map.md` from paths and manifest names only. It never
reads environment files, secrets, logs, or dependency trees. Use `refresh` to
replace the generated map deliberately; `--dry-run` previews the target.

## What belongs where

| Artifact | Holds | Read when |
| --- | --- | --- |
| `AGENTS.md` | Mandatory project behavior and commands | Every task |
| This file | Stable project map and intake sequence | Every task, after `AGENTS.md` |
| `docs/project-map.md` | Generated repository structure and stack signals | First intake or after a structural change |
| `docs/context/<module>.md` | Module vocabulary, architecture, contracts, and non-obvious constraints | That module is in scope |
| `docs/decisions/<id>.md` | A durable decision, its reason, and consequences | The decision or affected seam is in scope |
| Session memory | Short-lived investigation notes | Only when it accelerates a related task; verify drift-prone facts |

Do not place secrets, credentials, personal data, raw provider output, or
large logs in project memory. Link to durable evidence instead.

## Current map

- `skills/`: local, task-specific Codex skills; choose only those relevant to
  the objective.
- `.codex/agents/`: read-only planning/review roles and specialist profiles.
- `docs/orchestration-task-requests.md`: intake fields and spec-driven routing.
- `docs/orchestration-output-contract.md`: machine-readable phase contracts
  and the local audit/controller boundary.
- `scripts/` and `tests/`: sync, validation, audit helpers, and Bats coverage.

Create a module context file only after the module has recurring terminology,
contracts, or constraints that cannot be cheaply rediscovered from code. Keep
it concise and update it in the same change that alters its facts.
