---
name: open-pr
description: Commit any pending changes, push the branch, and open a GitHub pull request (English title, Spanish body), run by the git-ops subagent on haiku.
context: fork
agent: git-ops
model: haiku
disable-model-invocation: true
license: MIT
metadata:
  author: momotombo
  version: "1.0.0"
argument-hint: "[optional: target branch, ticket, or context for the description]"
---

> Codex: spawn the `git-ops` custom agent by name and give it these steps. Don't follow them directly: the commit and PR formats live in that agent.

Open a pull request for the work in the git repository at the current working directory.

User note (may be empty): $ARGUMENTS

1. Follow the **Before acting** and **Branches** sections of the `git-ops` agent, and also check `git remote -v` and `git ls-remote --heads origin`.
2. Commit pending changes that belong to this work as its **Commits** section defines.
3. Read the full branch diff against the target that its **Pull requests** section picks. A target in the user note overrides it.
4. Run the project's test, lint, and typecheck commands if they are cheap and obvious (from package.json scripts, composer.json, Makefile, or AGENTS.md). Report exactly what passed or failed.
5. Push with `git push -u origin <branch>`, never force.
6. Run `gh pr create` with the title and body exactly as the **Pull requests** section defines them. If an open PR already exists for the branch, update it with `gh pr edit` instead.

Return the PR URL, the commits included, and the validation results.
