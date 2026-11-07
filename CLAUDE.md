# agent-config

Personal Claude Code configuration (subagents, skills, global CLAUDE.md), synced across machines through symlinks into `~/.claude`.

## Layout

- `agents/*.md` → `~/.claude/agents/`: subagents with model routing (git-ops=haiku, analyst/reviewer=sonnet, planner/implementer=opus).
- `skills/<name>/` → `~/.claude/skills/<name>`: versioned skills. Each one needs a `SKILL.md` with `name` and `description` frontmatter.
- `home/CLAUDE.md` → `~/.claude/CLAUDE.md`: global instructions and the routing table.
- `tools/`: Rust crate with the CLIs that skills run through `skills/<name>/bin/` symlinks. Build: `cargo build --release --manifest-path tools/Cargo.toml`.

## Commands

```bash
./install.sh [--dry-run|--force]   # link everything into ~/.claude
./sync.sh                          # status
./sync.sh add|remove|delete <skill|agent> <name>
./sync.sh validate                 # validate skills and agents
./sync.sh undo | backups
bats tests/                        # test suite
```

## Rules for changes here

- After changing scripts, skills, or agents, run `bats tests/` and `./sync.sh validate`.
- Agents need `name`, `description`, and `model` frontmatter. Keep the routing table in `home/CLAUDE.md` in sync with `agents/`.
- No Python. New skill scripts go in `tools/src/bin/` as Rust binaries, or as plain bash.
- Don't add skills that already ship through plugins or `~/.agents/skills`.
