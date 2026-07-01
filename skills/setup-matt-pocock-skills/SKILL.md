---
name: setup-matt-pocock-skills
description: Configure a repo for Matt-style engineering skills by setting the issue tracker, triage label vocabulary, and domain-doc layout. Use when bootstrapping a new repo for to-prd, to-issues, triage, grill-with-docs, improve-codebase-architecture, or domain-modeling.
---

# Setup Matt Pocock Skills

Use this when you are bootstrapping a repo for the engineering flow or when its tracker, labels, or domain docs need to be aligned.

## Process

1. Explore the repo state first.
2. Present findings and ask one decision at a time.
3. Confirm the answers before writing anything.
4. Write the repo-level docs and the agent-skill block.
5. Finish with a short summary of what now works.

## Explore

Check, at minimum:
- `git remote -v` and `.git/config`
- `AGENTS.md` or `CLAUDE.md` at the repo root
- `CONTEXT.md` and `CONTEXT-MAP.md`
- `docs/adr/` and any nested ADR folders
- `docs/agents/`
- `.scratch/`

## Ask in order

### 1. Issue tracker

Decide where issues live for this repo.

Default options:
- GitHub issues
- GitLab issues
- Local markdown under `.scratch/`
- Other tracker, if the repo uses one already

If the tracker is GitHub or GitLab, also ask whether external PRs should be treated as request surfaces for triage.

### 2. Triage labels

Map the five canonical roles to the repo's real labels:
- `needs-triage`
- `needs-info`
- `ready-for-agent`
- `ready-for-human`
- `wontfix`

Use the repo's existing label names if they differ.

### 3. Domain docs

Confirm whether the repo is:
- single-context: one `CONTEXT.md` plus `docs/adr/` at the root
- multi-context: `CONTEXT-MAP.md` at the root pointing to per-area context files

## Write

Update the chosen root file:
- Prefer `CLAUDE.md` if it exists
- Otherwise use `AGENTS.md`
- If neither exists, ask which one to create

Add or replace an `## Agent skills` block with concise summaries for:
- issue tracker
- triage labels
- domain docs

Write the repo docs under `docs/agents/`:
- `issue-tracker.md`
- `triage-labels.md`
- `domain.md`

## Done

Report what was discovered, what was written, and which engineering skills will now read from the new docs.
