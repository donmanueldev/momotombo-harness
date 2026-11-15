---
name: commit
description: Commit the current changes as English Conventional Commits, run by the git-ops subagent on haiku. Does not push.
context: fork
agent: git-ops
model: haiku
disable-model-invocation: true
argument-hint: "[optional note: scope, ticket, or what to include]"
---

> Codex: run these steps through the `git-ops` custom agent (spawn it by name), or follow them directly.

Commit the current changes in the git repository at the current working directory.

User note (may be empty): $ARGUMENTS

1. Inspect `git status --short`, the current branch, and the full diff (staged and unstaged).
2. If the branch is `main`, `master`, or `develop`, create a correctly prefixed Git Flow branch first (`feature/`, `bugfix/`, `chore/`, `docs/`, `refactor/`), named after the change.
3. Group the changes by intent. Make one commit per logical change, staging explicit paths only. Never stage secrets, `.env*`, or unrelated files. Report anything you left out and why.
4. Write each message as `type(scope): imperative summary` in English, ≤ 72 chars, with a body only when the why isn't obvious. Add no attribution trailers.
5. Do not push.

Return the commits created (hash and subject) and any files left uncommitted.
