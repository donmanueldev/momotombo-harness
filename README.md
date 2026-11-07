# agent-config

Personal Claude Code setup (subagents, skills, and global instructions), versioned here and symlinked into `~/.claude` on every machine.

## Model routing

`home/CLAUDE.md` tells Claude which subagent handles each task, so cheap work runs on a cheap model:

| Subagent | Model | Handles |
|----------|-------|---------|
| `git-ops` | haiku | commits, branches, pushes, pull requests |
| `analyst` | sonnet | read-only exploration, root-cause analysis, audits, research |
| `reviewer` | sonnet | read-only review of a diff or PR |
| `planner` | opus | implementation plans for multi-file work |
| `implementer` | opus | writing code and tests |

Typical feature flow: `planner` → you approve → `implementer` → `reviewer` → `git-ops`.

To change a model, edit the `model:` line in `agents/<name>.md` (`haiku`, `sonnet`, `opus`, `inherit`, or a full `claude-*` id).

## Layout

```
agents/        → ~/.claude/agents/*.md
skills/<name>/ → ~/.claude/skills/<name>
home/CLAUDE.md → ~/.claude/CLAUDE.md
tools/         Rust CLIs used by skills (audit-site, run-lighthouse, detect-web-adapter)
install.sh     build tools + link everything (backs up whatever it replaces)
sync.sh        status / add / remove / delete / validate / undo
tests/         Bats suite
```

## Setup

```bash
git clone https://github.com/manuelduarte077/agent-config.git ~/agent-config
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
bats tests/
```

## Skills in this repo

- Git: `commit-english` (preloaded by `git-ops`), `pr-comments`
- Swift: `swift-api-design-guidelines`, `swift-architecture`, `swift-codable`, `swift-testing-pro`
- Web and utilities: `agent-browser`, `deslop`, `favicon`, `fix-sentry-issues`, `knip`, `rams`, `pagespeed-optimization`, `website-seo-finalization`, `web-design-guidelines`

The SEO and PageSpeed skills call Rust binaries through `skills/<name>/bin/` symlinks into `tools/target/release/`. `install.sh` builds them with `cargo build --release`, so you need [Rust](https://rustup.rs). The repo has no Python.

Skills that ship through plugins or `~/.agents/skills` are not duplicated here.
