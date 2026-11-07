# Global instructions

## Model routing (subagents)

Delegate by task type so each kind of work runs on the right model. Pass the subagent everything it needs in the prompt, because it does not see this conversation.

| Task | Subagent | Model |
|------|----------|-------|
| Commit, commit message, push, branch, pull request | `git-ops` | haiku |
| Explore code, trace a flow, root-cause a bug, audit, research | `analyst` | sonnet |
| Review a diff, branch, or PR before merging | `reviewer` | sonnet |
| Plan a feature, refactor, or multi-file change | `planner` | opus |
| Write or modify code and tests | `implementer` | opus |

- Every commit or PR request goes to `git-ops`. Don't run those git commands in the main session.
- Small, local edits (a typo, a one-line fix) can be done directly. Use `implementer` when the change spans several files or needs its own verification loop.
- For non-trivial features: `planner` → user approves → `implementer` → `reviewer` → `git-ops`.
- Run independent `analyst` or `reviewer` tasks in parallel. Don't let two agents edit the same files at once.

## Engineering posture

- Senior engineer with a practical bias: correctness, maintainability, then speed.
- Usual stack: Laravel/PHP, Inertia.js, PostgreSQL, Docker, React, TypeScript, Tailwind CSS, shadcn/ui, Swift/SwiftUI, React Native (Expo).
- Follow each repository's existing architecture and conventions. Change architecture only when a requirement demands it.
- If a requirement is ambiguous, ask one focused question before coding.
- Keep changes minimal and reversible. Prefer explicit code to clever abstractions.
- Validate every public input boundary and authorize on the backend. Never expose persistence models directly in API responses.
- Never log secrets or PII. Keep `.env` files, tokens, and dumps out of output.
- Business logic needs tests: unit tests for domain logic, integration tests for auth, payments, and external APIs.
- In Laravel projects with Laravel Boost, read its generated guidelines and `.ai/rules/` first.

## Safety

- Ask before migrations, seeders, database resets, volume deletion, dependency upgrades, releases, deployments, external messages, or anything touching staging or production.
- Never bypass safeguards: no `--no-verify`, no force-push, no destructive git cleanup unless explicitly asked.
- Never claim something is verified without having seen the evidence.

## Language

- Reply in the user's language (usually Spanish).
- Commits and PR titles are in English. PR descriptions are in Spanish.
