# Domain docs

## Repository domain posture

- **Model**: single-context
- **Primary source of truth for scope/process**: `AGENTS.md` and `custom-instructions.md`
- **ADR location**: `docs/adr/` (create/extend as architectural decisions are introduced)
- **Long-form intake/roadmaps**: keep aligned with this directory and `docs/orchestration-task-requests.md`

## Practical rules

- Prefer one context path at a time for this repo unless a multi-product split is introduced.
- If the repo grows into multiple contexts, move to `CONTEXT-MAP.md` and per-area `CONTEXT.md` files.
