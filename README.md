# agent-config

My Codex configuration for skills, subagents, and synced workflow files.

## Quick start

```bash
git clone https://github.com/brianlovin/agent-config.git
cd agent-config
./install.sh
```

## What's included

### Codex
- `.codex/config.toml` - Project overlay for subagents and stack-specific plugin toggles
- `.codex/agents/*.toml` - Codex native subagents for planning, web, backend, data, mobile, and review

### Prompts
- `prompts/` - Copy-paste task starters for zero-to-one work, tickets, backend, frontend, fullstack, fixes, and features

### Skills
Reusable capabilities that your coding agents can invoke.

| Skill | Description |
|-------|-------------|
| `ask-matt` | Route a task to the right Matt-style workflow skill |
| `agent-browser` | Browser automation for web testing and interaction |
| `codebase-design` | Establish architecture vocabulary and choose deep seams |
| `commit-english` | Write git commit messages in English |
| `codex-extensions` | Add or update Codex prompts, skills, agents, and config |
| `codex-rules` | Write and maintain repository instructions and guardrails |
| `favicon` | Generate favicons from a source image |
| `domain-modeling` | Keep the project domain glossary and model consistent |
| `workflow` | Orchestrate complex tasks with plans, subagents, and verification |
| `grill-with-docs` | Interview a plan in a real codebase and sharpen it with docs |
| `grill-me` | Stress-test a plan and turn it into execution notes |
| `improve-codebase-architecture` | Find deepening opportunities and propose refactors |
| `diagnosing-bugs` | Diagnose hard bugs and regressions with a tight feedback loop |
| `knip` | Find and remove unused files, dependencies, and exports |
| `rams` | Run accessibility and visual design review |
| `simplify` | Code simplification specialist |
| `deslop` | Remove AI-generated code slop |
| `setup-matt-pocock-skills` | Configure issue tracker, triage labels, and domain docs |
| `to-issues` | Break a PRD into independently grabbable vertical slices |
| `to-prd` | Turn a settled conversation into a concise PRD or brief |
| `pr-comments` | Draft concise PR comments and review feedback |
| `triage` | Triage incoming issues or PRs into clear states |

## Managing your config

```bash
# See what's synced vs local-only
./sync.sh

# Preview what install would do
./install.sh --dry-run

# Add a local skill to the repo
./sync.sh add skill my-skill
# Add a local subagent to the repo
./sync.sh add agent planner
./sync.sh push

# Delete a skill from repo and local machines
./sync.sh delete skill my-skill
./sync.sh push

# Pull changes on another machine
./sync.sh pull

# Remove a skill or subagent from repo (keeps local copy)
./sync.sh remove skill my-skill
./sync.sh remove agent planner
./sync.sh push
```

### Safe operations with backups

All destructive operations create timestamped backups:

```bash
# List available backups
./sync.sh backups

# Restore from last backup
./sync.sh undo
```

### Validate skills

```bash
./sync.sh validate
```

Skills must have a `SKILL.md` with frontmatter containing `name` and `description`.

## Testing

Tests use [Bats](https://github.com/bats-core/bats-core) (Bash Automated Testing System).

```bash
# Install bats (one-time)
brew install bats-core

# Run all tests
bats tests/

# Run specific test file
bats tests/install.bats
bats tests/sync.bats
bats tests/validation.bats
```

Tests run in isolated temp directories and don't affect your actual `~/.codex` config.
Tests also cover Codex skills syncing in `~/.codex/skills` and Codex agents in `~/.codex/agents`.

## Local-only config

Not everything needs to be synced. The install script only creates symlinks for what's in this repo - it won't delete your local-only skills or agents.

Use `./sync.sh remove skill <name>` when you want to stop syncing a skill but keep the local copy.
Use `./sync.sh remove agent <name>` when you want to stop syncing a subagent but keep the local copy.
Use `./sync.sh delete skill <name>` when you want to remove the skill from both the repo and the local `~/.codex` directories.

If a skill keeps coming back, it is usually coming from a plugin or marketplace install rather than this repo. Disable that source in Codex first, then delete the local skill.

Codex skills are linked from this repo into `~/.codex/skills`.
Codex agents from `.codex/agents/` are linked into `~/.codex/agents` by `./install.sh`.

Project-specific Codex config lives in `.codex/config.toml` so the repo can narrow the skill and plugin surface without overwriting your home Codex config. The subagents in `.codex/agents/` are the ones to use when you want Codex to plan first, then delegate.

Prompt starters:
- `Interview this plan using the repo docs and codebase, then tell me the safest execution path.`
- `Set up this repo for Matt-style engineering skills and align tracker, labels, and domain docs.`
- `Bootstrap a new repo for Matt-style skills using the setup skill and the repo conventions.`
- `Bootstrap a new repo with the short Matt-style prompt.`
- `Turn this conversation into a PRD and then split it into vertical-slice issues.`
- `Plan this feature, split it into web/backend/data work, and tell me which subagents to run first.`
- `Diagnose this bug and build the smallest reproducible loop before changing code.`
- `Review this diff for regressions, missing tests, and any risky behavior.`
- `Write a concise English commit message for this diff and explain the grouping.`
- `Draft PR review comments for this diff, ordered by severity.`
- `Update the repo rules or Codex extensions without reintroducing Claude/Cursor files.`

## Creating your own

Fork this repo and customize! The structure is simple:

```
agent-config/
├── .codex/            # Codex config overlay and native agents
├── prompts/           # Reusable task starters
├── skills/            # Skills (subdirectories with SKILL.md)
└── tests/             # Bats tests
```

## See also

- [My dotfiles](https://github.com/brianlovin/dotfiles) - Shell, git, SSH config
