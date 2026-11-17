#!/usr/bin/env bats
# Tests for the PreToolUse guard hook and the status line script

setup() {
    PROJECT_DIR="$(cd "$BATS_TEST_DIRNAME/.." && pwd)"
    GUARD="$PROJECT_DIR/global/hooks/guard-bash.sh"
    TEST_TMP="$(cd "$(mktemp -d)" && pwd -P)"
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
        [[ "$output" == *"Blocked by the harness guard"* ]] || return 1
    done
}

@test "guard sees through wrappers, quoting, subshells, absolute paths, and split flags" {
    for command in \
        'bash -c "git push -f"' \
        "sh -c 'rm -rf /'" \
        '(git push -f)' \
        'echo $(git reset --hard)' \
        'echo `git reset --hard`' \
        '/usr/bin/git push -f' \
        '/bin/rm -rf /' \
        'git push -fu origin x' \
        'git push -uf origin x' \
        'git push origin +feature' \
        'git push origin HEAD:main' \
        'git clean --force' \
        'git commit -n -m x' \
        'git commit -nm x' \
        'git -c core.hooksPath=/dev/null commit -m x' \
        'rm -r -f .' \
        'rm -f -r /' \
        'rm --recursive /' \
        'rm -rf -- /' \
        'rm -rf "$HOME"' \
        'rm -rf "/"' \
        'rm -rf ${HOME}'; do
        run guard "$command"
        [[ "$status" -eq 2 ]] || { echo "not blocked: $command"; return 1; }
    done
}

@test "guard doesn't read commit messages or longer branch names as commands" {
    git -C "$TEST_TMP" init -q -b feature/x
    for command in \
        'git commit -m "docs: explain why rm -rf / is blocked"' \
        'git commit -m "note git push --force is blocked"' \
        'git push origin feature/main' \
        'git push -u origin my-main' \
        'git checkout -- main.ts' \
        'rm -r -f build'; do
        run guard "$command"
        [[ "$status" -eq 0 ]] || { echo "wrongly blocked: $command"; return 1; }
    done
}

@test "guard blocks reading dotenv files that hold secrets" {
    for command in \
        'cat .env' \
        'cat app/.env.staging' \
        'less .env.production' \
        'head -n 5 .env.local' \
        'grep DB_PASSWORD .env' \
        'source .env' \
        '. ./.env.testing' \
        'bash -c "cat .env"' \
        'cd api && cat .env | pbcopy' \
        'cat <.env' \
        'cat<.env' \
        'cat .env>/tmp/x' \
        'tee < .env' \
        'cat .env*' \
        'cat .e*' \
        'cat .ENV' \
        'cat .en\v' \
        '/bin/cat .env' \
        'sudo cat .env' \
        'sort .env' \
        'dd if=.env' \
        'diff <(cat .env) .env.example' \
        'sed -i s/a/b/ .env' \
        'grep -e KEY .env'; do
        run guard "$command"
        [[ "$status" -eq 2 ]] || { echo "not blocked: $command"; return 1; }
        [[ "$output" == *"would expose secrets"* ]] || return 1
    done
}

@test "guard allows dotenv templates and commands that don't print secrets" {
    for command in \
        'cat .env.example' \
        'cat .env.sample' \
        'cp .env.example .env' \
        'ls -la .env' \
        'grep -r DB_HOST config/' \
        'cat environment.ts' \
        'php artisan key:generate' \
        'gh pr create --title x --body "never cat .env"' \
        'echo "run cat .env to see it"' \
        'grep .env .gitignore' \
        'find . -name .env' \
        'cat <<EOF > .env
APP_KEY=
EOF' \
        'cat <<<"$value" > out.txt'; do
        run guard "$command"
        [[ "$status" -eq 0 ]] || { echo "wrongly blocked: $command"; return 1; }
    done
}

@test "guard ignores heredoc bodies, such as commit messages that mention dotenv files" {
    git -C "$TEST_TMP" init -q -b feature/x
    run guard "git commit -m \"\$(cat <<'EOF'
fix: stop reading .env files
cat .env is now blocked; so is rm -rf /
EOF
)\""
    [[ "$status" -eq 0 ]] || { echo "$output"; return 1; }
    run guard "cat <<-EOF
	cat .env
	EOF
cat .env"
    [[ "$status" -eq 2 ]] || { echo "a command after the heredoc must still be checked"; return 1; }
}

@test "guard blocks a commit after switching to a protected branch, and bare pushes from main" {
    git -C "$TEST_TMP" init -q -b feature/x
    for command in 'git checkout main && git commit -m x' 'git switch develop; git commit -m x'; do
        run guard "$command"
        [[ "$status" -eq 2 ]] || { echo "not blocked: $command"; return 1; }
    done
    run guard 'git checkout -b feature/y && git commit -m x'
    [[ "$status" -eq 0 ]] || return 1

    git -C "$TEST_TMP" switch -q -c main
    for command in 'git push' 'git push origin' 'git push origin HEAD'; do
        run guard "$command"
        [[ "$status" -eq 2 ]] || { echo "not blocked: $command"; return 1; }
    done
}

@test "guard follows cd \$HOME into a repository" {
    mkdir -p "$TEST_TMP/home/app"
    git -C "$TEST_TMP/home/app" init -q -b main
    HOME="$TEST_TMP/home" run guard 'cd $HOME/app && git commit -m x' /
    [[ "$status" -eq 2 ]] || return 1
    HOME="$TEST_TMP/home" run guard 'cd "$HOME/app" && git commit -m x' /
    [[ "$status" -eq 2 ]] || return 1
}

@test "guard fails closed on malformed input" {
    run bash -c "echo 'not json' | '$GUARD'"
    [[ "$status" -eq 2 ]] || return 1
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
    [[ "$status" -eq 2 ]] || return 1
    [[ "$output" == *"'main'"* ]] || return 1

    git -C "$TEST_TMP" switch -q -c feature/login
    run guard 'git commit -m x'
    [[ "$status" -eq 0 ]] || return 1
}

@test "guard ignores input without a command" {
    run bash -c "echo '{}' | '$GUARD'"
    [[ "$status" -eq 0 ]] || return 1
}

@test "status line shows model, directory, branch, context, and cost" {
    git -C "$TEST_TMP" init -q -b feature/demo
    run bash -c "jq -n --arg d '$TEST_TMP' '{model: {display_name: \"Opus\"}, workspace: {current_dir: \$d}, context_window: {used_percentage: 42.4}, cost: {total_cost_usd: 1.234}}' | HARNESS_MEMORY_SWAP_PCT=100 HARNESS_MEMORY_FREE_PCT=0 '$PROJECT_DIR/global/claude/statusline.sh'"
    [[ "$status" -eq 0 ]] || return 1
    [[ "$output" == "Opus · $(basename "$TEST_TMP") · feature/demo · ctx 42% · \$1.23" ]] || return 1
}

@test "versioned settings wire the guard hook, the stall watcher, and the status line" {
    run jq -e '.hooks.PreToolUse[0].matcher == "Bash"
        and (.hooks.PreToolUse[0].hooks[0].command | contains(".claude/hooks/guard-bash.sh"))
        and .hooks.PostToolUse[0].matcher == "Bash"
        and (.hooks.PostToolUse[0].hooks[0] | (.command | contains(".claude/hooks/stall-watch.sh")) and .asyncRewake == true and .timeout >= 3600)
        and (.statusLine.command | contains(".claude/statusline.sh"))
        and (.permissions.deny | index("Read(**/.env)"))' "$PROJECT_DIR/global/claude/settings.json"
    [[ "$status" -eq 0 ]] || return 1
    [[ -x "$GUARD" && -x "$PROJECT_DIR/global/claude/statusline.sh" ]] || return 1
}
