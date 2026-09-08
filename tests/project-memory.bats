#!/usr/bin/env bats

load 'test_helper'

setup() {
    setup_test_env
    cp "$PROJECT_DIR/scripts/project-memory.sh" "$FAKE_REPO/scripts/"
    chmod +x "$FAKE_REPO/scripts/project-memory.sh"
}

teardown() { teardown_test_env; }

@test "project memory generator creates a path-only project map" {
    touch "$FAKE_REPO/package.json" "$FAKE_REPO/.env"
    mkdir -p "$FAKE_REPO/src/auth" "$FAKE_REPO/node_modules/ignored"

    run bash scripts/project-memory.sh initialize

    [[ "$status" -eq 0 ]]
    [[ -f "$FAKE_REPO/docs/project-map.md" ]]
    grep -q 'Node.js' "$FAKE_REPO/docs/project-map.md"
    grep -q '`src/auth`' "$FAKE_REPO/docs/project-map.md"
    ! grep -q '.env' "$FAKE_REPO/docs/project-map.md"
    ! grep -q 'node_modules' "$FAKE_REPO/docs/project-map.md"
}

@test "project memory generator refuses an accidental overwrite and supports dry runs" {
    mkdir -p "$FAKE_REPO/docs"
    touch "$FAKE_REPO/docs/project-map.md"

    run bash scripts/project-memory.sh initialize
    [[ "$status" -ne 0 ]]
    [[ "$output" == *"already exists"* ]]

    run bash scripts/project-memory.sh refresh --dry-run
    [[ "$status" -eq 0 ]]
    [[ "$output" == *"Would write"* ]]
}
