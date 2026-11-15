#!/usr/bin/env bats
# Codex-specific checks: the .rules command policy and the guard hook with Codex's payload shape.

setup() {
    PROJECT_DIR="$(cd "$BATS_TEST_DIRNAME/.." && pwd)"
    RULES="$PROJECT_DIR/global/codex/rules/momotombo.rules"
}

decision() {
    codex execpolicy check --rules "$RULES" -- "$@" < /dev/null 2>/dev/null | jq -r '.decision // "allowed"'
}

@test "codex rules forbid history rewrites and pushes to main" {
    command -v codex >/dev/null || skip "codex CLI not installed"
    [[ "$(decision git push --force)" == forbidden ]] || return 1
    [[ "$(decision git push -f origin x)" == forbidden ]] || return 1
    [[ "$(decision git push origin main)" == forbidden ]] || return 1
    [[ "$(decision git reset --hard HEAD)" == forbidden ]] || return 1
    [[ "$(decision git clean -fd)" == forbidden ]] || return 1
    [[ "$(decision git commit --no-verify -m x)" == forbidden ]] || return 1
}

@test "codex rules allow everyday git" {
    command -v codex >/dev/null || skip "codex CLI not installed"
    [[ "$(decision git push -u origin feature/x)" == allowed ]] || return 1
    [[ "$(decision git push origin main-fixes)" == allowed ]] || return 1
    [[ "$(decision git reset --soft HEAD~1)" == allowed ]] || return 1
    [[ "$(decision git clean -n)" == allowed ]] || return 1
}

@test "guard blocks with Codex's PreToolUse payload (no cwd)" {
    run bash -c "jq -n '{hook_event_name: \"PreToolUse\", tool_name: \"Bash\", tool_input: {command: \"git push --force\"}}' | '$PROJECT_DIR/global/hooks/guard-bash.sh'"
    [[ "$status" -eq 2 ]] || return 1
    [[ "$output" == *"force-push"* ]] || return 1
}

@test "codex hooks.json runs the shared guard on Bash" {
    run jq -e '.hooks.PreToolUse[0].matcher == "^Bash$"
        and (.hooks.PreToolUse[0].hooks[0].command | contains(".codex/hooks/guard-bash.sh"))' "$PROJECT_DIR/global/codex/hooks.json"
    [[ "$status" -eq 0 ]] || return 1
}
