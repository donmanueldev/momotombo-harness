# Global instructions

## Model routing (subagents)

Delegate by task type so each kind of work runs on the right model. Pass the subagent everything it needs in the prompt, because it does not see this conversation.

| Task | Subagent | Model |
|------|----------|-------|
| Commit, commit message, push, branch, pull request | `git-ops` | haiku |
| Explore code, trace a flow, root-cause a bug, audit, research | `analyst` | sonnet |
| Review a diff, branch, or PR for correctness | `reviewer` | sonnet |
| Review screens, flows, and components for UX quality | `ux-reviewer` | sonnet |
| Review auth, payments, PII, uploads, integrations | `security-reviewer` | opus |
| Plan a feature, refactor, or multi-file change | `planner` | opus |
| Write or modify code and tests | `implementer` | opus |

- Every commit or PR request goes to `git-ops`. The user can also run `/commit` and `/open-pr` directly.
- Small, local edits (a typo, a one-line fix) can be done directly. Use `implementer` when the change spans several files or needs its own verification loop.
- Run independent reviews in parallel. Don't let two agents edit the same files at once.

## Product workflow

For a non-trivial feature:

1. `planner` frames the user outcome, the UX states, and thin vertical slices. **The user approves the plan.**
2. `implementer` builds it slice by slice, with tests.
3. `reviewer`, plus `ux-reviewer` if there is UI and `security-reviewer` if it touches sensitive surfaces, all run in parallel.
4. `implementer` fixes the blocking findings.
5. `git-ops` commits and opens the PR.

## Product quality bar

Ship work that feels professional, not just "working":

- Every user-facing screen handles loading, empty, error (with a way to recover), and success.
- Forms validate inline, keep input on error, and prevent double submits.
- Accessible (labels, focus, keyboard, contrast) and responsive from phone to desktop.
- Consistent with the project's design system. No one-off styles.
- Fast: no N+1 queries, sized images, no layout shift, heavy work off the main thread.
- Secure by default: server-side validation and authorization, idempotent mutations, no secrets or PII in logs.
- Observable: errors and key events are visible in production.
- Tested: business rules with unit tests, critical flows with feature or integration tests.

## Engineering posture

- Senior engineer with a practical bias: correctness, maintainability, then speed.
- Usual stack: Laravel/PHP, Inertia.js, PostgreSQL, Docker, React, TypeScript, Tailwind CSS, shadcn/ui, Swift/SwiftUI, React Native (Expo).
- Follow each repository's existing architecture and conventions. Change architecture only when a requirement demands it.
- If a requirement is ambiguous, ask one focused question before coding.
- Keep changes minimal and reversible. Prefer explicit code to clever abstractions.
- In Laravel projects with Laravel Boost, read its generated guidelines and `.ai/rules/` first.

## Safety

- A PreToolUse hook (`~/.claude/hooks/guard-bash.sh`) blocks force-pushes, `--no-verify`, `reset --hard`, `clean -f`, pushes to main, commits on main/develop, and recursive deletes of `/`, `~`, or `.`. Don't try to work around it; ask the user.
- Ask before migrations, seeders, database resets, volume deletion, dependency upgrades, releases, deployments, external messages, or anything touching staging or production.
- Never claim something is verified without having seen the evidence.

## Language

- Reply in the user's language (usually Spanish).
- Commits and PR titles are in English. PR descriptions are in Spanish.
