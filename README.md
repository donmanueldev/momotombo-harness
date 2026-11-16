# momotombo-harness

The agent harness Momotombo uses to build products with **Claude Code** and **Codex**. It turns "ask the AI and hope" into a controlled loop:

- You approve specs before any code is written.
- Agents implement one small phase at a time and stop for review.
- Every correction you make becomes a convention the agents follow next time.

It stands on three projects: [OpenSpec](https://openspec.dev/) for the spec-driven flow, [Codely's agent-harness](https://github.com/CodelyTV/agent-harness), which inspired its structure, and [Matt Pocock's skills](https://www.aihero.dev/skills), whose shaping and engineering skills plug into the flow. See [Credits](#credits).

## Why

Most AI slop has five causes. The harness answers each one:

| Cause | Answer |
|-------|--------|
| The agent guesses your conventions | `docs/` per project with good and bad examples. `AGENTS.md` maps them so agents read only what the task needs |
| Tasks are too big to review | OpenSpec specs plus a plan of vertical-slice phases with contracts **you approve first** |
| The same mistake comes back | `codely-doc-create` turns a correction into a versioned doc |
| "Done" without proof | Every phase ends with verification, and agents report exactly what ran |
| Generic, unsafe code | A product quality bar, stack conventions, and UX and security reviewer agents |

## Two layers

```
GLOBAL (your machine)                      PROJECT (each product repo)
harness install                            harness init
───────────────────────────────            ─────────────────────────────────────
~/.claude + ~/.codex + ~/.agents           AGENTS.md (= CLAUDE.md): commands + docs map
  · global instructions (one file)         docs/: conventions that grow with every correction
  · 7 subagents, routed by model           openspec/: specs + the momotombo schema
  · /commit, /open-pr                      Codely skills, pinned in skills-lock.json
  · guard hook + Codex rules
  · status line, settings
```

## Setup

Requirements: macOS, `git`, `jq`, Node 22+ (for `npx`), [OpenSpec](https://openspec.dev) (`brew install openspec`), and Claude Code and/or Codex. Without Codex, `install` and `doctor` skip its links and hook.

```bash
git clone git@github.com:donmanueldev/momotombo-harness.git ~/momotombo-harness
~/momotombo-harness/bin/harness install --dry-run   # preview
~/momotombo-harness/bin/harness install             # apply; then `harness` is on your PATH
harness doctor
```

`install` never deletes your data:
- A file that differs from the harness is kept, unless you pass `--force`. With `--force` it is backed up first.
- Settings are **merged**: your keys and other tools' hooks stay.
- `harness undo` restores the last backup. Backups live in `~/.local/state/momotombo-harness/backups` (override with `HARNESS_BACKUP_DIR`), outside the checkout, because they hold copies of your own config.

In Codex, approve the guard hook once with `/hooks`.

## Start a product

```bash
cd ~/momotombo/my-product
harness init                     # detects Laravel or Expo, or pass --stack laravel,expo
```

Then fill in the Architecture section of `AGENTS.md`, trim the seeded `docs/`, and commit.

With Laravel Boost (`boost.json`), Boost owns `AGENTS.md` and it is often gitignored. `init` then writes the docs map to `.ai/guidelines/docs-map.md`, which is versioned and which Boost adds to `AGENTS.md`. Run `php artisan boost:update` afterwards, and add new docs to that file by hand.

## Daily workflow

Talk to the agent in Spanish. It writes code, specs, and docs in English.

| Step | Claude Code | Codex | You |
|------|-------------|-------|-----|
| 1. Think it through | `/openspec-explore` | `$openspec-explore` | Discuss options, no code yet |
| 2. Propose | `/openspec-propose add-x` | `$openspec-propose add-x` | **Approve the specs and contracts** |
| 3. Build one phase | `/openspec-apply-change` | `$openspec-apply-change` | Review the diff (it stops, never commits) |
| 4. Commit | `/commit` | ask for `git-ops` | Check the commits it made (apply also suggests messages if you prefer to write your own) |
| 5. Repeat 3–4 per phase, then open the PR | `/open-pr` | ask for `git-ops` | |
| 6. Archive | `/openspec-archive-change` | `$openspec-archive-change` | Specs now describe the system |

- **When you correct the agent**, run `/codely-doc-create`. The correction becomes a doc and won't repeat.
- **Before changing old code that has no spec**, run `/codely-openspec-spec-create-from-code <capability>`.
- **Small fixes** skip OpenSpec: ask directly, verify, then `/commit`.

## Subagents

One table drives both tools (`global/AGENTS.md`):

| Agent | Claude | Codex | Handles |
|-------|--------|-------|---------|
| `git-ops` | haiku | gpt-6-luna, low | commits, branches, pushes, PRs |
| `analyst` | sonnet | session model, medium | read-only exploration and root-cause analysis |
| `reviewer` | sonnet | session model, medium | correctness review |
| `ux-reviewer` | sonnet | session model, medium | UI states, accessibility, responsiveness, copy |
| `security-reviewer` | opus | session model, high | authz, injection, secrets, payments, uploads |
| `planner` | opus | session model, high | plans outside OpenSpec |
| `implementer` | opus | session model, high | production-quality code and tests |

Claude picks agents from their descriptions. Codex spawns them when the instructions say so.

## Safety

- **Guard hook** (`global/hooks/guard-bash.sh`): runs in Claude and Codex, including inside subagents. It blocks:
  - force-push, `--no-verify`, `reset --hard`, and `clean -f`
  - pushes to `main`/`master`, and commits on `main`/`master`/`develop`, even after `cd` or with `git -C`
  - `rm -r` on `/`, `~`, `.`, or `*`
  - printing or sourcing `.env` / `.env.*` files (`cat`, `grep`, `source`, …); templates like `.env.example` stay readable. This check is best effort (an interpreter such as `node -e` can still read the file); the Read deny rules below are the real control
- **Codex `.rules`** forbid the same git operations, as a second line of defense.
- **Claude deny rules** block the Read tool on `.env` files (`.env`, `.env.local`, `.env.production`, `.env.staging`, …), `~/.ssh`, and AWS credentials.

If a legitimate command is blocked, run it yourself (prefix it with `!` in Claude Code).

## Commands

```
harness install [--dry-run] [--force]   link and merge the global layer
harness status                          what is installed, missing, or broken
harness doctor                          tools, links, and hooks
harness undo [--dry-run] [--yes]        restore what the last install replaced
harness cleanup [--dry-run]             list simulators, emulators, containers, and dev servers; ask before stopping each
harness init [dir] [--stack laravel,expo] [--no-codely] [--dry-run]
harness validate                        what CI checks
harness codex-agents                    regenerate Codex agents from the Claude ones
```

## Repository layout

```
global/AGENTS.md         → ~/.claude/CLAUDE.md and ~/.codex/AGENTS.md
global/claude/           Claude agents, status line, settings (merged)
global/codex/            Codex agents (generated), .rules, hooks (merged)
global/hooks/            guard-bash.sh, shared by both tools
skills/                  → ~/.claude/skills and ~/.agents/skills
project/                 what `harness init` copies: docs/ seeds, the OpenSpec schema and config
bin/harness              the CLI (bash + jq)
tests/                   Bats suites; CI runs them on macOS
```

## Credits

- [OpenSpec](https://github.com/Fission-AI/OpenSpec) provides the spec-driven workflow.
- [Codely's agent-harness](https://github.com/CodelyTV/agent-harness) inspired this project:
  - The `momotombo` schema is adapted from their `codely-spec-plan`.
  - `harness init` installs their `codely-doc-create` and `codely-openspec-spec-create-from-code` skills by reference.
- [Matt Pocock's skills](https://github.com/mattpocock/skills) ([aihero.dev/skills](https://www.aihero.dev/skills)) shaped how the harness plans and builds. Their shaping and engineering skills plug into the OpenSpec flow instead of running a second one:
  - before `explore`: `grill-me`, `grill-with-docs`
  - in `propose`: `domain-modeling`, `codebase-design`
  - in `apply`: `tdd`; for bugs: `diagnosing-bugs`
  - Install them once: `npx skills add mattpocock/skills -g -s grill-me grill-with-docs domain-modeling codebase-design tdd diagnosing-bugs improve-codebase-architecture`
