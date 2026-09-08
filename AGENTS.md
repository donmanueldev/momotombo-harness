# Agent Config

Personal Codex skills, subagents, and sync helpers, synced across machines via symlinks.

## Alignment (local instruction policy)

- This repo defines behavior through this file and `custom-instructions.md`.
- Nested AGENTS files (in subdirectories) can refine behavior for specialized areas.
- Use `.rules` for temporary command controls; avoid embedding ad-hoc command bans in prompts.
- Keep policy and process decisions in versioned docs (`AGENTS.md`, `custom-instructions.md`, and skill docs). Memories are optional context only.

## Commands

```bash
./install.sh              # Set up symlinks (run after cloning)
./install.sh --dry-run    # Preview what would be done
./sync.sh                 # Show sync status
./sync.sh add <type> <name>     # Add a local item to repo
./sync.sh remove <type> <name>  # Remove item from repo (keeps local)
./sync.sh pull            # Pull latest and reinstall symlinks
./sync.sh push            # Commit and push changes
./sync.sh undo            # Restore from last backup
./sync.sh validate        # Validate all skills
./sync.sh backups         # List available backups
bats tests/               # Run tests
```

Types: `skill`, `agent`, `subagent`

## Testing

Tests use Bats. Run `bats tests/` to execute all tests. Tests run in isolated temp directories.

## Key Files

- `install.sh` - Creates symlinks from ~/.codex/skills and ~/.codex/agents to this repo
- `sync.sh` - Codex sync/status helper for skills and subagents
- `prompts/` - Reusable task starters for common Codex workflows
- `tests/` - Bats test suite
- `~/.codex/agent-config-docs` - Installed docs symlink after running `./install.sh`

## Verification

After making changes:
- `bats tests/` - Run all tests
- `./sync.sh validate` - Validate all skills
- Before merging instruction or control changes, re-read project instructions with `bats tests/` + `./sync.sh validate`.

## Codex Workflow

- Start complex product work with `specifier`; obtain explicit approval of its testable specification before asking `planner` for an execution plan.
- Use `orchestrator` for multi-agent routing; it is read-only and returns the required machine-readable route for the root agent to execute before implementation begins.
- Preserve `SPEC-*` and `REQ-*` IDs through planning, routing, validation, and review. Do not treat a green build as proof that a requirement is complete without its linked evidence.
- In Laravel projects with Laravel Boost, inspect its generated instructions and `.ai/rules/index.md` first, then load the relevant project-local skills (such as Inertia, Pest, or Filament when installed). Boost's package-aware guidance and project rules refine this generic configuration.
- Delegate one concern per subagent.
- Use `web`, `backend`, `infra`, `mobile`, and `reviewer` as the default routing map.
- Use `commit-english` for English commit messages and `pr-comments` for PR feedback. Keep prompts, skills, agents, and rules aligned with their repository documentation and validation.
- Keep this repo's `.codex/config.toml` as the project overlay instead of overwriting the user's home Codex config.

## Rules, subagents, and memory notes (Codex-specific)

- Use project-specific `.rules` files only for explicit command policy. Prefer:
  - `allow` when a command is safe and frequently needed
  - `prompt` when a command needs approval context
  - `forbidden` for dangerous commands in this workspace
- For this repo, avoid overriding global tool policies unless explicitly requested.
- Subagents: spawn explicitly for independent read-heavy analysis (security, tests, design, maintenability) and wait for summaries before touching shared files.
- Memories are supplemental context. Required operational constraints stay in AGENTS, `.codex/config.toml`, or `.rules`.

## Agent skills

- Issue tracker: GitHub Issues (external PRs are valid triage entry points). See [`docs/agents/issue-tracker.md`](docs/agents/issue-tracker.md).
- Triage labels: canonical mapping documented in [`docs/agents/triage-labels.md`](docs/agents/triage-labels.md).
- Domain docs: single-context posture. Current guidance documented in [`docs/agents/domain.md`](docs/agents/domain.md).

### Orchestration durability

- Keep decision history in `.codex/orchestration.jsonl` using `scripts/log-orchestration-event.sh`.
- Route all handoffs through the same contract format to keep execution state explicit and auditable.
- Before requesting work, use the standardized intake format in:
- `docs/orchestration-task-requests.md`
- Required intake fields: objective, scope, evidence, constraints, success criteria, and owner.
