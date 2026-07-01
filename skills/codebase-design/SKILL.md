---
name: codebase-design
description: Establish architecture vocabulary and choose deep seams for a codebase.
---

# Codebase Design

Use this when you need architecture vocabulary before proposing refactors.

## Rules

- Use the project's domain language first.
- Prefer the terms `module`, `interface`, `seam`, `adapter`, `depth`, `locality`, and `leverage`.
- Apply the deletion test: if removing a piece only moves complexity around, it is probably shallow.
- Prefer the highest useful seam, not the lowest one.
- Do not propose interfaces before you understand the friction.
