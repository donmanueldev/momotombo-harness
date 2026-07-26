---
name: to-tickets
description: Turn a plan or spec into tracer-bullet tickets with blocking edges.
disable-model-invocation: true
---

# To Tickets

Use this when you need execution-ready work slicing from a spec or clear plan.

## Process

- Read the current context and any referenced spec (URL/path/issue).
- Split into vertical slices (schema + API + UI + tests together).
- Assign explicit **Blocked by** relationships for every ticket.
- Keep slices small enough to finish in one fresh context.

## Ticket format (for each ticket)

- **Title**: short and decision-clear
- **Blocked by**: blockers or `None`
- **Goal**: end-to-end behavior delivered
- **Acceptance criteria**: testable assertions
- **Status**: `ready-for-agent`

### Optional refactoring exception

If one change is mechanically wide (`rename`, shared type migration), plan as:
`expand → migrate in batches → contract`.

## Publish

- Tracker unknown: create `.scratch/issues/01-.../.md` one file per ticket.
- Real tracker: open one issue per ticket with tracker-native blocking edges.
