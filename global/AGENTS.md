# Momotombo harness: global instructions

These apply to every project, in Claude Code (`~/.claude/CLAUDE.md`) and Codex (`~/.codex/AGENTS.md`). A project's own `AGENTS.md` and `docs/` refine them and win on conflicts.

## Language

Talk to the user in their language (usually Spanish). Everything versioned is in **English** (code, docs, `AGENTS.md`, commits, PR titles), except PR descriptions, which are in Spanish.

## How to work

For every task that investigates, changes, fixes, refactors, speeds up, or integrates code, follow the **`momotombo-mode`** skill: it picks the playbook and holds the code, test, architecture, and security principles. Do the work in the main session.

Before the first edit, check the branch. On `main`, `master`, or `develop`, or on a branch with unrelated commits, create a `feature/*` or `bugfix/*` branch first.

## Stack

Momotombo products use **Laravel + Inertia.js + React + TypeScript + Tailwind CSS + shadcn/ui + PostgreSQL**, run locally with Docker, and **Expo / React Native** for mobile. Official skills own each stack's how-to; use them before improvising:

- **Laravel**: Laravel Boost. Read its guidelines (rendered into `AGENTS.md`), use its skills (`laravel-best-practices`, `testing-best-practices`, `inertia-react-development`, `pest-testing`, `tailwindcss-development`, `wayfinder-development`, and `filament-development` when Filament is installed), and its MCP tools: `search-docs` for version-matched docs, the database schema, routes, and logs. Record a project convention with Boost's `record-rule` tool (`.ai/rules/`).
- **Expo**: the `expo-*` skills, and Software Mansion's `react-native-best-practices` for performance and its libraries.
- **AI agents**: Laravel stays the core. Build agents with the Laravel AI SDK (`laravel/ai`), stream them to React with `@laravel/stream-react`, and expose app tools with Laravel MCP (`laravel/mcp`). Add a Python service (3.12+, `uv`, `ruff`, `pytest`, `pyright`) only for a concrete need PHP can't meet. Run evals before and after changing a prompt, tool, or model; use structured output; validate and authorize tool input on the server; keep secrets and PII out of prompts and traces; keep a cost and latency budget; default to the latest Claude models.
- **Next.js, Vue, NativePHP**: same rules, with that stack's installed skill (the Vercel plugin for Next.js and the AI SDK).

## Approval boundary

Proceed without asking on anything local and reversible. Ask before anything that leaves the machine or is hard to undo: push, merge, deploy, external messages, production or shared data, migrations on shared data, seeders, database resets, new or upgraded dependencies, releases.

## Subagents

Use a subagent only to keep bulk reading out of the main context, to run independent work in parallel, or to review when the user integrates. It doesn't see the conversation: give it the goal and everything it needs. **Codex doesn't pick these automatically: spawn them by name.**

| Task | Agent | Claude | Codex |
|------|-------|--------|-------|
| Commit, branch, push, pull request | `git-ops` | haiku | gpt-6-luna, low |
| Explore code, trace a flow, root-cause, research | `analyst` | sonnet | session model, medium |
| Review a diff when integrating | `reviewer` | sonnet | session model, medium |
| Review auth, payments, PII, uploads, integrations | `security-reviewer` | opus | session model, high |
| Find comments that don't earn their place | `comment-sicko` | sonnet | session model, medium |
| Independent code task run in parallel | `worker` | opus | session model, high |

Commits and PRs go through `git-ops` (or `/commit` and `/open-pr` in Claude).

## Verification

Never claim something works without evidence. Report exactly what ran and passed, and mark anything not run as not verified.

## Long-running commands

Builds, installs, test suites, migrations, Docker, deploys (subagents too):

- Claude: run them with `run_in_background`. The stall watcher wakes you once after 10 silent minutes at near-idle CPU; set your own check after that. Codex has no watcher: check the log's age and the exit yourself. Don't wait with `until … sleep` loops; use Monitor on the log.
- Judge progress from the process tree (`pgrep -P <pid>`, `ps -o pid,etime,%cpu,command -p <pid>`), never from the last log line. When it's stuck, find out why and tell the user before stopping anything.

## Safety

- The comment guard reports comments an edit adds: remove them, or keep one that fits an exception and say which.
- The guard hook (`guard-bash.sh`, Claude and Codex, backed by Codex `.rules`) blocks force-pushes, skipping hooks, `reset --hard`, `clean -f`, discarding all changes, pushes to main, commits and merges on main/master/develop, recursive deletes of `/`, `~`, or `.`, and printing `.env` files. Don't work around it; ask the user.
- Never read or print `.env` files, keys, or tokens.
