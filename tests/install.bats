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

@test "install.sh creates ~/.codex directory" {
    create_fake_skill "test-skill"
    run_install
    assert_dir "$FAKE_HOME/.codex"
}

@test "install.sh symlinks skills" {
    create_fake_skill "test-skill"
    run_install
    assert_symlink "$FAKE_HOME/.codex/skills/test-skill" "$FAKE_REPO/skills/test-skill"
}

@test "install.sh symlinks multiple skills" {
    create_fake_skill "skill-one"
    create_fake_skill "skill-two"
    create_fake_skill "skill-three"
    run_install
    assert_symlink "$FAKE_HOME/.codex/skills/skill-one" "$FAKE_REPO/skills/skill-one"
    assert_symlink "$FAKE_HOME/.codex/skills/skill-two" "$FAKE_REPO/skills/skill-two"
    assert_symlink "$FAKE_HOME/.codex/skills/skill-three" "$FAKE_REPO/skills/skill-three"
}

@test "install.sh symlinks codex agents" {
    create_fake_codex_agent "planner"
    run_install
    assert_symlink "$FAKE_HOME/.codex/agents/planner.toml" "$FAKE_REPO/.codex/agents/planner.toml"
}

@test "install.sh symlinks command-policy rules" {
    mkdir -p "$FAKE_REPO/rules"
    cat > "$FAKE_REPO/rules/agent-config.rules" <<'EOF'
prefix_rule(pattern=["git", "status"], decision="allow")
EOF

    run_install

    assert_symlink "$FAKE_HOME/.codex/rules/agent-config.rules" "$FAKE_REPO/rules/agent-config.rules"
}

@test "install.sh synchronizes Matt skills only when explicitly requested" {
    mkdir -p "$TEST_TMP/bin"
    cat > "$TEST_TMP/bin/npx" <<'EOF'
#!/usr/bin/env bash
printf '%s\n' "$*" >> "$NPX_LOG"
EOF
    chmod +x "$TEST_TMP/bin/npx"

    run env PATH="$TEST_TMP/bin:$PATH" NPX_LOG="$TEST_TMP/npx.log" HOME="$FAKE_HOME" bash ./install.sh
    [[ "$status" -eq 0 ]]
    [[ ! -e "$TEST_TMP/npx.log" ]]

    run env PATH="$TEST_TMP/bin:$PATH" NPX_LOG="$TEST_TMP/npx.log" HOME="$FAKE_HOME" bash ./install.sh --with-matt
    [[ "$status" -eq 0 ]]
    grep -q "skills@latest add mattpocock/skills --global --agent codex --skill \* --yes" "$TEST_TMP/npx.log"
}

# =============================================================================
# Dry Run Tests
# =============================================================================

@test "install.sh --dry-run doesn't create symlinks" {
    create_fake_skill "test-skill"
    create_fake_codex_agent "planner"
    run_install --dry-run
    [[ ! -L "$FAKE_HOME/.codex/skills/test-skill" ]]
    [[ ! -L "$FAKE_HOME/.codex/agents/planner.toml" ]]
    [[ ! -d "$FAKE_HOME/.codex/agents" ]]
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
    mkdir -p "$FAKE_HOME/.codex/skills"
    echo "local content" > "$FAKE_HOME/.codex/skills/test-skill"

    run run_install --dry-run
    [[ "$output" == *"Would backup and replace"* ]]
}

@test "install.sh --force overwrites conflicts with backup" {
    create_fake_skill "test-skill"
    mkdir -p "$FAKE_HOME/.codex/skills"
    echo "local content" > "$FAKE_HOME/.codex/skills/test-skill"

    run_install --force
    assert_symlink "$FAKE_HOME/.codex/skills/test-skill" "$FAKE_REPO/skills/test-skill"
    assert_backup_exists
}

@test "install.sh doesn't treat existing correct symlinks as conflicts" {
    create_fake_skill "test-skill"
    mkdir -p "$FAKE_HOME/.codex/skills"
    ln -s "$FAKE_REPO/skills/test-skill" "$FAKE_HOME/.codex/skills/test-skill"

    run run_install --dry-run
    [[ "$output" != *"Would backup"* ]]
}

# =============================================================================
# Backup Tests
# =============================================================================

@test "install.sh creates backup when overwriting" {
    create_fake_skill "test-skill"
    mkdir -p "$FAKE_HOME/.codex/skills"
    echo "local content" > "$FAKE_HOME/.codex/skills/test-skill"

    run_install --force
    assert_backup_exists
    assert_manifest_operation "install"
}

@test "install.sh backup contains original file" {
    create_fake_skill "test-skill"
    mkdir -p "$FAKE_HOME/.codex/skills"
    echo "local content" > "$FAKE_HOME/.codex/skills/test-skill"

    run_install --force

    local latest
    latest="$(get_latest_backup)"
    [[ -f "$FAKE_REPO/.backup/$latest/.codex/skills/test-skill" ]]
    grep -q "local content" "$FAKE_REPO/.backup/$latest/.codex/skills/test-skill"
}

@test "install.sh backup manifest has correct format" {
    create_fake_skill "test-skill"
    mkdir -p "$FAKE_HOME/.codex/skills"
    echo "local content" > "$FAKE_HOME/.codex/skills/test-skill"

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
    create_fake_codex_agent "planner"

    run_install
    run_install

    assert_symlink "$FAKE_HOME/.codex/skills/test-skill" "$FAKE_REPO/skills/test-skill"
    assert_symlink "$FAKE_HOME/.codex/agents/planner.toml" "$FAKE_REPO/.codex/agents/planner.toml"
}

@test "install.sh preserves local-only items" {
    mkdir -p "$FAKE_HOME/.codex/skills/local-skill"
    echo "local content" > "$FAKE_HOME/.codex/skills/local-skill/SKILL.md"

    run_install

    # Local skill should still exist
    [[ -d "$FAKE_HOME/.codex/skills/local-skill" ]]
    [[ -f "$FAKE_HOME/.codex/skills/local-skill/SKILL.md" ]]
}
