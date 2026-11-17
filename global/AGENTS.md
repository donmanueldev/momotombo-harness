# Momotombo harness: global instructions

These apply to every project, in Claude Code (`~/.claude/CLAUDE.md`) and Codex (`~/.codex/AGENTS.md`). A project's own `AGENTS.md` and `docs/` refine them and win on conflicts.

## Language

Talk to the user in their language (usually Spanish). Everything versioned is in **English** (code, specs, plans, `docs/`, `AGENTS.md`, commits, PR titles), except PR descriptions, which are in Spanish.

## Start of every task

1. Read the project's `AGENTS.md`, and only the `docs/` files its map lists for the task.
2. With `openspec/`, run `openspec list --specs` and read the specs the task touches.
3. If a requirement is ambiguous, ask one focused question before writing code.

## Workflow

| Task size | How |
|-----------|-----|
| Typo, one-line fix, config tweak | Do it directly, then verify. |
| Small change inside one existing component or module, with no new contract, backend, data, or auth | Settle it in chat (or `grill-me`), then `implementer` with a test, the reviewers, and `/commit`. No OpenSpec, and no spec-from-code for it. |
| Bug fix | Reproduce it, find the root cause (`analyst`, with `diagnosing-bugs` if installed), fix it with a regression test (`implementer`). |
| Feature or behavior change | Spec-driven with OpenSpec (below). |

Before the first edit, check the branch. On `main`, `master`, or `develop`, or on a branch with unrelated commits, create a `feature/*` or `bugfix/*` branch from the base first; ask if unsure.

**Spec-driven flow** (Claude: `/openspec-…` · Codex: `$openspec-…`):

1. `openspec-explore`: think it through with the user; no code yet.
2. `openspec-propose <change>`: lean specs plus a phased `plan.md`. **Stop until the user approves the specs and contracts.**
3. `openspec-apply-change`: implement **only the first phase with unchecked tasks**, verify, check off its tasks, then **stop without committing**. The user reviews and commits (`/commit` in Claude, `git-ops` in Codex); the next phase waits for the next apply.
4. `openspec-archive-change`: once every phase is merged, fold the specs into `openspec/specs/`.

OpenSpec is the only planning flow: no other spec, ticket, or implement-plan flow (`to-spec`, `to-tickets`, `implement-spec`) next to it. When installed, these skills plug into its steps (skip them silently otherwise): `grill-me` before `explore` when the idea is fuzzy (`grill-with-docs` when the repo has domain docs); in `propose`, `domain-modeling` for new domain concepts and `codebase-design` for module boundaries; `tdd` for business rules in `apply`; `improve-codebase-architecture` for cleanup outside a change.

Before changing behavior that has no spec, run `codely-openspec-spec-create-from-code <capability>`. When the user corrects a convention, offer `codely-doc-create` so it becomes a `docs/` page, without Codely's mascot sign-off or course links. `harness init` installs both `codely-*` skills per project; if they're missing, suggest it instead of improvising them.

## Subagents

Delegate by task type so each kind of work runs on the right model. A subagent doesn't see the conversation: put everything it needs in the prompt. **Codex doesn't pick these agents automatically: spawn them by name.**

| Task | Agent | Claude | Codex |
|------|-------|--------|-------|
| Commit, branch, push, pull request | `git-ops` | haiku | gpt-6-luna, low |
| Explore code, trace a flow, root-cause, research | `analyst` | sonnet | session model, medium |
| Review a diff or PR for correctness | `reviewer` | sonnet | session model, medium |
| Review UI: states, accessibility, responsive, copy | `ux-reviewer` | sonnet | session model, medium |
| Review auth, payments, PII, uploads, integrations | `security-reviewer` | opus | session model, high |
| Plan work outside OpenSpec | `planner` | opus | session model, high |
| Write or modify code and tests | `implementer` | opus | session model, high |

- Every commit or PR goes through `git-ops` (or `/commit` and `/open-pr` in Claude).
- Run independent reviews in parallel; two agents never edit the same files at once.
- After a phase: `reviewer`, plus `ux-reviewer` if UI changed and `security-reviewer` if a sensitive surface changed. If review follow-ups add markup or logic beyond copy and tests, re-run that reviewer on the new delta.

## Stack

Momotombo products use **Laravel + Inertia.js + React + TypeScript + Tailwind CSS + shadcn/ui + PostgreSQL**, and **Expo / React Native** for mobile, run locally with Docker. Follow each repo's existing architecture; change it only when a requirement demands it.

- Laravel with Boost: read Boost's guidelines first (it renders `.ai/guidelines/*.md`, including the docs map, into `AGENTS.md`) and use its skills.
- Expo: use the `expo-*` skills, and `react-native-best-practices` for performance and libraries. Check native config (`app.json`, config plugins, permissions, locales) against the Expo docs before planning or writing it.
- Vue, Next.js, or NativePHP repos: same rules, with that stack's installed skill instead of improvised conventions.
- Laravel stays the product core. Python is only for AI agents and the services that expose them, when PHP can't meet a concrete need; don't port what works. Use 3.12+, `pyproject.toml`, `uv`, `ruff`, `pytest`, `pyright`.
- AI agents: run the evals before and after changing a prompt, tool, or model (propose a small set if there are none); use structured output with a schema instead of parsing free text; give every tool a clear contract and validate and authorize its input on the server as if it came from a user; keep secrets and PII out of prompts, traces, and logs; keep a cost and latency budget per conversation and know which model call dominates it; default to the latest Claude models.

## Writing code

Priorities: correct and secure, then simple and readable, then fast. Clear beats clever.

- Read the surrounding code and match its patterns, names, and structure. Reuse an existing helper, hook, component, or schema before writing one.
- Use libraries as their current docs say (installed skill, official docs, then the web). Don't invent patterns from memory, and don't add a dependency for what the framework or an existing package already does.
- Fix the root cause, with the smallest change. No workarounds or special cases around a problem you haven't understood, and no layers "for later".
- Name things after the domain, not `data`, `handle`, `Manager`, or `Helper`.
- Validate at the boundaries (user input, network, storage, external APIs) and trust the types inside: no defensive re-checks or normalizing, and no regex where a string method does the job.
- Comment only a why the code can't show. No restating comments, banners, obvious JSDoc, or commented-out code.
- Delete what your change made dead. Handle errors where they can be handled; don't wrap everything in try/catch.
- Tests check behaviour through the public interface and fail without the change. No tests that only assert mocks or repeat the implementation, no comments inside tests, no tests of trivial code.
- User-facing work meets the product bar: loading, empty, error with recovery, and success states; accessible and responsive; design-system components; server-side validation and authorization; idempotent mutations; no secrets or PII in logs. `implementer` and the reviewers carry the full checklist.

## Verification

Never claim something works without evidence. Before reporting work as done, run the check command named in the project's `AGENTS.md` (such as `composer check`); without one, run tests, typecheck, lint, and format, and suggest adding it. Report exactly what ran and passed, and mark anything not run as not verified.

## Long-running commands

Builds, installs, test and eval suites, migrations, Docker, deploys: anything that can take minutes, in every stack (subagents too).

- Claude: run it with `run_in_background`, never in the foreground. The stall watcher wakes you once if it writes nothing for 10 minutes at near-idle CPU; set your own check for any waiting after that, and know it misses a hang hidden by a busy sibling process. Codex has no watcher: check the log's age and the exit yourself.
- Don't wait with an `until … sleep` loop: it writes nothing, so the watcher reports it. The process you wait on is already watched; otherwise use Monitor on its log.
- Never estimate a finish time from the last log line. Walk the process tree (`pgrep -P <pid>`, `ps -o pid,etime,%cpu,command -p <pid>`) down to the leaf.
- When it's stuck, name the step and process, find out why, and tell the user before stopping anything; don't wait it out or retry blindly. Turn off steps that don't belong in the run (such as a symbol upload in a dev build) through the tool's documented config, or agree on the fix with the user when there is none.

## Safety

- The guard hook (`guard-bash.sh`, Claude and Codex, backed by Codex `.rules`) blocks force-pushes, `--no-verify`, `reset --hard`, `clean -f`, pushes to main, commits on main/master/develop, recursive deletes of `/`, `~`, or `.`, and printing `.env` files. Don't work around it; ask the user.
- Ask before migrations on shared data, seeders, database resets, volume deletion, dependency upgrades, releases, deployments, external messages, or anything touching staging or production.
- Never read or print `.env` files, keys, or tokens.
