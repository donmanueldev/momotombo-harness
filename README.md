# agent-config

Personal Claude Code setup: subagents, skills, global instructions, a safety hook, a status line, and settings. Everything is versioned here and installed into `~/.claude` on every machine.

## Model routing

`home/CLAUDE.md` tells Claude which subagent handles each task, so each kind of work runs on the right model:

| Subagent | Model | Handles |
|----------|-------|---------|
| `git-ops` | haiku | commits, branches, pushes, pull requests |
| `analyst` | sonnet | read-only exploration, root-cause analysis, audits, research |
| `reviewer` | sonnet | read-only correctness review of a diff or PR |
| `ux-reviewer` | sonnet | UX states, accessibility, responsiveness, copy, design-system consistency |
| `security-reviewer` | opus | authz, injection, secrets, payments, uploads, dependencies |
| `planner` | opus | product framing, UX states, and sliced implementation plans |
| `implementer` | opus | production-quality code and tests |

Feature flow: `planner` → you approve → `implementer` → `reviewer` + `ux-reviewer` + `security-reviewer` (in parallel, as they apply) → fixes → `git-ops`.

To change a model, edit the `model:` line in `agents/<name>.md` (`haiku`, `sonnet`, `opus`, `inherit`, or a full `claude-*` id).

## Commands

| Command | What it does |
|---------|--------------|
| `/commit [note]` | Commits the current changes as Conventional Commits, in `git-ops` on haiku. Doesn't push. |
| `/open-pr [note]` | Commits anything pending, pushes, and opens or updates the PR (English title, Spanish body), in `git-ops` on haiku. |

## Safety

- `home/hooks/guard-bash.sh` is a PreToolUse hook. It also runs inside subagents. It blocks:
  - force-pushes, `--no-verify`, `reset --hard`, and `clean -f`
  - pushes to `main`/`master`, and commits on `main`/`master`/`develop`
  - `rm -r` on `/`, `~`, `$HOME`, `.`, or `*`
- `home/settings.json` denies reading `.env` files, `~/.ssh`, and AWS credentials.

## Settings

`home/settings.json` holds the portable policy: model, attribution, permissions, hooks, status line, and plugins. `install.sh` **merges** it into `~/.claude/settings.json`:

- Repo keys win.
- `permissions.allow/ask/deny` lists are unioned.
- Machine-specific keys (theme, `autoMode`, notifications) are kept.
- A backup is saved first, so `./sync.sh undo` restores it.

The repo owns `hooks.PreToolUse`. Add machine-only hooks under other events.

## Layout

```
agents/              → ~/.claude/agents/*.md
skills/<name>/       → ~/.claude/skills/<name>
home/CLAUDE.md       → ~/.claude/CLAUDE.md
home/rules/*.md      → ~/.claude/rules/
home/hooks/*         → ~/.claude/hooks/
home/statusline.sh   → ~/.claude/statusline.sh   (model · dir · branch* · ctx % · $cost)
home/settings.json   merged into ~/.claude/settings.json
tools/               Rust CLIs used by skills (audit-site, run-lighthouse, detect-web-adapter)
install.sh           build tools, link everything, merge settings (backs up whatever it replaces)
sync.sh              status / add / remove / delete / validate / undo
tests/               Bats suite (+ cargo test in tools/)
.github/workflows/   CI: clippy, cargo test, bats, validate
```

## Setup

Requires `jq` (ships with macOS) and [Rust](https://rustup.rs).

```bash
git clone git@github.com:donmanueldev/agent-config.git ~/agent-config
cd ~/agent-config
./install.sh --dry-run   # preview
./install.sh             # apply (prompts on conflicts; --force backs up and replaces)
```

## Day to day

```bash
./sync.sh                         # what is synced, local-only, or conflicting
./sync.sh add skill <name>        # move a local ~/.claude skill into the repo
./sync.sh add agent <name>        # same for a subagent
./sync.sh remove skill <name>     # stop tracking, keep the local copy
./sync.sh delete skill <name>     # remove from repo and ~/.claude
./sync.sh validate                # check frontmatter of skills and agents
./sync.sh pull                    # git pull + reinstall
./sync.sh undo                    # restore the last backup
bats tests/ && cargo test --manifest-path tools/Cargo.toml
```

## Skills in this repo

- Git: `commit`, `open-pr`, `commit-english` (preloaded by `git-ops`), `pr-comments`
- Swift: `swift-api-design-guidelines`, `swift-architecture`, `swift-codable`, `swift-testing-pro`
- Web and utilities: `agent-browser`, `deslop`, `favicon`, `fix-sentry-issues`, `knip`, `rams`, `pagespeed-optimization`, `website-seo-finalization`, `web-design-guidelines` (preloaded by `ux-reviewer`)

The SEO and PageSpeed skills call Rust binaries through `skills/<name>/bin/` symlinks into `tools/target/release/`. The repo has no Python.

Skills that ship through plugins or `~/.agents/skills` are not duplicated here.
