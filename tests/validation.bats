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

@test "orchestration contract document exists" {
    [[ -f "$PROJECT_DIR/docs/orchestration-output-contract.md" ]]
    grep -q "Orchestration Output Contract" "$PROJECT_DIR/docs/orchestration-output-contract.md"
}

@test "orchestration log helper writes a canonical JSON entry" {
    mkdir -p "$FAKE_REPO/.codex" "$FAKE_REPO/scripts"
    cp "$PROJECT_DIR/scripts/log-orchestration-event.sh" "$FAKE_REPO/scripts/log-orchestration-event.sh"

    run bash "$FAKE_REPO/scripts/log-orchestration-event.sh" \
        --agent orchestrator \
        --goal "test goal" \
        --status ok \
        --decision "router-ready" \
        --result "logged" \
        --next-step "continue"

    [[ "$status" -eq 0 ]]
    [[ -f "$FAKE_REPO/.codex/orchestration.jsonl" ]]
    run python3 -c 'import json, sys; entry = json.load(open(sys.argv[1])); assert entry["agent"] == "orchestrator"; assert entry["next_step"] == "continue"' "$FAKE_REPO/.codex/orchestration.jsonl"
    [[ "$status" -eq 0 ]]
}

@test "orchestration validator rejects non-JSON output" {
    mkdir -p "$FAKE_REPO/scripts"
    cp "$PROJECT_DIR/scripts/validate-orchestration-output.sh" "$FAKE_REPO/scripts/validate-orchestration-output.sh"
    printf 'agent: planner\n' > "$FAKE_REPO/route.yaml"

    run bash "$FAKE_REPO/scripts/validate-orchestration-output.sh" --agent planner --input "$FAKE_REPO/route.yaml"
    [[ "$status" -ne 0 ]]
    [[ "$output" == *"input must be valid JSON"* ]]
}

@test "orchestration validator accepts a complete orchestrator payload" {
    mkdir -p "$FAKE_REPO/scripts"
    cp "$PROJECT_DIR/scripts/validate-orchestration-output.sh" "$FAKE_REPO/scripts/validate-orchestration-output.sh"
    cat > "$FAKE_REPO/route.json" <<'JSON'
{"agent":"orchestrator","goal":"route work","status":"ok","decision":"continue","evidence":[],"result":"route created","specification_id":"SPEC-42","specification_revision":1,"route":"plan","agent_order":["planner"],"risk_register":[],"verification_plan":["bats tests/"],"requirement_assignments":[{"requirement_id":"REQ-1","owner":"backend","scope":"validator","verification":"bats tests/"}]}
JSON

    run bash "$FAKE_REPO/scripts/validate-orchestration-output.sh" --agent orchestrator --input "$FAKE_REPO/route.json"
    [[ "$status" -eq 0 ]]
    [[ "$output" == *"valid"* ]]
}

@test "orchestration validator accepts a traceable specification payload" {
    mkdir -p "$FAKE_REPO/scripts"
    cp "$PROJECT_DIR/scripts/validate-orchestration-output.sh" "$FAKE_REPO/scripts/validate-orchestration-output.sh"
    cat > "$FAKE_REPO/spec.json" <<'JSON'
{"agent":"specifier","goal":"define login change","status":"ok","decision":"request approval","evidence":["ticket-42"],"result":"draft specification","approval_status":"draft","specification_id":"SPEC-42","specification_revision":1,"scope_in":["email login"],"scope_out":["social login"],"requirements":[{"id":"REQ-1","description":"Users can sign in with an email and password.","acceptance_criteria":["Valid credentials create a session."]}],"non_functional_requirements":["Do not log credentials."],"contracts":[],"assumptions":[],"open_questions":[],"verification_matrix":[{"requirement_id":"REQ-1","method":"integration test","evidence":"auth integration test"}],"risks":[]}
JSON

    run bash "$FAKE_REPO/scripts/validate-orchestration-output.sh" --agent specifier --input "$FAKE_REPO/spec.json"
    [[ "$status" -eq 0 ]]
    [[ "$output" == *"valid"* ]]
}

@test "orchestration validator requires approval evidence for approved specifications" {
    mkdir -p "$FAKE_REPO/scripts"
    cp "$PROJECT_DIR/scripts/validate-orchestration-output.sh" "$FAKE_REPO/scripts/validate-orchestration-output.sh"
    cat > "$FAKE_REPO/spec.json" <<'JSON'
{"agent":"specifier","goal":"define login change","status":"ok","decision":"approved","evidence":["ticket-42"],"result":"approved specification","approval_status":"approved","specification_id":"SPEC-42","specification_revision":1,"scope_in":["email login"],"scope_out":[],"requirements":[{"id":"REQ-1","description":"Users can sign in with an email and password.","acceptance_criteria":["Valid credentials create a session."]}],"non_functional_requirements":[],"contracts":[],"assumptions":[],"open_questions":[],"verification_matrix":[{"requirement_id":"REQ-1","method":"integration test","evidence":"auth integration test"}],"risks":[]}
JSON

    run bash "$FAKE_REPO/scripts/validate-orchestration-output.sh" --agent specifier --input "$FAKE_REPO/spec.json"
    [[ "$status" -ne 0 ]]
    [[ "$output" == *"approval_evidence must be a non-empty string"* ]]
}

@test "orchestration validator accepts traceable planner and reviewer payloads" {
    mkdir -p "$FAKE_REPO/scripts"
    cp "$PROJECT_DIR/scripts/validate-orchestration-output.sh" "$FAKE_REPO/scripts/validate-orchestration-output.sh"
    cat > "$FAKE_REPO/planner.json" <<'JSON'
{"agent":"planner","goal":"plan login change","status":"ok","decision":"route implementation","evidence":["SPEC-42"],"result":"plan created","specification_id":"SPEC-42","specification_revision":1,"context":"login","constraints":[],"done_criteria":["REQ-1 verified"],"execution_plan":["backend"],"risks":[],"verification":["integration test"],"requirement_plan":[{"requirement_id":"REQ-1","owner":"backend","work":"implement login","verification":"integration test"}]}
JSON
    cat > "$FAKE_REPO/reviewer.json" <<'JSON'
{"agent":"reviewer","goal":"review login change","status":"ok","decision":"ready","evidence":["integration test"],"result":"review complete","specification_id":"SPEC-42","specification_revision":1,"findings":[],"blocking_items":[],"requirement_coverage":[{"requirement_id":"REQ-1","status":"verified","evidence":"integration test"}]}
JSON

    run bash "$FAKE_REPO/scripts/validate-orchestration-output.sh" --agent planner --input "$FAKE_REPO/planner.json"
    [[ "$status" -eq 0 ]]
    [[ "$output" == *"valid"* ]]

    run bash "$FAKE_REPO/scripts/validate-orchestration-output.sh" --agent reviewer --input "$FAKE_REPO/reviewer.json"
    [[ "$status" -eq 0 ]]
    [[ "$output" == *"valid"* ]]
}

@test "sync.sh validate checks repo and local skills" {
    create_fake_skill "repo-skill"
    create_fake_skill "local-skill" "$FAKE_HOME/.codex/skills"

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
    mkdir -p "$FAKE_REPO/.codex/agents"
    mkdir -p "$FAKE_HOME/.codex/agents"
    create_fake_codex_agent "planner-agent" "$FAKE_REPO/.codex/agents"
    create_fake_codex_agent "local-agent" "$FAKE_HOME/.codex/agents"

    run run_sync validate
    [[ "$status" -eq 0 ]]
    [[ "$output" == *"planner-agent"* ]]
    [[ "$output" == *"local-agent.toml (local)"* ]]
}

@test "sync.sh validate fails for agent missing description" {
    mkdir -p "$FAKE_REPO/.codex/agents"
    cat > "$FAKE_REPO/.codex/agents/bad-agent.toml" << EOF
name = "bad-agent"
developer_instructions = "Bad agent without description."
EOF

    run run_sync validate
    [[ "$status" -ne 0 ]]
    [[ "$output" == *"bad-agent"* ]]
    [[ "$output" == *"Missing 'description'"* ]]
}

# =============================================================================
# Validation on Add Tests
# =============================================================================

@test "sync.sh add skill validates before adding" {
    create_invalid_skill "bad-skill" "$FAKE_HOME/.codex/skills"

    run bash -c 'printf "n\n" | HOME="'"$FAKE_HOME"'" bash "'"$FAKE_REPO"'/sync.sh" --dry-run add skill bad-skill'
    [[ "$output" == *"Missing frontmatter"* ]]
}

@test "sync.sh add skill with valid skill doesn't show warnings" {
    create_fake_skill "good-skill" "$FAKE_HOME/.codex/skills"

    run run_sync --dry-run add skill good-skill
    [[ "$status" -eq 0 ]]
    [[ "$output" != *"Missing"* ]]
    [[ "$output" == *"[dry-run]"* ]]
}
