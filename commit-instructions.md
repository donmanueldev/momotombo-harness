# Commit Instructions (agent-config)

## Scope and intent

- Keep commit history intentional: one logical change per commit.
- Use conventional commits with explicit scope: `type(scope): description`.
- Scope examples for this repo:
  - `skills(...)` for skill changes
  - `agents(...)` for agent/agent-doc updates
  - `chore(...)` for workflow/sync/tooling documentation
  - `docs(...)` for markdown/docs-only updates

## Before commit

- Confirm target files and avoid unrelated edits.
- `git status --short` to verify only expected files changed.
- If behavior changes are added:
  - update only the minimum files required
  - keep naming and conventions consistent with surrounding files
- If tests or scripts changed:
  - run `bats tests/`.
- Run `git diff --check` to catch whitespace/formatting issues.

- If `AGENTS.md`, `SKILL.md`, or automation files are changed, include rationale in the commit body.

## Commit message format

Use:

```
type(scope): imperative summary

- Why: why this change
- What: what changed
- Validation: commands executed
- Risk: level + mitigation
```

Examples:
- `feat(skills): add workflow orchestration guardrails`
- `fix(skills): remove stale routing in workflow skill`
- `chore(agents): sync codex doc references`

## Validation checklist

- Keep at least one test/validation command run when touched logic can affect behavior.
- For command-policy or instruction changes, validate discoverability manually by running the related flow once (e.g., sync/plan).
- Prefer small, reversible commits over broad monolith commits.

## Approval policy

- Ask before committing changes that touch destructive command execution paths or security-sensitive automation.
- Proceed with user confirmation for potentially wide-ranging instruction changes.
