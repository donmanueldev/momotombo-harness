#!/usr/bin/env bats
# Tests for install.sh

load 'test_helper'

setup() {
    setup_test_env
}

teardown() {
    teardown_test_env
}

# =============================================================================
# Basic Installation Tests
# =============================================================================

@test "install.sh creates ~/.claude directory" {
    create_fake_skill "test-skill"
    run_install
    assert_dir "$FAKE_HOME/.claude"
}

@test "install.sh symlinks skills" {
    create_fake_skill "test-skill"
    run_install
    assert_symlink "$FAKE_HOME/.claude/skills/test-skill" "$FAKE_REPO/skills/test-skill"
}

@test "install.sh symlinks multiple skills" {
    create_fake_skill "skill-one"
    create_fake_skill "skill-two"
    create_fake_skill "skill-three"
    run_install
    assert_symlink "$FAKE_HOME/.claude/skills/skill-one" "$FAKE_REPO/skills/skill-one"
    assert_symlink "$FAKE_HOME/.claude/skills/skill-two" "$FAKE_REPO/skills/skill-two"
    assert_symlink "$FAKE_HOME/.claude/skills/skill-three" "$FAKE_REPO/skills/skill-three"
}

@test "install.sh links global CLAUDE.md" {
    mkdir -p "$FAKE_REPO/home"
    echo "# global" > "$FAKE_REPO/home/CLAUDE.md"
    run_install
    assert_symlink "$FAKE_HOME/.claude/CLAUDE.md" "$FAKE_REPO/home/CLAUDE.md"
}

@test "install.sh backs up an existing global CLAUDE.md with --force" {
    mkdir -p "$FAKE_REPO/home" "$FAKE_HOME/.claude"
    echo "# global" > "$FAKE_REPO/home/CLAUDE.md"
    echo "# mine" > "$FAKE_HOME/.claude/CLAUDE.md"
    run_install --force
    assert_symlink "$FAKE_HOME/.claude/CLAUDE.md" "$FAKE_REPO/home/CLAUDE.md"
    grep -q "# mine" "$FAKE_REPO/.backup/$(get_latest_backup)/.claude/CLAUDE.md"
}

@test "install.sh symlinks agents" {
    create_fake_agent "planner"
    run_install
    assert_symlink "$FAKE_HOME/.claude/agents/planner.md" "$FAKE_REPO/agents/planner.md"
}

# =============================================================================
# Dry Run Tests
# =============================================================================

@test "install.sh --dry-run doesn't create symlinks" {
    create_fake_skill "test-skill"
    create_fake_agent "planner"
    run_install --dry-run
    [[ ! -L "$FAKE_HOME/.claude/skills/test-skill" ]]
    [[ ! -L "$FAKE_HOME/.claude/agents/planner.md" ]]
    [[ ! -d "$FAKE_HOME/.claude/agents" ]]
}

@test "install.sh --dry-run shows what would be done" {
    create_fake_skill "test-skill"
    run run_install --dry-run
    [[ "$output" == *"[dry-run]"* ]]
    [[ "$output" == *"Would link"* ]]
}

@test "install.sh -n is alias for --dry-run" {
    create_fake_skill "test-skill"
    run run_install -n
    [[ "$output" == *"[dry-run]"* ]]
}

# =============================================================================
# Conflict Detection Tests
# =============================================================================

@test "install.sh detects conflict when local file exists" {
    create_fake_skill "test-skill"
    mkdir -p "$FAKE_HOME/.claude/skills"
    echo "local content" > "$FAKE_HOME/.claude/skills/test-skill"

    run run_install --dry-run
    [[ "$output" == *"Would backup and replace"* ]]
}

@test "install.sh --force overwrites conflicts with backup" {
    create_fake_skill "test-skill"
    mkdir -p "$FAKE_HOME/.claude/skills"
    echo "local content" > "$FAKE_HOME/.claude/skills/test-skill"

    run_install --force
    assert_symlink "$FAKE_HOME/.claude/skills/test-skill" "$FAKE_REPO/skills/test-skill"
    assert_backup_exists
}

@test "install.sh doesn't treat existing correct symlinks as conflicts" {
    create_fake_skill "test-skill"
    mkdir -p "$FAKE_HOME/.claude/skills"
    ln -s "$FAKE_REPO/skills/test-skill" "$FAKE_HOME/.claude/skills/test-skill"

    run run_install --dry-run
    [[ "$output" != *"Would backup"* ]]
}

# =============================================================================
# Backup Tests
# =============================================================================

@test "install.sh creates backup when overwriting" {
    create_fake_skill "test-skill"
    mkdir -p "$FAKE_HOME/.claude/skills"
    echo "local content" > "$FAKE_HOME/.claude/skills/test-skill"

    run_install --force
    assert_backup_exists
    assert_manifest_operation "install"
}

@test "install.sh backup contains original file" {
    create_fake_skill "test-skill"
    mkdir -p "$FAKE_HOME/.claude/skills"
    echo "local content" > "$FAKE_HOME/.claude/skills/test-skill"

    run_install --force

    local latest
    latest="$(get_latest_backup)"
    [[ -f "$FAKE_REPO/.backup/$latest/.claude/skills/test-skill" ]]
    grep -q "local content" "$FAKE_REPO/.backup/$latest/.claude/skills/test-skill"
}

@test "install.sh backup manifest has correct format" {
    create_fake_skill "test-skill"
    mkdir -p "$FAKE_HOME/.claude/skills"
    echo "local content" > "$FAKE_HOME/.claude/skills/test-skill"

    run_install --force

    local latest
    latest="$(get_latest_backup)"
    local manifest="$FAKE_REPO/.backup/$latest/manifest.json"

    # Check manifest structure
    grep -q '"timestamp":' "$manifest"
    grep -q '"operation": "install"' "$manifest"
    grep -q '"items":' "$manifest"
    grep -q '"user":' "$manifest"
}

# =============================================================================
# Help Tests
# =============================================================================

@test "install.sh --help shows usage" {
    run run_install --help
    [[ "$output" == *"Usage:"* ]]
    [[ "$output" == *"--dry-run"* ]]
    [[ "$output" == *"--force"* ]]
}

@test "install.sh -h shows usage" {
    run run_install -h
    [[ "$output" == *"Usage:"* ]]
}

# =============================================================================
# Idempotency Tests
# =============================================================================

@test "install.sh is idempotent (running twice works)" {
    create_fake_skill "test-skill"
    create_fake_agent "planner"

    run_install
    run_install

    assert_symlink "$FAKE_HOME/.claude/skills/test-skill" "$FAKE_REPO/skills/test-skill"
    assert_symlink "$FAKE_HOME/.claude/agents/planner.md" "$FAKE_REPO/agents/planner.md"
}

@test "install.sh preserves local-only items" {
    mkdir -p "$FAKE_HOME/.claude/skills/local-skill"
    echo "local content" > "$FAKE_HOME/.claude/skills/local-skill/SKILL.md"

    run_install

    # Local skill should still exist
    [[ -d "$FAKE_HOME/.claude/skills/local-skill" ]]
    [[ -f "$FAKE_HOME/.claude/skills/local-skill/SKILL.md" ]]
}

# =============================================================================
# Global rules, hooks, status line, and settings
# =============================================================================

@test "install.sh links rules, hooks, and the status line" {
    mkdir -p "$FAKE_REPO/home/rules" "$FAKE_REPO/home/hooks"
    echo "# rule" > "$FAKE_REPO/home/rules/pr-titles.md"
    printf '#!/bin/bash\nexit 0\n' > "$FAKE_REPO/home/hooks/guard-bash.sh"
    printf '#!/bin/bash\necho status\n' > "$FAKE_REPO/home/statusline.sh"

    run_install

    assert_symlink "$FAKE_HOME/.claude/rules/pr-titles.md" "$FAKE_REPO/home/rules/pr-titles.md"
    assert_symlink "$FAKE_HOME/.claude/hooks/guard-bash.sh" "$FAKE_REPO/home/hooks/guard-bash.sh"
    assert_symlink "$FAKE_HOME/.claude/statusline.sh" "$FAKE_REPO/home/statusline.sh"
}

@test "install.sh merges settings, keeps local keys, and unions permissions" {
    mkdir -p "$FAKE_REPO/home"
    cat > "$FAKE_REPO/home/settings.json" <<'JSON'
{"model": "opus", "permissions": {"deny": ["Read(**/.env)"]}, "statusLine": {"type": "command", "command": "x"}}
JSON
    cat > "$FAKE_HOME/.claude/settings.json" <<'JSON'
{"model": "sonnet", "theme": "dark", "permissions": {"allow": ["Bash(ls *)"], "deny": ["Bash(rm *)"]}}
JSON

    run run_install
    [[ "$status" -eq 0 ]]

    run jq -e '.model == "opus" and .theme == "dark" and .statusLine.command == "x"
        and .permissions.allow == ["Bash(ls *)"]
        and (.permissions.deny | sort) == ["Bash(rm *)", "Read(**/.env)"]' "$FAKE_HOME/.claude/settings.json"
    [[ "$status" -eq 0 ]]
    grep -q '"sonnet"' "$FAKE_REPO/.backup/$(get_latest_backup)/.claude/settings.json"
}

@test "install.sh leaves settings untouched when already merged" {
    mkdir -p "$FAKE_REPO/home"
    echo '{"model": "opus"}' > "$FAKE_REPO/home/settings.json"
    run_install
    run run_install
    [[ "$output" == *"settings.json (up to date)"* ]]
}

@test "install.sh refuses to overwrite invalid local settings" {
    mkdir -p "$FAKE_REPO/home"
    echo '{"model": "opus"}' > "$FAKE_REPO/home/settings.json"
    echo '{not json' > "$FAKE_HOME/.claude/settings.json"

    run run_install
    [[ "$output" == *"not valid JSON; left unchanged"* ]]
    grep -q '{not json' "$FAKE_HOME/.claude/settings.json"
}
