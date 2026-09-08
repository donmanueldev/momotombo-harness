#!/usr/bin/env bats

load 'test_helper'

setup() {
    setup_test_env
    mkdir -p "$TEST_TMP/bin"
    cat > "$TEST_TMP/bin/npx" <<'EOF'
#!/usr/bin/env bash
printf '%s\n' "$*" >> "$NPX_LOG"
EOF
    chmod +x "$TEST_TMP/bin/npx"
}

teardown() {
    teardown_test_env
}

@test "Android skill synchronization uses the complete official source" {
    run env PATH="$TEST_TMP/bin:$PATH" NPX_LOG="$TEST_TMP/npx.log" HOME="$FAKE_HOME" bash ./scripts/sync-android-skills.sh

    [[ "$status" -eq 0 ]]
    grep -q "skills@latest add android/skills --global --agent codex --skill \* --yes" "$TEST_TMP/npx.log"
}

@test "Kotlin skill synchronization uses the complete official source" {
    run env PATH="$TEST_TMP/bin:$PATH" NPX_LOG="$TEST_TMP/npx.log" HOME="$FAKE_HOME" bash ./scripts/sync-kotlin-skills.sh

    [[ "$status" -eq 0 ]]
    grep -q "skills@latest add Kotlin/kotlin-agent-skills --global --agent codex --skill \* --yes" "$TEST_TMP/npx.log"
}

@test "mobile sync reconciles both official sources" {
    run env PATH="$TEST_TMP/bin:$PATH" NPX_LOG="$TEST_TMP/npx.log" HOME="$FAKE_HOME" bash ./sync.sh mobile

    [[ "$status" -eq 0 ]]
    grep -q "skills@latest add android/skills --global --agent codex --skill \* --yes" "$TEST_TMP/npx.log"
    grep -q "skills@latest add Kotlin/kotlin-agent-skills --global --agent codex --skill \* --yes" "$TEST_TMP/npx.log"
}

@test "mobile sync dry run does not call the external CLI" {
    run env PATH="$TEST_TMP/bin:$PATH" NPX_LOG="$TEST_TMP/npx.log" HOME="$FAKE_HOME" bash ./sync.sh --dry-run mobile

    [[ "$status" -eq 0 ]]
    [[ "$output" == *"official Android skills"* ]]
    [[ "$output" == *"official Kotlin skills"* ]]
    [[ ! -e "$TEST_TMP/npx.log" ]]
}

@test "install synchronizes mobile skills only when explicitly requested" {
    run env PATH="$TEST_TMP/bin:$PATH" NPX_LOG="$TEST_TMP/npx.log" HOME="$FAKE_HOME" bash ./install.sh
    [[ "$status" -eq 0 ]]
    [[ ! -e "$TEST_TMP/npx.log" ]]

    run env PATH="$TEST_TMP/bin:$PATH" NPX_LOG="$TEST_TMP/npx.log" HOME="$FAKE_HOME" bash ./install.sh --with-mobile
    [[ "$status" -eq 0 ]]
    grep -q "skills@latest add android/skills --global --agent codex --skill \* --yes" "$TEST_TMP/npx.log"
    grep -q "skills@latest add Kotlin/kotlin-agent-skills --global --agent codex --skill \* --yes" "$TEST_TMP/npx.log"
}
