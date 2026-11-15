# momotombo-harness

The Momotombo agent harness for Claude Code and Codex. It has two layers:

- **Global** (`global/`, `skills/`): installed on a developer machine by `bin/harness install`. It links into `~/.claude`, `~/.codex`, and `~/.agents`.
- **Project** (`project/`): copied into a product repository by `bin/harness init`.

## Layout

- `global/AGENTS.md` → `~/.claude/CLAUDE.md` and `~/.codex/AGENTS.md`: one set of global instructions for both tools.
- `global/claude/`: Claude subagents (`agents/*.md`), the rules, the status line, and the settings merged into `~/.claude/settings.json`.
- `global/codex/`: Codex custom agents (`agents/*.toml`), the `.rules` command policy, and the hooks merged into `~/.codex/hooks.json`.
- `global/hooks/guard-bash.sh`: the PreToolUse guard shared by Claude and Codex.
- `skills/<name>/` → `~/.claude/skills/` and `~/.agents/skills/` (Codex): skills shared by both tools.
- `bin/harness`: the CLI (`install`, `init`, `status`, `doctor`, `undo`, `validate`).
- `tests/`: Bats suites. CI runs `bin/harness validate` and `bats tests/` on macOS.

## Commands

```bash
bin/harness install --dry-run   # preview, then: bin/harness install
bin/harness doctor              # tools, links, hooks
bin/harness validate            # what CI checks
bats tests/
```

## Rules for changes here

- Run `bin/harness validate` and `bats tests/` before committing.
- Bats runs on macOS bash 3.2, where a failing `[[ ]]` mid-test does NOT fail the test. End every `[[ ]]` assertion with `|| return 1`.
- Every subagent exists for both tools: `global/claude/agents/<name>.md` and `global/codex/agents/<name>.toml`, with the same name and role. Keep the routing table in `global/AGENTS.md` in sync with them.
- Changes to `global/hooks/guard-bash.sh` need a blocked case and an allowed case in `tests/hooks.bats`.
- No Python and no compiled tools. Scripts are bash plus `jq`.
- Don't add skills that already ship through plugins, OpenSpec, Laravel Boost, or the Codely skills.
- Everything in the repo is in English.
