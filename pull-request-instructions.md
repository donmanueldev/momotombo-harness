# Pull Request Instructions

Use this file when generating a pull request title or description.

## Evidence first

Inspect the branch diff, target branch, changed contracts, and executed validation before drafting. The PR must distinguish verified behavior from planned or unverified work.

## Title

Write an English Conventional Commit-style title:

```
type(scope): imperative summary
```

- Keep it specific, imperative, and 72 characters or fewer.
- Use the same change type and narrow scope as the primary commit.
- State the user or system outcome, not an implementation detail.
- Omit ticket prefixes unless the repository or user explicitly requires one.

## Description

Use this structure and remove sections that have no meaningful content:

```md
## Problem
<What was wrong, missing, or being improved.>

## Changes
- <Behavior or contract change>
- <Supporting implementation change>

## Validation
- [x] `<command actually run>` — <result>
- [ ] `<command not run>` — <reason>

## Risk and rollback
- Risk: <low|medium|high and blast radius>
- Rollback: <revert path, migration consideration, or why no rollback is needed>
```

Add these sections only when applicable:

- `## Migration` for schema, configuration, or compatibility changes.
- `## Screenshots` for visible UI changes.
- `## Follow-ups` for intentionally deferred work.
- `## Related work` for issues, ADRs, or dependent PRs.

## Quality bar

- Explain outcomes and contracts; group file-level detail only when it helps review.
- State validation exactly as observed. Do not mark a check complete unless it ran successfully.
- Name behavior outside the repository that is affected, including APIs, configuration, or deployment steps.
- Keep the description reviewable: no unrelated history, raw logs, secrets, or speculative claims.
- Call out risk and rollback whenever a change can affect users, data, automation, or compatibility.

Do not create, update, merge, or comment on a PR while drafting its title or description unless the user explicitly requests that action.
