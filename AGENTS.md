# Agent Config

Personal Codex skills, subagents, and sync helpers, synced across machines via symlinks.

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

## Verification

After making changes:
- `bats tests/` - Run all tests
- `./sync.sh validate` - Validate all skills

## Codex Workflow

- Start complex work with `ask-matt`, `planner`, `grill-with-docs`, `grill-me`, or `setup-matt-pocock-skills` for repo bootstrap.
- Delegate one concern per subagent.
- Use `web`, `backend`, `data`, `mobile`, and `reviewer` as the default routing map.
- Use `setup-matt-pocock-skills` when a repo needs its issue tracker, triage labels, and domain-doc layout aligned.
- Use `to-prd` when the conversation is ready to become a spec, `to-issues` when that spec needs vertical slices, `triage` for incoming tickets or PRs, `improve-codebase-architecture` for refactor opportunities, and `domain-modeling` for glossary work.
- Use `diagnosing-bugs` for hard failures, flakes, and performance regressions.
- Use `commit-english` for English commit messages, `pr-comments` for PR feedback, `codex-extensions` for prompts/skills/agents/config, and `codex-rules` for repo instructions.
- Keep this repo's `.codex/config.toml` as the project overlay instead of overwriting the user's home Codex config.
