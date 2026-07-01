---
name: codex-extensions
description: Add or update Codex prompts, skills, agents, and local workflow extensions.
---

# Codex Extensions

Use this when adding or changing repository-level Codex surface area: `prompts/`, `skills/`, `.codex/agents/`, `.codex/config.toml`, or the sync/install flow.

## Rules

- Keep each file as small as possible.
- Put reusable task starters in `prompts/`.
- Put recurring workflows in `skills/`.
- Put delegation logic in `.codex/agents/`.
- Update README and AGENTS when the user-facing workflow changes.
- If install or sync behavior changes, update the tests first.
- Keep the repo Codex-only; do not reintroduce Claude or Cursor surfaces.
