# Momotombo harness: global instructions

These instructions apply to every project, in Claude Code (`~/.claude/CLAUDE.md`) and Codex (`~/.codex/AGENTS.md`). A project's own `AGENTS.md` and `docs/` refine them and win on conflicts.

## Language

- Talk to the user in their language (usually Spanish).
- Write everything that is versioned in **English**: code, specs, plans, `docs/`, `AGENTS.md`, commit messages, and PR titles.
- PR descriptions are in Spanish.

## Start of every task

1. Read the project's `AGENTS.md`. Open only the `docs/` files its map lists as relevant to the task.
2. If the project has `openspec/`, run `openspec list --specs` to learn which capabilities already have specs, and read the specs the task touches.
3. If a requirement is ambiguous, ask one focused question before writing code.

## Workflow

| Task size | How |
|-----------|-----|
| Typo, one-line fix, config tweak | Do it directly, then verify. |
| Bug fix | Reproduce it, find the root cause (`analyst`), fix it with a regression test (`implementer`). |
| Feature or behavior change | Spec-driven with OpenSpec (below). |

**Spec-driven flow** (Claude: `/openspec-…` · Codex: `$openspec-…`):

1. `openspec-explore`: think it through with the user; no code yet.
2. `openspec-propose <change>`: write lean specs plus a phased `plan.md`. **Stop and let the user approve the specs and the contracts** before implementing.
3. `openspec-apply-change`: implement **only the first phase with unchecked tasks**, run its verification, check off its tasks, then **stop**. Don't commit. Tell the user to review, then commit with `/commit` (Claude) or the `git-ops` agent (Codex). The next phase starts only when the user runs apply again.
4. `openspec-archive-change`: once every phase is merged, fold the specs into `openspec/specs/`.

For a project that already has behavior without specs, run `codely-openspec-spec-create-from-code <capability>` before changing it.

**When the user corrects you** on a convention ("we don't do it that way"), offer to run `codely-doc-create` so the correction becomes a `docs/` page and doesn't happen again. Leave out Codely's mascot sign-off and course links from docs and replies.

## Subagents

Delegate by task type so each kind of work runs on the right model. A subagent doesn't see the conversation, so give it everything it needs in the prompt. **Codex: spawn these agents by name when the table says so. Codex doesn't pick them automatically.**

| Task | Agent | Claude | Codex |
|------|-------|--------|-------|
| Commit, branch, push, pull request | `git-ops` | haiku | gpt-6-luna, low |
| Explore code, trace a flow, root-cause, research | `analyst` | sonnet | session model, medium |
| Review a diff or PR for correctness | `reviewer` | sonnet | session model, medium |
| Review UI: states, accessibility, responsive, copy | `ux-reviewer` | sonnet | session model, medium |
| Review auth, payments, PII, uploads, integrations | `security-reviewer` | opus | session model, high |
| Plan work outside OpenSpec | `planner` | opus | session model, high |
| Write or modify code and tests | `implementer` | opus | session model, high |

- Every commit or PR goes through `git-ops`. In Claude the user can also run `/commit` and `/open-pr`.
- Run independent reviews in parallel. Two agents never edit the same files at once.
- After implementing a phase: `reviewer`, plus `ux-reviewer` if UI changed, plus `security-reviewer` if a sensitive surface changed.

## Stack

Momotombo products use **Laravel + Inertia.js + React + TypeScript + Tailwind CSS + shadcn/ui + PostgreSQL**, run locally with Docker, and **Expo / React Native** for mobile.

- Laravel projects with Laravel Boost: read Boost's guidelines and `.ai/rules/` first, and use its skills (Inertia, Pest, and others).
- Expo projects: use the Expo plugin skills (`expo-*`) for routing, native UI, animation, builds, and updates.
- Follow each repository's existing architecture and conventions. Change architecture only when a requirement demands it.

## Product quality bar

Ship work that feels professional, not just "working":

- Every user-facing screen handles loading, empty, error (with a way to recover), and success.
- Forms validate inline, keep input on error, and prevent double submits.
- Accessible (labels, focus, keyboard, contrast) and responsive from phone to desktop.
- Consistent with the project's design system. No one-off styles.
- Fast: no N+1 queries, sized images, no layout shift, heavy work off the main thread.
- Secure by default: server-side validation and authorization (Form Requests, Policies), idempotent mutations, no secrets or PII in logs.
- Observable: errors and key events are visible in production.
- Tested: business rules with unit tests, critical flows with feature or integration tests. A test must fail without the change.

## Verification

Never say something works without evidence. Before reporting a task or phase as done, run the project's checks (tests, typecheck, lint, format) and report exactly what ran and what passed. Mark anything you couldn't run as not verified.

## Safety

- A guard hook (`guard-bash.sh`, in both Claude and Codex) blocks force-pushes, `--no-verify`, `reset --hard`, `clean -f`, pushes to main, commits on main/master/develop, and recursive deletes of `/`, `~`, or `.`. Codex `.rules` back it up. Don't work around them; ask the user.
- Ask before migrations on shared data, seeders, database resets, volume deletion, dependency upgrades, releases, deployments, external messages, or anything touching staging or production.
- Never read or print `.env` files, keys, or tokens.
