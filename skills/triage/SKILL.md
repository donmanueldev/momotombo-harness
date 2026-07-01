---
name: triage
description: Triage incoming issues or PRs into clear states and agent-ready briefs.
---

# Triage

Use this when a project has incoming bugs, requests, or PRs that need sorting.

## Rules

- Classify the item first: bug, enhancement, or other.
- Move it through a small state machine: needs-triage, needs-info, ready-for-agent, ready-for-human, or wontfix.
- Ask only for the minimum missing information.
- If the item is ready for an agent, write a concise brief with context, goal, constraints, and verification.
- If the item is a PR, review the diff and comment on the attached code, not just the description.
