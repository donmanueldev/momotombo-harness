#!/usr/bin/env bats
# Tests for skills validation

load 'test_helper'

setup() {
    setup_test_env
}

teardown() {
    teardown_test_env
}

# =============================================================================
# Validate Command Tests
# =============================================================================

@test "sync.sh validate shows 'no skills' when empty" {
    run run_sync validate
    [[ "$status" -eq 0 ]]
    [[ "$output" == *"No skills or agents found to validate."* ]]
}

@test "sync.sh validate passes for valid skills" {
    create_fake_skill "valid-skill"

    run run_sync validate
    [[ "$status" -eq 0 ]]
    [[ "$output" == *"valid-skill"* ]]
    [[ "$output" == *"All"*"valid"* ]]
}

@test "sync.sh validate fails for skill without SKILL.md" {
    create_skill_no_md "bad-skill"

    run run_sync validate
    [[ "$status" -ne 0 ]]
    [[ "$output" == *"bad-skill"* ]]
    [[ "$output" == *"Missing SKILL.md"* ]]
}

@test "sync.sh validate fails for skill without frontmatter" {
    create_invalid_skill "bad-skill"

    run run_sync validate
    [[ "$status" -ne 0 ]]
    [[ "$output" == *"bad-skill"* ]]
    [[ "$output" == *"Missing frontmatter"* ]]
}

@test "sync.sh validate fails for skill missing name" {
    mkdir -p "$FAKE_REPO/skills/bad-skill"
    cat > "$FAKE_REPO/skills/bad-skill/SKILL.md" << EOF
---
description: Has description but no name
---

# Bad Skill
EOF

    run run_sync validate
    [[ "$status" -ne 0 ]]
    [[ "$output" == *"Missing 'name'"* ]]
}

@test "sync.sh validate fails for skill missing description" {
    mkdir -p "$FAKE_REPO/skills/bad-skill"
    cat > "$FAKE_REPO/skills/bad-skill/SKILL.md" << EOF
---
name: bad-skill
---

# Bad Skill
EOF

    run run_sync validate
    [[ "$status" -ne 0 ]]
    [[ "$output" == *"Missing 'description'"* ]]
}

@test "sync.sh validate checks repo and local skills" {
    create_fake_skill "repo-skill"
    create_fake_skill "local-skill" "$FAKE_HOME/.claude/skills"

    run run_sync validate
    [[ "$status" -eq 0 ]]
    [[ "$output" == *"repo-skill"* ]]
    [[ "$output" == *"local-skill"* ]]
    [[ "$output" == *"(local)"* ]]
}

@test "sync.sh validate doesn't double-count synced skills" {
    create_fake_skill "my-skill"
    run_install

    run run_sync validate

    local count
    count=$(echo "$output" | grep "my-skill" | grep -v "(local)" | wc -l | tr -d ' ')
    [[ "$count" -eq 1 ]]
}

@test "sync.sh validate checks repo and local agents" {
    mkdir -p "$FAKE_REPO/agents"
    mkdir -p "$FAKE_HOME/.claude/agents"
    create_fake_agent "planner-agent" "$FAKE_REPO/agents"
    create_fake_agent "local-agent" "$FAKE_HOME/.claude/agents"

    run run_sync validate
    [[ "$status" -eq 0 ]]
    [[ "$output" == *"planner-agent"* ]]
    [[ "$output" == *"local-agent.md (local)"* ]]
}

@test "sync.sh validate fails for agent missing description" {
    mkdir -p "$FAKE_REPO/agents"
    cat > "$FAKE_REPO/agents/bad-agent.md" << EOF
---
name: bad-agent
---
Bad agent without description.
EOF

    run run_sync validate
    [[ "$status" -ne 0 ]]
    [[ "$output" == *"bad-agent"* ]]
    [[ "$output" == *"Missing 'description'"* ]]
}

@test "sync.sh validate rejects an unknown agent model" {
    mkdir -p "$FAKE_REPO/agents"
    cat > "$FAKE_REPO/agents/odd-agent.md" << EOF
---
name: odd-agent
description: Agent with an unknown model
model: gpt-4
---
Body.
EOF

    run run_sync validate
    [[ "$status" -ne 0 ]]
    [[ "$output" == *"Invalid 'model'"* ]]
}

@test "repository agents declare the routed models" {
    grep -q '^model: haiku$' "$PROJECT_DIR/agents/git-ops.md"
    grep -q '^model: sonnet$' "$PROJECT_DIR/agents/analyst.md"
    grep -q '^model: sonnet$' "$PROJECT_DIR/agents/reviewer.md"
    grep -q '^model: opus$' "$PROJECT_DIR/agents/planner.md"
    grep -q '^model: opus$' "$PROJECT_DIR/agents/implementer.md"
    grep -q '^model: sonnet$' "$PROJECT_DIR/agents/ux-reviewer.md"
    grep -q '^model: opus$' "$PROJECT_DIR/agents/security-reviewer.md"
}

@test "git skills fork into the git-ops agent on haiku" {
    for skill in commit open-pr; do
        grep -q '^context: fork$' "$PROJECT_DIR/skills/$skill/SKILL.md"
        grep -q '^agent: git-ops$' "$PROJECT_DIR/skills/$skill/SKILL.md"
        grep -q '^model: haiku$' "$PROJECT_DIR/skills/$skill/SKILL.md"
    done
}

# =============================================================================
# Validation on Add Tests
# =============================================================================

@test "sync.sh add skill validates before adding" {
    create_invalid_skill "bad-skill" "$FAKE_HOME/.claude/skills"

    run bash -c 'printf "n\n" | HOME="'"$FAKE_HOME"'" bash "'"$FAKE_REPO"'/sync.sh" --dry-run add skill bad-skill'
    [[ "$output" == *"Missing frontmatter"* ]]
}

@test "sync.sh add skill with valid skill doesn't show warnings" {
    create_fake_skill "good-skill" "$FAKE_HOME/.claude/skills"

    run run_sync --dry-run add skill good-skill
    [[ "$status" -eq 0 ]]
    [[ "$output" != *"Missing"* ]]
    [[ "$output" == *"[dry-run]"* ]]
}
