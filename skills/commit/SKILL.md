---
name: commit
description: Commit the current changes as English Conventional Commits, run by the git-ops subagent on haiku. Does not push.
context: fork
agent: git-ops
model: haiku
disable-model-invocation: true
license: MIT
metadata:
  author: momotombo
  version: "1.0.0"
argument-hint: "[optional note: scope, ticket, or what to include]"
---

> Codex: spawn the `git-ops` custom agent by name and give it these steps. Don't follow them directly: the commit and PR formats live in that agent.

Commit the current changes in the git repository at the current working directory.

User note (may be empty): $ARGUMENTS

1. Inspect `git status --short`, the current branch, and the full diff (staged and unstaged).
2. If the branch is `main`, `master`, or `develop`, create a correctly prefixed Git Flow branch first (`feature/`, `bugfix/`, `chore/`, `docs/`, `refactor/`), named after the change. If the current branch is about something else (its prefix or commits don't match this change), stop and ask before committing.
3. Group the changes by intent. Make one commit per logical change, staging explicit paths only. Never stage secrets, `.env*`, or unrelated files. Report anything you left out and why.
4. Write each message as the **Commits** section of the `git-ops` agent defines (English Conventional Commits, ≤ 72 chars, body only when the why isn't obvious). Add no attribution trailers.
5. Do not push.

Return the commits created (hash and subject) and any files left uncommitted.
