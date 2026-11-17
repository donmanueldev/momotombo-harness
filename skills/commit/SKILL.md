---
name: commit
description: Commit the current changes as English Conventional Commits, run by the git-ops subagent on haiku. Does not push.
context: fork
agent: git-ops
model: haiku
disable-model-invocation: true
license: MIT
metadata:
  author: momotombodevs
  version: "1.0.0"
argument-hint: "[optional note: scope, ticket, or what to include]"
---

> Codex: spawn the `git-ops` custom agent by name and give it these steps. Don't follow them directly: the commit and PR formats live in that agent.

Commit the current changes in the git repository at the current working directory.

User note (may be empty): $ARGUMENTS

1. Follow the **Before acting** and **Branches** sections of the `git-ops` agent: inspect the diff, branch off `main`/`master`/`develop`, and stop if the branch is about something else.
2. Group the changes by intent and make one commit per logical change, staging explicit paths only. Report anything you left out and why.
3. Write each message as the **Commits** section of the `git-ops` agent defines.
4. Do not push.

Return the commits created (hash and subject) and any files left uncommitted.
