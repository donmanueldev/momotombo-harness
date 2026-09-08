# agent-config

My Codex configuration for skills, subagents, and synced workflow files.

## Quick start

```bash
git clone https://github.com/manuelduarte077/agent-config.git
cd agent-config
./install.sh
```

## What's included

### Codex
- `.codex/config.toml` - Project overlay for subagents and stack-specific plugin toggles
- `.codex/agents/*.toml` - Codex native subagents for specification, planning, orchestration, web, backend, infrastructure, mobile, and review
- `rules/agent-config.rules` - Safe command allowlist installed at `~/.codex/rules/agent-config.rules`

### Prompts
- `prompts/` - Copy-paste task starters for zero-to-one work, tickets, backend, frontend, fullstack, fixes, and features

### Skills
Reusable capabilities that your coding agents can invoke.

Matt Pocock skills are managed globally by the official `skills.sh` installer, outside this repository. Run `./sync.sh matt` when you explicitly want to install or update that source for Codex; `./install.sh --with-matt` does the same during setup. They are never copied into this repository, which avoids duplicate local copies.

| Skill | Description |
|-------|-------------|
| `agent-browser` | Evidence-based browser verification for web interfaces |
| `commit-english` | Write git commit messages in English |
| `fix-sentry-issues` | Investigate and address Sentry issues |
| `pr-comments` | Draft concise PR comments and review feedback |
| `rams` | Run accessibility and visual design review |
| `deslop` | Remove verified redundant code from an approved scope |
| `favicon` (optional) | Generate favicons from a source image |
| `knip` (optional) | Investigate potentially unused Node.js code and dependencies |

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

# Update Matt Pocock skills only
./sync.sh matt

# Install this configuration and explicitly reconcile Matt Pocock skills
./install.sh --with-matt

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
Matt Pocock skills are installed separately by `skills.sh` and reconciled by `./sync.sh matt`.

Project-specific Codex config lives in `.codex/config.toml` so the repo can narrow the skill and plugin surface without overwriting your home Codex config. For complex product work, use `specifier` to produce an approved, testable specification before `planner` and `orchestrator` delegate implementation. The specification preserves requirement IDs through planning, routing, verification, and review.

Prompt starters:
- `Turn this request into a spec with scope, non-goals, acceptance criteria, and a verification matrix. Wait for my approval before planning.`
- `Plan this feature, split it into web/backend/infra work, and tell me which subagents to run first.`
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
