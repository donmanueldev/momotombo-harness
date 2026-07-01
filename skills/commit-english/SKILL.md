---
name: commit-english
description: Write git commit messages in English with a clear scope and intent.
---

# Commit in English

Use this when the user wants help writing commits, splitting work into coherent commits, or phrasing commit messages in English.

## Rules

- Inspect the diff first.
- Group changes by intent, not by file.
- Write commit subjects in English, imperative mood, and conventional-commit form when possible: `type(scope): summary`.
- Keep the subject short and specific.
- Use a body only when the "why" is not obvious from the diff.
- If there are multiple logical commits, list them in order and explain the purpose of each one.
- If the user wants the commit created, produce the exact `git commit` command after the message.
