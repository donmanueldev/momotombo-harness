#!/usr/bin/env bats
# Tests for the PreToolUse guard hook and the status line script

setup() {
    PROJECT_DIR="$(cd "$BATS_TEST_DIRNAME/.." && pwd)"
    GUARD="$PROJECT_DIR/home/hooks/guard-bash.sh"
    TEST_TMP="$(mktemp -d)"
}

teardown() {
    rm -rf "$TEST_TMP"
}

guard() {
    jq -n --arg c "$1" --arg d "${2:-$TEST_TMP}" '{tool_input: {command: $c}, cwd: $d}' | "$GUARD"
}

@test "guard blocks history rewrites and safeguard bypasses" {
    for command in \
        'git push --force' \
        'git push origin feature/x -f' \
        'git push --force-with-lease origin x' \
        'cd app && git push origin main' \
        'git commit --no-verify -m x' \
        'git reset --hard HEAD~1' \
        'git clean -fd' \
        'rm -rf /' \
        'rm -rf ~' \
        'rm -fr $HOME/' \
        'rm -rf .'; do
        run guard "$command"
        [[ "$status" -eq 2 ]] || { echo "not blocked: $command"; return 1; }
        [[ "$output" == *"Blocked by agent-config guard"* ]]
    done
}

@test "guard allows everyday commands" {
    for command in \
        'git push -u origin bugfix/x' \
        'git push origin main-fixes' \
        'rm -rf node_modules' \
        'rm -rf ./dist' \
        'git reset --soft HEAD~1' \
        'git log --grep force' \
        'git clean -n' \
        'ls -la'; do
        run guard "$command"
        [[ "$status" -eq 0 ]] || { echo "wrongly blocked: $command"; return 1; }
    done
}

@test "guard blocks commits on protected branches only" {
    git -C "$TEST_TMP" init -q -b main
    run guard 'git commit -m x'
    [[ "$status" -eq 2 ]]
    [[ "$output" == *"'main'"* ]]

    git -C "$TEST_TMP" switch -q -c feature/login
    run guard 'git commit -m x'
    [[ "$status" -eq 0 ]]
}

@test "guard ignores input without a command" {
    run bash -c "echo '{}' | '$GUARD'"
    [[ "$status" -eq 0 ]]
}

@test "status line shows model, directory, branch, context, and cost" {
    git -C "$TEST_TMP" init -q -b feature/demo
    run bash -c "jq -n --arg d '$TEST_TMP' '{model: {display_name: \"Opus\"}, workspace: {current_dir: \$d}, context_window: {used_percentage: 42.4}, cost: {total_cost_usd: 1.234}}' | '$PROJECT_DIR/home/statusline.sh'"
    [[ "$status" -eq 0 ]]
    [[ "$output" == "Opus · $(basename "$TEST_TMP") · feature/demo · ctx 42% · \$1.23" ]]
}

@test "versioned settings wire the guard hook and status line" {
    run jq -e '.hooks.PreToolUse[0].matcher == "Bash"
        and (.hooks.PreToolUse[0].hooks[0].command | contains(".claude/hooks/guard-bash.sh"))
        and (.statusLine.command | contains(".claude/statusline.sh"))
        and (.permissions.deny | index("Read(**/.env)"))' "$PROJECT_DIR/home/settings.json"
    [[ "$status" -eq 0 ]]
    [[ -x "$GUARD" && -x "$PROJECT_DIR/home/statusline.sh" ]]
}
