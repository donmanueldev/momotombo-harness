#!/usr/bin/env bats

load 'test_helper'

setup() {
    setup_test_env
}

teardown() {
    teardown_test_env
}

@test "Matt synchronization removes only legacy repository links and reconciles all skills" {
    mkdir -p "$FAKE_HOME/.codex/skills" "$TEST_TMP/bin"
    ln -s "$FAKE_REPO/skills/code-review" "$FAKE_HOME/.codex/skills/code-review"
    ln -s "/outside/keep-me" "$FAKE_HOME/.codex/skills/keep-me"
    cat > "$TEST_TMP/bin/npx" <<'EOF'
#!/usr/bin/env bash
printf '%s\n' "$*" >> "$NPX_LOG"
EOF
    chmod +x "$TEST_TMP/bin/npx"

    run env PATH="$TEST_TMP/bin:$PATH" NPX_LOG="$TEST_TMP/npx.log" HOME="$FAKE_HOME" bash ./scripts/sync-matt-skills.sh

    [[ "$status" -eq 0 ]]
    assert_absent "$FAKE_HOME/.codex/skills/code-review"
    [[ -L "$FAKE_HOME/.codex/skills/keep-me" ]]
    grep -q "skills@latest add mattpocock/skills --global --agent codex --skill \* --yes" "$TEST_TMP/npx.log"
}
