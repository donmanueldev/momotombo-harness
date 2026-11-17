# momotombo-harness

The agent harness Momotombo uses to build products with **Claude Code** and **Codex**:

- The agent does the work in your session and proves it works before saying so.
- It asks only before what leaves your machine or is hard to undo.
- Every correction you make becomes a rule, a check, or a doc the agents follow next time.

Its structure is inspired by [Codely's agent-harness](https://github.com/CodelyTV/agent-harness). See [Credits](#credits).

## Why

Most AI slop has five causes. The harness answers each one:

| Cause | Answer |
|-------|--------|
| The agent guesses your conventions | `docs/` per project with good and bad examples. `AGENTS.md` maps them so agents read only what the task needs |
| Big changes nobody can review | For a large feature, the data shape, contracts, and phases **you approve once** before it is built |
| The same mistake comes back | A correction becomes a lint rule, test, or script; otherwise a versioned rule (Laravel Boost `record-rule`, or `codely-doc-create` outside Laravel) |
| "Done" without proof | Agents verify what they changed and report exactly what ran |
| Generic, unsafe code | Principles for code, tests, architecture, and security, stack conventions, and reviews when you integrate |

## Two layers

```
GLOBAL (your machine)                      PROJECT (each product repo)
harness install                            harness init
───────────────────────────────            ─────────────────────────────────────
~/.claude + ~/.codex + ~/.agents           AGENTS.md (= CLAUDE.md): commands + docs map
  · global instructions (one file)         docs/: conventions that grow with every correction
  · momotombo-mode: playbooks + principles codely-doc-create, pinned in skills-lock.json
  · 6 subagents, routed by model
  · /commit, /open-pr, no-comments
  · guard hook, comment guard, loop hook, stall watcher, Codex rules
  · status line, settings
```

## Setup

Requirements: macOS, `git`, `jq`, Node 22+ (for `npx`), and Claude Code and/or Codex. Without Codex, `install` and `doctor` skip its links and hook.

```bash
git clone git@github.com:donmanueldev/momotombo-harness.git ~/momotombo-harness
~/momotombo-harness/bin/harness install --dry-run   # preview
~/momotombo-harness/bin/harness install             # apply; then `harness` is on your PATH
harness doctor
```

`install` never deletes your data:
- A file that differs from the harness is kept, unless you pass `--force`. With `--force` it is backed up first.
- Settings are **merged**: other tools' hooks and your keys stay, permissions are combined, and the keys the harness sets (`model`, `statusLine`, `attribution`, `enabledPlugins`, its own hooks) take the harness's value. The previous file is backed up first.
- `harness undo` restores the last backup. Backups live in `~/.local/state/momotombo-harness/backups` (override with `HARNESS_BACKUP_DIR`), outside the checkout, because they hold copies of your own config.

In Codex, approve the guard hook once with `/hooks`.

## Start a product

```bash
cd ~/momotombo/my-product
harness init                     # detects Laravel, React, Expo, Python, or pass --stack laravel,react
```

Then fill in the Architecture section of `AGENTS.md`, trim the seeded `docs/`, ask the agent to set up the kitchen (lint, static analysis, and the check commands for your stack, from `skills/momotombo-mode/kitchens/`), and commit.

With Laravel Boost (`boost.json`), Boost owns `AGENTS.md` and it is often gitignored. `init` then writes the docs map to `.ai/guidelines/docs-map.md`, which is versioned and which Boost adds to `AGENTS.md`. Run `php artisan boost:update` afterwards, and add new docs to that file by hand.

## Daily workflow

Talk to the agent in Spanish. It writes code and docs in English.

Every code task runs through the `momotombo-mode` skill (`/momotombo-mode <task>` to call it explicitly). It picks a playbook (investigate, bug fix, feature, big feature, refactor, perf, integrate, setup kitchen, autonomous run, parallel) and applies 26 stack-agnostic principles for code, tests, architecture, and security, from `skills/momotombo-mode/`.

- **Bugs and changes**: ask for it. The agent changes the code, runs the checks that cover it, and shows the evidence.
- **Large features**: the agent proposes the data shape, contracts, and phases in a few lines. You approve once, and it builds every phase.
- **Integrate**: `/commit` and `/open-pr` in Claude Code (ask for `git-ops` in Codex). Reviews run here: one pass, blocking findings fixed, the rest listed for you.
- **When you correct the agent**, it tries to turn the correction into a lint rule, test, or script first, then a doc with `/codely-doc-create`.

## Subagents

One table drives both tools (`global/AGENTS.md`):

| Agent | Claude | Codex | Handles |
|-------|--------|-------|---------|
| `git-ops` | haiku | gpt-6-luna, low | commits, branches, pushes, PRs |
| `analyst` | sonnet | session model, medium | read-only exploration and root-cause analysis |
| `reviewer` | sonnet | session model, medium | correctness and UI review when integrating |
| `security-reviewer` | opus | session model, high | authz, injection, secrets, payments, uploads |
| `comment-sicko` | sonnet | session model, medium | comments that don't earn their place (via `no-comments`) |
| `worker` | opus | session model, high | independent code tasks run in parallel |

Claude picks agents from their descriptions. Codex spawns them when the instructions say so.

## Safety

- **Guard hook** (`global/hooks/guard-bash.sh`): runs in Claude and Codex, including inside subagents. It blocks:
  - force-push, `--mirror`/`--all` pushes, `--no-verify` and `commit -n`, `core.hooksPath` overrides, `reset --hard`, `clean -f`, and `checkout .` / `restore .`
  - pushes to `main`/`master`, and commits, merges, cherry-picks, and reverts on `main`/`master`/`develop`, even after `cd` or with `git -C` (`pull` and `merge --ff-only` stay allowed)
  - `rm -r` on `/`, `~`, `.`, or `*`
  - printing, sourcing, or copying `.env` / `.env.*` files (`cat`, `grep`, `source`, `cp`, `git show`, …); templates like `.env.example` or `.env.local.example` stay readable. This check is best effort (an interpreter such as `node -e`, `find -exec`, or a path built at runtime can still read the file); the Read deny rules below are the real control
- **Codex `.rules`** forbid the same git operations, as a second line of defense.
- **Claude deny rules** block the Read tool on `.env` files (`.env`, `.env.local`, `.env.production`, `.env.staging`, …), `~/.ssh`, and AWS credentials.
- **Stall watcher** (`global/hooks/stall-watch.sh`, Claude only): a `PostToolUse` hook with `asyncRewake` that watches each command Claude runs with `run_in_background`. When the command has written no output for 10 minutes and its processes use under 5% CPU in total, it wakes Claude with a diagnosis (processes, the newest log and its last lines) and posts a macOS notification. It never stops anything. It reports once per command, and it doesn't report while the command listens on a port (a dev server). It only catches low-CPU stalls: a busy sibling process can hide a hung one. Tune it with `HARNESS_STALL_SECONDS`, `HARNESS_STALL_CPU`, and `HARNESS_STALL_POLL`. Codex has no equivalent hook, so its agents set their own check. It needs `lsof`, which macOS ships.
- **Comment guard** (`global/hooks/comment-guard.sh`, Claude and Codex): a `PostToolUse` hook on file edits (Claude `Edit`, `MultiEdit`, `Write`; Codex `apply_patch`). It lists the comments an edit added to source code (TypeScript, JavaScript, PHP, Python, CSS, Vue, Blade, Swift, Kotlin, Go, Rust, …) and asks the agent to remove them in the same turn. It lets through license headers, PHPDoc tags, lint and type directives, links, and comments the edit only moved, and skips `vendor/`, `node_modules/`, and build output. The edit is never undone.
- **Loop hook** (`global/hooks/loop-stop.sh`, Claude and Codex): a `Stop` hook for autonomous runs. `harness loop start --max N --done "<command>" "<goal>"` records a loop for the current repository (state in `~/.local/state/momotombo-harness/loops`, outside the repo). Each time the agent tries to stop, the hook runs the done command: if it passes, or after N rounds, the loop ends; otherwise the agent gets the goal and the failing output back and keeps working. `harness loop stop` ends it early. Without a loop it does nothing.

If a legitimate command is blocked, run it yourself (prefix it with `!` in Claude Code).

## Commands

```
harness install [--dry-run] [--force]   link and merge the global layer
harness status                          what is installed, missing, or broken
harness doctor                          tools, links, and hooks
harness undo [--dry-run] [--yes]        restore what the last install replaced
harness init [dir] [--stack laravel,react,expo,python] [--no-codely] [--dry-run]
harness validate                        what CI checks
harness loop start --max N --done "<command>" "<goal>"   keep the agent working until the command passes
harness loop status | stop               show or end the loop of this repository
harness codex-agents                    regenerate Codex agents from the Claude ones
```

## Repository layout

```
global/AGENTS.md         → ~/.claude/CLAUDE.md and ~/.codex/AGENTS.md
global/claude/           Claude agents, status line, settings (merged)
global/codex/            Codex agents (generated), .rules, hooks (merged)
global/hooks/            guard-bash.sh, comment-guard.sh, loop-stop.sh (both tools), stall-watch.sh (Claude)
skills/                  → ~/.claude/skills and ~/.agents/skills
project/                 what `harness init` copies: docs/ seeds and stacks/ lint configs
bin/harness              the CLI (bash + jq)
tests/                   Bats suites; CI runs them on macOS
```

## Credits

- [Laravel Boost](https://laravel.com/docs/boost) owns Laravel conventions, skills, docs search, and project rules in Laravel projects; the harness adds only what Boost doesn't cover.
- [pstack](https://github.com/cursor/plugins/tree/main/pstack) by poteto (MIT): `momotombo-mode`, its principles, `no-comments`, and `comment-sicko` are adapted from it.
- [agent-skills](https://github.com/addyosmani/agent-skills) by Addy Osmani (MIT): the `never-lower-the-bar` and `make-it-observable` principles and the expand/contract migration rule are adapted from it.
- Full license notices for both are in [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).
- [Codely's agent-harness](https://github.com/CodelyTV/agent-harness) inspired this project. `harness init` installs their `codely-doc-create` skill by reference.
