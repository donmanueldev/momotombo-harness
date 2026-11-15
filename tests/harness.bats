#!/usr/bin/env bats
# Tests for bin/harness global-layer commands, run against a fake HOME and a fake harness checkout.

setup() {
    PROJECT_DIR="$(cd "$BATS_TEST_DIRNAME/.." && pwd)"
    TEST_TMP="$(cd "$(mktemp -d)" && pwd -P)"
    export HOME="$TEST_TMP/home"
    export CODEX_HOME="$HOME/.codex"
    export HARNESS_LEGACY_DIRS="$TEST_TMP/old-checkout"
    REPO="$TEST_TMP/harness"
    mkdir -p "$HOME" "$REPO/bin" "$REPO/skills/demo" "$REPO/global/claude/agents" "$REPO/global/codex/agents" \
             "$REPO/global/claude/rules" "$REPO/global/codex/rules" "$REPO/global/hooks"
    cp "$PROJECT_DIR/bin/harness" "$REPO/bin/harness"
    printf -- '---\nname: demo\ndescription: Demo skill\n---\nBody\n' > "$REPO/skills/demo/SKILL.md"
    printf -- '---\nname: git-ops\ndescription: Git\nmodel: haiku\n---\nBody\n' > "$REPO/global/claude/agents/git-ops.md"
    "$REPO/bin/harness" codex-agents >/dev/null
    echo "# global" > "$REPO/global/AGENTS.md"
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
    backup=$(ls -d "$REPO"/.backup/*)
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
    ls -d "$REPO"/.backup/*.restored
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
