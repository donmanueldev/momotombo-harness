---
name: improve-codebase-architecture
description: Find deepening opportunities in a codebase and propose the highest-leverage refactors.
---

# Improve Codebase Architecture

Use this when the codebase feels hard to navigate, test, or extend.

## Rules

- Read the project glossary and any ADRs before suggesting changes.
- Look for shallow modules, hidden coupling, low locality, and unclear seams.
- Use the deletion test to decide whether a refactor is real leverage or just movement.
- Prefer a short ranked list of opportunities over a huge refactor dump.
- When the user picks one, hand off to `grill-with-docs` or the relevant implementation skill.
