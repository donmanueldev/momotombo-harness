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
| Small change inside one existing component or module, with no new contract, backend, data, or auth | Settle it in chat (or `grill-me`), then `implementer` with a test, the reviewers, and `/commit`. No OpenSpec, and no spec-from-code for it. |
| Bug fix | Reproduce it, find the root cause (`analyst`, with `diagnosing-bugs` if installed), fix it with a regression test (`implementer`). |
| Feature or behavior change | Spec-driven with OpenSpec (below). |

Before the first edit of any change, check the branch. If it is `main`, `master`, or `develop`, or it holds commits unrelated to the task, create a `feature/*` or `bugfix/*` branch from the base branch first, and ask if unsure.

**Spec-driven flow** (Claude: `/openspec-…` · Codex: `$openspec-…`):

1. `openspec-explore`: think it through with the user; no code yet.
2. `openspec-propose <change>`: write lean specs plus a phased `plan.md`. **Stop and let the user approve the specs and the contracts** before implementing.
3. `openspec-apply-change`: implement **only the first phase with unchecked tasks**, run its verification, check off its tasks, then **stop**. Don't commit. Tell the user to review, then commit with `/commit` (Claude) or the `git-ops` agent (Codex). The next phase starts only when the user runs apply again.
4. `openspec-archive-change`: once every phase is merged, fold the specs into `openspec/specs/`.

OpenSpec is the only planning flow. Don't run another spec, ticket, or implement-plan flow (such as `to-spec`, `to-tickets`, `implement-spec`) next to it. These skills plug into its steps when installed; skip them silently when not:

- Before `explore`, if the idea is still fuzzy: `grill-me` (`grill-with-docs` when the repo has domain docs).
- In `propose`: `domain-modeling` for new domain concepts, `codebase-design` for module boundaries.
- In `apply`: `tdd` for business rules.
- Outside a change, for cleanup: `improve-codebase-architecture`.

For a project that already has behavior without specs, run `codely-openspec-spec-create-from-code <capability>` before changing it.

The two `codely-*` skills are installed per project by `harness init`. If they're missing in a repo, say so and suggest `harness init` instead of improvising them.

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
- After implementing a phase: `reviewer`, plus `ux-reviewer` if UI changed, plus `security-reviewer` if a sensitive surface changed. If the follow-ups to a review add markup or logic beyond copy and tests, run that reviewer again on the new delta.

## Stack

Momotombo products use **Laravel + Inertia.js + React + TypeScript + Tailwind CSS + shadcn/ui + PostgreSQL**, run locally with Docker, and **Expo / React Native** for mobile.

- Laravel projects with Laravel Boost: read Boost's guidelines first (Boost renders `.ai/guidelines/*.md`, including the docs map, into `AGENTS.md`), and use its skills (Inertia, Pest, and others).
- Expo projects: use the Expo plugin skills (`expo-*`) for routing, native UI, animation, builds, and updates. For React Native performance and libraries, use `react-native-best-practices`. Check native config (`app.json`, config plugins, permissions, locales) against the Expo docs before writing it into a plan or code.
- Some repos use Vue instead of React, Next.js, or NativePHP. The same rules apply; use the installed skill for that stack (for example the Vercel plugin for Next.js) instead of improvising conventions.
- Python is for AI agents and the services that expose them. Laravel stays the product core; add a Python service only for a concrete need PHP can't meet, and don't port what works. Python projects use 3.12+, `pyproject.toml`, `uv`, `ruff`, `pytest`, and `pyright`.
- Follow each repository's existing architecture and conventions. Change architecture only when a requirement demands it.

## Writing code

Priorities, in order: correct and secure, then simple and readable, then fast. Clear beats clever.

- Read the surrounding code first and match its patterns, names, and structure.
- Search for an existing helper, hook, component, or schema before writing one. Don't duplicate logic.
- Use a library the way its current docs say (Zod, Zustand, TanStack Query, Expo, Laravel, and so on). Check the installed skill or the official docs, and search the web when neither covers it. Don't invent a pattern from memory.
- Fix the root cause. No patches, workarounds, or special cases around a problem you haven't understood.
- Make the smallest change that solves the task. No abstractions, options, or layers "for later".
- Don't add a dependency for what the framework or an existing package already does.
- Name things after the domain, not `data`, `handle`, `Manager`, or `Helper`.
- Validate at the boundaries (user input, network, storage, external APIs) and trust the types inside. No defensive re-checks, normalizing, or sanitizing of data that is already valid, and no regex where a string method or the library already does the job.
- Most code needs no comments. Comment only a why the code can't show (a platform bug, a business rule, a deliberate trade-off). No comments that restate the code, section banners, JSDoc on obvious functions, or commented-out code.
- Delete what your change made dead: code, imports, files, flags.
- Handle the errors that can happen, where they can be handled. Don't wrap everything in try/catch.
- Tests check behaviour through the public interface. No tests that can't fail, that only assert a mock was called, or that repeat the implementation, and no comments inside tests: the test name says what it checks. Don't test trivial code.

## Building AI agents

- Run the project's evals before and after changing a prompt, a tool, or a model. If there are none, propose a small eval set before making the change.
- Use structured output with a schema instead of parsing free text.
- Give every tool a clear contract, and validate and authorize its input on the server as if it came from a user.
- Keep secrets and personal data out of prompts, traces, and logs.
- Keep a cost and latency budget per conversation, and know which model call dominates it.
- Default to the latest Claude models.

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

- A guard hook (`guard-bash.sh`, in both Claude and Codex) blocks force-pushes, `--no-verify`, `reset --hard`, `clean -f`, pushes to main, commits on main/master/develop, recursive deletes of `/`, `~`, or `.`, and printing `.env` files. Codex `.rules` back it up. Don't work around them; ask the user.
- Ask before migrations on shared data, seeders, database resets, volume deletion, dependency upgrades, releases, deployments, external messages, or anything touching staging or production.
- Never read or print `.env` files, keys, or tokens.
