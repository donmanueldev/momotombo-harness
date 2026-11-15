---
name: open-pr
description: Commit any pending changes, push the branch, and open a GitHub pull request (English title, Spanish body), run by the git-ops subagent on haiku.
context: fork
agent: git-ops
model: haiku
disable-model-invocation: true
argument-hint: "[optional: target branch, ticket, or context for the description]"
---

> Codex: run these steps through the `git-ops` custom agent (spawn it by name), or follow them directly.

Open a pull request for the work in the git repository at the current working directory.

User note (may be empty): $ARGUMENTS

1. Inspect `git status --short`, the current branch, `git remote -v`, and `git ls-remote --heads origin`.
2. If you are on `main`, `master`, or `develop`, create a correctly prefixed Git Flow branch first.
3. If there are uncommitted changes that belong to this work, commit them as English Conventional Commits, staging explicit paths and never secrets or `.env*`.
4. Read the full branch diff against the target. The target is `develop` if it exists on origin, otherwise the default branch; `release/*` and `hotfix/*` target `main`. A target in the user note overrides this.
5. Run the project's test, lint, and typecheck commands if they are cheap and obvious (from package.json scripts, composer.json, Makefile, or CLAUDE.md). Report exactly what passed or failed.
6. Push with `git push -u origin <branch>`, never force.
7. Run `gh pr create` with:
   - The title (English) and body (Spanish template) exactly as the **Pull requests** section of the `git-ops` agent defines them. That agent is the single source for the format. Add `## Migración` or `## Capturas` only when they apply.
   - If an open PR already exists for the branch, update it with `gh pr edit` instead of creating another.

Return the PR URL, the commits included, and the validation results.
