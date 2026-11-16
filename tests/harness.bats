#!/usr/bin/env bats
# Tests for bin/harness global-layer commands, run against a fake HOME and a fake harness checkout.

setup() {
    PROJECT_DIR="$(cd "$BATS_TEST_DIRNAME/.." && pwd)"
    TEST_TMP="$(cd "$(mktemp -d)" && pwd -P)"
    export HOME="$TEST_TMP/home"
    unset XDG_STATE_HOME HARNESS_BACKUP_DIR # keep backups inside the fake HOME
    export CODEX_HOME="$HOME/.codex"
    export HARNESS_LEGACY_DIRS="$TEST_TMP/old-checkout"
    REPO="$TEST_TMP/harness"
    mkdir -p "$HOME" "$CODEX_HOME" "$REPO/bin" "$REPO/skills/demo" "$REPO/global/claude/agents" "$REPO/global/codex/agents" \
             "$REPO/global/claude/rules" "$REPO/global/codex/rules" "$REPO/global/hooks"
    cp "$PROJECT_DIR/bin/harness" "$REPO/bin/harness"
    cp -R "$PROJECT_DIR/project" "$REPO/project"
    printf -- '---\nname: demo\ndescription: Demo skill\nlicense: MIT\nmetadata:\n  version: "1.0.0"\n---\nBody\n' > "$REPO/skills/demo/SKILL.md"
    printf -- '---\nname: git-ops\ndescription: Git\nmodel: haiku\n---\nBody\n' > "$REPO/global/claude/agents/git-ops.md"
    "$REPO/bin/harness" codex-agents >/dev/null
    printf '# global\n\n| Task | Agent | Claude | Codex |\n|---|---|---|---|\n| Commits | `git-ops` | haiku | gpt-6-luna, low |\n' > "$REPO/global/AGENTS.md"
    echo "# pr rules" > "$REPO/global/claude/rules/pr-titles.md"
    echo 'prefix_rule(pattern=["git", "push", "--force"], decision="forbidden")' > "$REPO/global/codex/rules/momotombo.rules"
    printf '#!/bin/bash\nexit 0\n' > "$REPO/global/hooks/guard-bash.sh"
    chmod +x "$REPO/global/hooks/guard-bash.sh"
    echo '{"model": "opus", "permissions": {"deny": ["Read(**/.env)"]}}' > "$REPO/global/claude/settings.json"
    echo '{"hooks": {"PreToolUse": [{"matcher": "^Bash$", "hooks": [{"type": "command", "command": "guard-bash.sh"}]}]}}' > "$REPO/global/codex/hooks.json"
}

teardown() {
    rm -rf "$TEST_TMP"
}

harness() {
    "$REPO/bin/harness" "$@"
}

@test "install links skills for both agents, instructions, agents, rules, and hooks" {
    run harness install
    [[ "$status" -eq 0 ]] || return 1
    [[ "$(readlink "$HOME/.claude/skills/demo")" == "$REPO/skills/demo" ]] || return 1
    [[ "$(readlink "$HOME/.agents/skills/demo")" == "$REPO/skills/demo" ]] || return 1
    [[ "$(readlink "$HOME/.claude/CLAUDE.md")" == "$REPO/global/AGENTS.md" ]] || return 1
    [[ "$(readlink "$HOME/.codex/AGENTS.md")" == "$REPO/global/AGENTS.md" ]] || return 1
    [[ "$(readlink "$HOME/.claude/agents/git-ops.md")" == "$REPO/global/claude/agents/git-ops.md" ]] || return 1
    [[ "$(readlink "$HOME/.codex/agents/git-ops.toml")" == "$REPO/global/codex/agents/git-ops.toml" ]] || return 1
    [[ "$(readlink "$HOME/.claude/rules/pr-titles.md")" == "$REPO/global/claude/rules/pr-titles.md" ]] || return 1
    [[ "$(readlink "$HOME/.codex/rules/momotombo.rules")" == "$REPO/global/codex/rules/momotombo.rules" ]] || return 1
    [[ "$(readlink "$HOME/.claude/hooks/guard-bash.sh")" == "$REPO/global/hooks/guard-bash.sh" ]] || return 1
    [[ "$(readlink "$HOME/.codex/hooks/guard-bash.sh")" == "$REPO/global/hooks/guard-bash.sh" ]] || return 1
    [[ "$(readlink "$HOME/.local/bin/harness")" == "$REPO/bin/harness" ]] || return 1
}

@test "install is idempotent and quiet the second time" {
    harness install
    run harness install
    [[ "$status" -eq 0 ]] || return 1
    [[ "$output" != *"✓ ~/"* ]] || return 1
    run harness status
    [[ "$status" -eq 0 ]] || return 1
}

@test "dry run changes nothing" {
    run harness install --dry-run
    [[ "$status" -eq 0 ]] || return 1
    [[ "$output" == *"[dry-run] would link ~/.claude/skills/demo"* ]] || return 1
    [[ ! -e "$HOME/.claude" ]] || return 1
}

@test "conflicts are kept without --force and replaced with a backup with --force" {
    mkdir -p "$HOME/.codex"
    echo "my own instructions" > "$HOME/.codex/AGENTS.md"

    run harness install
    [[ "$output" == *"exists and differs"* ]] || return 1
    grep -q "my own instructions" "$HOME/.codex/AGENTS.md"

    run harness install --force
    [[ "$(readlink "$HOME/.codex/AGENTS.md")" == "$REPO/global/AGENTS.md" ]] || return 1
    backup=$(ls -d "$HOME/.local/state/momotombo-harness/backups"/*)
    grep -q "my own instructions" "$backup/.codex/AGENTS.md"
}

@test "settings merge keeps local keys and unions permissions; codex hooks merge too" {
    mkdir -p "$HOME/.claude" "$HOME/.codex"
    echo '{"model": "sonnet", "theme": "dark", "permissions": {"deny": ["Bash(rm *)"]}}' > "$HOME/.claude/settings.json"
    echo '{"hooks": {"Stop": [{"hooks": [{"type": "command", "command": "notify"}]}]}}' > "$HOME/.codex/hooks.json"

    harness install

    run jq -e '.model == "opus" and .theme == "dark" and (.permissions.deny | sort) == ["Bash(rm *)", "Read(**/.env)"]' "$HOME/.claude/settings.json"
    [[ "$status" -eq 0 ]] || return 1
    run jq -e '.hooks.Stop[0].hooks[0].command == "notify" and .hooks.PreToolUse[0].matcher == "^Bash$"' "$HOME/.codex/hooks.json"
    [[ "$status" -eq 0 ]] || return 1
}

@test "invalid local settings are left untouched" {
    mkdir -p "$HOME/.claude"
    echo '{not json' > "$HOME/.claude/settings.json"
    run harness install
    [[ "$output" == *"not valid JSON; left unchanged"* ]] || return 1
    grep -q '{not json' "$HOME/.claude/settings.json"
}

@test "install prunes broken links from this harness and from legacy checkouts only" {
    mkdir -p "$HOME/.claude/skills" "$HOME/.codex/agents"
    ln -s "$REPO/skills/removed-skill" "$HOME/.claude/skills/removed-skill"
    ln -s "$HARNESS_LEGACY_DIRS/.codex/agents/old.toml" "$HOME/.codex/agents/old.toml"
    ln -s "/somewhere/else/x" "$HOME/.claude/skills/foreign"

    run harness install
    [[ "$status" -eq 0 ]] || return 1
    [[ ! -L "$HOME/.claude/skills/removed-skill" ]] || return 1
    [[ ! -L "$HOME/.codex/agents/old.toml" ]] || return 1
    [[ -L "$HOME/.claude/skills/foreign" ]] || return 1
}

@test "status reports missing and broken items" {
    run harness status
    [[ "$status" -ne 0 ]] || return 1
    [[ "$output" == *"not installed"* ]] || return 1
}

@test "undo restores replaced files and links" {
    mkdir -p "$HOME/.claude"
    echo "mine" > "$HOME/.claude/CLAUDE.md"
    echo '{"theme": "light"}' > "$HOME/.claude/settings.json"
    harness install --force

    run harness undo --yes
    [[ "$status" -eq 0 ]] || return 1
    [[ ! -L "$HOME/.claude/CLAUDE.md" ]] || return 1
    grep -q "mine" "$HOME/.claude/CLAUDE.md"
    run jq -e '.theme == "light" and (has("model") | not)' "$HOME/.claude/settings.json"
    [[ "$status" -eq 0 ]] || return 1
    ls -d "$HOME/.local/state/momotombo-harness/backups"/*.restored
}

@test "undo refuses without confirmation when not interactive" {
    mkdir -p "$HOME/.claude"
    echo "mine" > "$HOME/.claude/CLAUDE.md"
    harness install --force
    run harness undo < /dev/null
    [[ "$status" -ne 0 ]] || return 1
    [[ "$output" == *"pass --yes"* ]] || return 1
}

@test "validate passes on a well-formed harness and catches broken agents" {
    run harness validate
    [[ "$status" -eq 0 ]] || return 1

    printf -- '---\nname: bad\ndescription: x\nmodel: gpt-4\n---\n' > "$REPO/global/claude/agents/bad.md"
    run harness validate
    [[ "$status" -ne 0 ]] || return 1
    [[ "$output" == *"claude agent bad: 'model'"* ]] || return 1
}

@test "validate catches an agent that preloads a skill the harness doesn't ship" {
    printf -- '---\nname: git-ops\ndescription: Git\nmodel: haiku\nskills:\n  - demo\n---\nBody\n' > "$REPO/global/claude/agents/git-ops.md"
    run harness validate
    [[ "$output" != *"preloads missing skill"* ]] || return 1

    printf -- '---\nname: git-ops\ndescription: Git\nmodel: haiku\nskills:\n  - gone\n---\nBody\n' > "$REPO/global/claude/agents/git-ops.md"
    run harness validate
    [[ "$status" -ne 0 ]] || return 1
    [[ "$output" == *"claude agent git-ops: preloads missing skill 'gone'"* ]] || return 1
}

@test "validate catches routing tables that drift from the agents' models" {
    printf -- '---\nname: git-ops\ndescription: Git\nmodel: sonnet\n---\nBody\n' > "$REPO/global/claude/agents/git-ops.md"
    harness codex-agents >/dev/null
    run harness validate
    [[ "$status" -ne 0 ]] || return 1
    [[ "$output" == *"global/AGENTS.md: routing table says git-ops runs on 'haiku' in Claude; its frontmatter says 'sonnet'"* ]] || return 1
    [[ "$output" == *"expected 'session model, medium'"* ]] || return 1

    printf -- '---\nname: planner\ndescription: Plan\nmodel: opus\n---\nBody\n' > "$REPO/global/claude/agents/planner.md"
    printf '| Agent | Claude | Codex |\n|---|---|---|\n| `git-ops` | sonnet | session model, medium |\n' > "$REPO/README.md"
    harness codex-agents >/dev/null
    run harness validate
    [[ "$output" == *"global/AGENTS.md: routing table has no row for agent planner"* ]] || return 1
    [[ "$output" == *"README.md: routing table has no row for agent planner"* ]] || return 1
    [[ "$output" != *"README.md: routing table says git-ops"* ]] || return 1
}

@test "validate reads only the routing table and catches unknown, duplicate, or missing entries" {
    printf '| Command | What | Notes |\n|---|---|---|\n| `git-ops` | not a routing row | x |\n\n| Agent | Claude | Codex |\n|---|---|---|\n| `git-ops` | haiku | gpt-6-luna, low |\n' > "$REPO/README.md"
    run harness validate
    [[ "$status" -eq 0 ]] || { echo "$output"; return 1; }

    printf '| Agent | Claude | Codex |\n|---|---|---|\n| `git-ops` | haiku | gpt-6-luna, low |\n| `git-ops` | haiku | gpt-6-luna, low |\n| `reveiwer` | sonnet | session model, medium |\n' > "$REPO/README.md"
    run harness validate
    [[ "$status" -ne 0 ]] || return 1
    [[ "$output" == *"README.md: routing table lists 'reveiwer', which has no global/claude/agents/reveiwer.md"* ]] || return 1
    [[ "$output" == *"README.md: routing table lists 'git-ops' more than once"* ]] || return 1

    rm "$REPO/global/AGENTS.md"
    run harness validate
    [[ "$status" -ne 0 ]] || return 1
    [[ "$output" == *"global/AGENTS.md: missing"* ]] || return 1
}

@test "the real repository passes validate" {
    run "$PROJECT_DIR/bin/harness" validate
    [[ "$status" -eq 0 ]] || return 1
}

@test "hook merge appends to existing events and never drops other tools' hooks" {
    mkdir -p "$HOME/.codex"
    echo '{"hooks": {"PreToolUse": [{"hooks": [{"type": "command", "command": "orca-hook"}]}]}}' > "$HOME/.codex/hooks.json"

    harness install
    harness install

    run jq -e '(.hooks.PreToolUse | length) == 2
        and .hooks.PreToolUse[0].hooks[0].command == "orca-hook"
        and .hooks.PreToolUse[1].matcher == "^Bash$"' "$HOME/.codex/hooks.json"
    [[ "$status" -eq 0 ]] || return 1
}

@test "a symlinked settings file is replaced, never written through, and undo brings the link back" {
    mkdir -p "$HOME/.claude" "$HOME/dots"
    echo '{"theme": "dark"}' > "$HOME/dots/settings.json"
    ln -s "$HOME/dots/settings.json" "$HOME/.claude/settings.json"

    harness install
    [[ ! -L "$HOME/.claude/settings.json" ]] || return 1
    run jq -e '.theme == "dark" and .model == "opus"' "$HOME/.claude/settings.json"
    [[ "$status" -eq 0 ]] || return 1
    [[ "$(cat "$HOME/dots/settings.json")" == '{"theme": "dark"}' ]] || return 1

    harness undo --yes
    [[ "$(readlink "$HOME/.claude/settings.json")" == "$HOME/dots/settings.json" ]] || return 1
}

@test "a broken settings symlink is left alone" {
    mkdir -p "$HOME/.claude"
    ln -s "$HOME/gone/settings.json" "$HOME/.claude/settings.json"
    run harness install
    [[ "$output" == *"broken symlink; left unchanged"* ]] || return 1
    [[ ! -e "$HOME/gone" ]] || return 1
}

@test "an install that fails midway still leaves a restorable backup" {
    mkdir -p "$HOME/.claude" "$HOME/.codex"
    echo "mine" > "$HOME/.claude/CLAUDE.md"
    chmod 555 "$HOME/.codex"   # linking ~/.codex/AGENTS.md fails right after CLAUDE.md was backed up
    run harness install --force
    chmod 755 "$HOME/.codex"
    [[ "$status" -ne 0 ]] || return 1

    run harness undo --yes
    [[ "$status" -eq 0 ]] || { echo "$output"; return 1; }
    grep -qx "mine" "$HOME/.claude/CLAUDE.md" || return 1
}

@test "undo skips a newer backup folder that has no manifest" {
    mkdir -p "$HOME/.claude"
    echo "mine" > "$HOME/.claude/CLAUDE.md"
    harness install --force
    mkdir -p "$HOME/.local/state/momotombo-harness/backups/99999999_999999"

    run harness undo --yes
    [[ "$status" -eq 0 ]] || { echo "$output"; return 1; }
    grep -qx "mine" "$HOME/.claude/CLAUDE.md" || return 1
}

@test "install refuses a CODEX_HOME outside HOME" {
    CODEX_HOME="$TEST_TMP/elsewhere" run harness install
    [[ "$status" -ne 0 ]] || return 1
    [[ "$output" == *"must be inside \$HOME"* ]] || return 1
}

@test "install doesn't delete sources reached through a linked parent folder" {
    mkdir -p "$HOME/.claude"
    ln -s "$REPO/global/claude/agents" "$HOME/.claude/agents"
    run harness install --force
    [[ "$status" -eq 0 ]] || return 1
    [[ -f "$REPO/global/claude/agents/git-ops.md" && ! -L "$REPO/global/claude/agents/git-ops.md" ]] || return 1
}

@test "install keeps backups outside the checkout, and undo still restores an old in-checkout backup" {
    mkdir -p "$HOME/.claude"
    echo "mine" > "$HOME/.claude/CLAUDE.md"
    harness install --force
    [[ ! -e "$REPO/.backup" ]] || return 1
    [[ -f "$(ls -d "$HOME/.local/state/momotombo-harness/backups"/*)/manifest.json" ]] || return 1

    mkdir -p "$REPO/.backup"
    mv "$HOME/.local/state/momotombo-harness/backups"/* "$REPO/.backup/"
    run harness undo --yes
    [[ "$status" -eq 0 ]] || { echo "$output"; return 1; }
    grep -qx "mine" "$HOME/.claude/CLAUDE.md" || return 1
}

@test "doctor warns about an old openspec and a PATH without ~/.local/bin" {
    mkdir -p "$TEST_TMP/bin"
    printf '#!/bin/bash\necho 1.9.3\n' > "$TEST_TMP/bin/openspec"
    chmod +x "$TEST_TMP/bin/openspec"
    PATH="$TEST_TMP/bin:/usr/bin:/bin:/opt/homebrew/bin:/usr/local/bin" run harness doctor
    [[ "$output" == *"openspec 1.9.3 is older than 2.0.0"* ]] || { echo "$output"; return 1; }
    [[ "$output" == *"~/.local/bin is not on PATH"* ]] || return 1

    printf '#!/bin/bash\necho 2.1.0\n' > "$TEST_TMP/bin/openspec"
    PATH="$TEST_TMP/bin:$HOME/.local/bin:/usr/bin:/bin:/opt/homebrew/bin:/usr/local/bin" run harness doctor
    [[ "$output" == *"openspec 2.1.0"* ]] || return 1
    [[ "$output" != *"older than"* ]] || return 1
    [[ "$output" == *"~/.local/bin is on PATH"* ]] || return 1
}

@test "validate requires a license and a semver metadata.version on every skill" {
    printf -- '---\nname: demo\ndescription: Demo skill\n---\nBody\n' > "$REPO/skills/demo/SKILL.md"
    run harness validate
    [[ "$status" -ne 0 ]] || return 1
    [[ "$output" == *"skills/demo: missing 'license'"* ]] || return 1
    [[ "$output" == *"skills/demo: missing 'metadata.version' (semver)"* ]] || return 1

    printf -- '---\nname: demo\ndescription: Demo skill\nlicense: MIT\nmetadata:\n  author: x\n  version: "1.2.0"\n---\nBody\n' > "$REPO/skills/demo/SKILL.md"
    run harness validate
    [[ "$output" != *"skills/demo"* ]] || { echo "$output"; return 1; }
}

@test "undo restores a legacy in-checkout backup when the new backup folder was never created" {
    mkdir -p "$HOME/.claude"
    echo "mine" > "$HOME/.claude/CLAUDE.md"
    harness install --force
    mkdir -p "$REPO/.backup"
    mv "$HOME/.local/state/momotombo-harness/backups"/* "$REPO/.backup/"
    rm -rf "$HOME/.local/state"

    run harness undo --yes
    [[ "$status" -eq 0 ]] || { echo "$output"; return 1; }
    grep -qx "mine" "$HOME/.claude/CLAUDE.md" || return 1
}

@test "undo explains when there is no backup anywhere" {
    run harness undo --yes
    [[ "$status" -ne 0 ]] || return 1
    [[ "$output" == *"no backup to restore"* ]] || return 1
}

@test "backups are readable only by their owner" {
    mkdir -p "$HOME/.claude"
    echo "mine" > "$HOME/.claude/CLAUDE.md"
    harness install --force
    [[ "$(stat -f %Lp "$HOME/.local/state/momotombo-harness/backups")" == 700 ]] || return 1
    [[ "$(stat -f %Lp "$(ls -d "$HOME/.local/state/momotombo-harness/backups"/*)")" == 700 ]] || return 1
}

@test "doctor finishes even when a tool prints no version" {
    mkdir -p "$TEST_TMP/bin"
    printf '#!/bin/bash\necho dev-build\n' > "$TEST_TMP/bin/codex"
    printf '#!/bin/bash\nexit 3\n' > "$TEST_TMP/bin/gh"
    chmod +x "$TEST_TMP/bin/codex" "$TEST_TMP/bin/gh"
    PATH="$TEST_TMP/bin:$PATH" run harness doctor
    [[ "$output" == *"Links"* ]] || { echo "$output"; return 1; }
    [[ "$output" == *"Settings"* ]] || return 1
}

@test "validate rejects a skill version under another key or with a mismatched quote" {
    printf -- '---\nname: demo\ndescription: Demo skill\nlicense: MIT\nother:\n  version: "1.0.0"\nmetadata:\n  author: x\n---\nBody\n' > "$REPO/skills/demo/SKILL.md"
    run harness validate
    [[ "$output" == *"skills/demo: missing 'metadata.version' (semver)"* ]] || return 1

    printf -- '---\nname: demo\ndescription: Demo skill\nlicense: MIT\nmetadata:\n  version: "1.0.0\n---\nBody\n' > "$REPO/skills/demo/SKILL.md"
    run harness validate
    [[ "$output" == *"skills/demo: missing 'metadata.version' (semver)"* ]] || return 1
}

@test "without Codex, install and doctor skip its links and hook instead of failing" {
    rmdir "$CODEX_HOME"
    mkdir -p "$TEST_TMP/bin" && ln -s "$(command -v jq)" "$TEST_TMP/bin/jq"
    export PATH="$TEST_TMP/bin:/usr/bin:/bin"   # no codex command
    run harness install
    [[ "$status" -eq 0 ]] || return 1
    [[ "$output" == *"Codex not found; its links and hooks were skipped"* ]] || return 1
    [[ ! -e "$CODEX_HOME" ]] || return 1
    [[ "$(readlink "$HOME/.claude/CLAUDE.md")" == "$REPO/global/AGENTS.md" ]] || return 1
    run harness doctor
    [[ "$output" == *"Codex not installed; its links and guard hook are skipped"* ]] || return 1
    [[ "$output" != *".codex"* ]] || { echo "$output"; return 1; }
    [[ "$output" == *"links installed"* && "$output" != *"need attention"* ]] || { echo "$output"; return 1; }
}

@test "a codex command on PATH counts as Codex installed, even before ~/.codex exists" {
    rmdir "$CODEX_HOME"
    mkdir -p "$TEST_TMP/bin" && ln -s "$(command -v jq)" "$TEST_TMP/bin/jq"
    printf '#!/bin/bash\n' > "$TEST_TMP/bin/codex" && chmod +x "$TEST_TMP/bin/codex"
    export PATH="$TEST_TMP/bin:/usr/bin:/bin"
    run harness install
    [[ "$status" -eq 0 ]] || return 1
    [[ "$(readlink "$CODEX_HOME/AGENTS.md")" == "$REPO/global/AGENTS.md" ]] || return 1
    jq -e '.hooks.PreToolUse[0].hooks[0].command == "guard-bash.sh"' "$CODEX_HOME/hooks.json" >/dev/null || return 1
}
