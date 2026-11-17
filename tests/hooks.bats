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
    run bash -c "jq -n --arg d '$TEST_TMP' '{model: {display_name: \"Opus\"}, workspace: {current_dir: \$d}, context_window: {used_percentage: 42.4}, cost: {total_cost_usd: 1.234}}' | '$PROJECT_DIR/global/claude/statusline.sh'"
    [[ "$status" -eq 0 ]] || return 1
    [[ "$output" == "Opus · $(basename "$TEST_TMP") · feature/demo · ctx 42% · \$1.23" ]] || return 1
}

@test "versioned settings wire the guard hook, the stall watcher, the comment guard, and the status line" {
    run jq -e '.hooks.PreToolUse[0].matcher == "Bash"
        and (.hooks.PreToolUse[0].hooks[0].command | contains(".claude/hooks/guard-bash.sh"))
        and (.hooks.PostToolUse | map(select(.matcher == "Bash"))[0].hooks[0] | (.command | contains(".claude/hooks/stall-watch.sh")) and .asyncRewake == true and .timeout >= 3600)
        and (.hooks.PostToolUse | map(select(.matcher == "Edit|Write|MultiEdit"))[0].hooks[0].command | contains(".claude/hooks/comment-guard.sh"))
        and (.hooks.Stop[0].hooks[0] | (.command | contains(".claude/hooks/loop-stop.sh")) and .timeout >= 3600)
        and (.statusLine.command | contains(".claude/statusline.sh"))
        and (.permissions.deny | index("Read(**/.env)"))' "$PROJECT_DIR/global/claude/settings.json"
    [[ "$status" -eq 0 ]] || return 1
    [[ -x "$GUARD" && -x "$PROJECT_DIR/global/claude/statusline.sh" ]] || return 1
}


@test "comment guard flags comments an edit adds to TypeScript, PHP, Python, and CSS" {
    run bash -c "jq -n '{tool_name: \"Edit\", tool_input: {file_path: \"/app/src/Card.tsx\", old_string: \"const a = 1;\", new_string: \"// compute the total\nconst a = 1; // one\n{/* header */}\"}}' | '$PROJECT_DIR/global/hooks/comment-guard.sh'"
    [[ "$(jq -r .decision <<<"$output")" == "block" ]] || return 1
    [[ "$output" == *"Card.tsx: // compute the total"* && "$output" == *"// one"* && "$output" == *"{/* header */}"* ]] || return 1

    run bash -c "jq -n '{tool_name: \"MultiEdit\", tool_input: {file_path: \"/app/User.php\", edits: [{old_string: \"\", new_string: \"# legacy hack\"}]}}' | '$PROJECT_DIR/global/hooks/comment-guard.sh'"
    [[ "$output" == *"User.php: # legacy hack"* ]] || return 1

    run bash -c "jq -n --arg p \$'*** Begin Patch\n*** Update File: app/svc.py\n@@\n-x = 1\n+x = 2  # bump\n*** End Patch' '{tool_name: \"apply_patch\", tool_input: {command: \$p}}' | '$PROJECT_DIR/global/hooks/comment-guard.sh'"
    [[ "$output" == *"app/svc.py:"*"# bump"* ]] || return 1

    run bash -c "jq -n '{tool_name: \"Edit\", tool_input: {file_path: \"/app/a.css\", old_string: \"\", new_string: \"/* reset */\"}}' | '$PROJECT_DIR/global/hooks/comment-guard.sh'"
    [[ "$output" == *"a.css: /* reset */"* ]] || return 1
}

@test "comment guard allows directives, docblock tags, links, moved comments, and non-code files" {
    for input in \
        '{"tool_name": "Edit", "tool_input": {"file_path": "/app/a.ts", "old_string": "", "new_string": "// eslint-disable-next-line no-console\n// @ts-expect-error vendor types\n// https://github.com/org/repo/issues/1"}}' \
        '{"tool_name": "Edit", "tool_input": {"file_path": "/app/A.php", "old_string": "", "new_string": "/**\n * @param array<int, User> $users\n */\n#[Override]"}}' \
        '{"tool_name": "Edit", "tool_input": {"file_path": "/app/a.ts", "old_string": "// keep\nlet a = 1", "new_string": "let b = 2\n// keep"}}' \
        '{"tool_name": "Edit", "tool_input": {"file_path": "/app/a.css", "old_string": "", "new_string": "* { margin: 0 }"}}' \
        '{"tool_name": "Edit", "tool_input": {"file_path": "/app/README.md", "old_string": "", "new_string": "# Title"}}' \
        '{"tool_name": "Edit", "tool_input": {"file_path": "/app/vendor/pkg/a.php", "old_string": "", "new_string": "// vendored"}}'; do
        run bash -c "printf '%s' '$input' | '$PROJECT_DIR/global/hooks/comment-guard.sh'"
        [[ "$status" -eq 0 && -z "$output" ]] || { echo "$input -> $output"; return 1; }
    done
}

@test "comment guard on a rewritten file flags only comments the committed version didn't have" {
    git -C "$TEST_TMP" init -q
    printf 'export const a = 1; // old note\n' > "$TEST_TMP/a.ts"
    git -C "$TEST_TMP" add a.ts && git -C "$TEST_TMP" -c user.email=t@t -c user.name=t commit -qm init
    run bash -c "jq -n --arg f '$TEST_TMP/a.ts' '{tool_name: \"Write\", tool_input: {file_path: \$f, content: \"export const a = 1; // old note\n// new note\nexport const b = 2;\"}}' | '$PROJECT_DIR/global/hooks/comment-guard.sh'"
    [[ "$output" == *"// new note"* ]] || return 1
    [[ "$output" != *"old note"* ]] || return 1
}
