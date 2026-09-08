#!/usr/bin/env bats
# Tests for sync.sh

load 'test_helper'

setup() {
    setup_test_env
}

teardown() {
    teardown_test_env
}

# =============================================================================
# Status Display Tests
# =============================================================================

@test "sync.sh shows status by default" {
    run run_sync
    [[ "$status" -eq 0 ]]
    [[ "$output" == *"Codex Sync Status"* ]]
    [[ "$output" == *"Skills (~/.codex):"* ]]
    [[ "$output" == *"Subagents (~/.codex/agents):"* ]]
}

@test "sync.sh shows synced skills and subagents" {
    create_fake_skill "test-skill"
    create_fake_codex_agent "planner"
    run_install

    run run_sync
    [[ "$output" == *"test-skill"* ]]
    [[ "$output" == *"planner.toml"* ]]
    [[ "$output" == *"synced"* ]]
}

# =============================================================================
# Add Skill Tests
# =============================================================================

@test "sync.sh add skill copies to repo and creates symlink" {
    create_fake_skill "my-skill" "$FAKE_HOME/.codex/skills"

    run_sync add skill my-skill

    assert_dir "$FAKE_REPO/skills/my-skill"
    [[ -f "$FAKE_REPO/skills/my-skill/SKILL.md" ]]
    assert_symlink "$FAKE_HOME/.codex/skills/my-skill" "$FAKE_REPO/skills/my-skill"
}

@test "sync.sh add skill creates backup" {
    create_fake_skill "my-skill" "$FAKE_HOME/.codex/skills"

    run_sync add skill my-skill

    assert_backup_exists
    assert_manifest_operation "add-skill"
}

@test "sync.sh add skill --dry-run doesn't modify" {
    create_fake_skill "my-skill" "$FAKE_HOME/.codex/skills"

    run run_sync --dry-run add skill my-skill

    [[ ! -d "$FAKE_REPO/skills/my-skill" ]]
    assert_regular_file "$FAKE_HOME/.codex/skills/my-skill/SKILL.md"
}

@test "sync.sh rejects traversal in an item name" {
    run run_sync --dry-run delete skill ../outside

    [[ "$status" -ne 0 ]]
    [[ "$output" == *"Invalid item name"* ]]
}

@test "sync.sh add skill fails if already synced" {
    create_fake_skill "my-skill"
    run_install

    run run_sync add skill my-skill
    [[ "$status" -ne 0 ]]
    [[ "$output" == *"already synced"* ]]
}

# =============================================================================
# Add Subagent Tests
# =============================================================================

@test "sync.sh add agent copies to repo and creates symlink" {
    create_fake_codex_agent "planner" "$FAKE_HOME/.codex/agents"

    run_sync add agent planner

    assert_regular_file "$FAKE_REPO/.codex/agents/planner.toml"
    assert_symlink "$FAKE_HOME/.codex/agents/planner.toml" "$FAKE_REPO/.codex/agents/planner.toml"
}

# =============================================================================
# Remove Tests
# =============================================================================

@test "sync.sh remove skill removes repo but keeps local" {
    create_fake_skill "my-skill"
    run_install

    run_sync remove skill my-skill

    [[ ! -d "$FAKE_REPO/skills/my-skill" ]]
    assert_dir "$FAKE_HOME/.codex/skills/my-skill"
    [[ ! -L "$FAKE_HOME/.codex/skills/my-skill" ]]
}

@test "sync.sh remove agent removes repo but keeps local" {
    create_fake_codex_agent "planner"
    run_install

    run_sync remove agent planner

    [[ ! -f "$FAKE_REPO/.codex/agents/planner.toml" ]]
    assert_regular_file "$FAKE_HOME/.codex/agents/planner.toml"
    [[ ! -L "$FAKE_HOME/.codex/agents/planner.toml" ]]
}

@test "sync.sh delete skill removes repo and local copies" {
    create_fake_skill "my-skill"
    run_install

    run_sync delete skill my-skill

    assert_absent "$FAKE_REPO/skills/my-skill"
    assert_absent "$FAKE_HOME/.codex/skills/my-skill"
    assert_backup_exists
    assert_manifest_operation "delete-skill"
}

# =============================================================================
# Backups and Undo
# =============================================================================

@test "sync.sh backups lists existing backups" {
    create_fake_skill "my-skill" "$FAKE_HOME/.codex/skills"
    run_sync add skill my-skill

    run run_sync backups
    [[ "$output" == *"Available Backups"* ]]
    [[ "$output" == *"add-skill"* ]]
}

@test "sync.sh undo restores original files" {
    create_fake_skill "my-skill"
    run_install

    run_sync remove skill my-skill

    run bash -c 'printf "y\n" | HOME="'"$FAKE_HOME"'" bash "'"$FAKE_REPO"'/sync.sh" undo'
    [[ "$status" -eq 0 ]]
    assert_symlink "$FAKE_HOME/.codex/skills/my-skill" "$FAKE_REPO/skills/my-skill"
    assert_dir "$FAKE_REPO/skills/my-skill"
}
