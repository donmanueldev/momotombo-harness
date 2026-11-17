#!/usr/bin/env bats
# Tests for `harness init` (project layer). npx is stubbed so the suite runs offline.

setup() {
    PROJECT_DIR="$(cd "$BATS_TEST_DIRNAME/.." && pwd)"
    TEST_TMP="$(cd "$(mktemp -d)" && pwd -P)"
    APP="$TEST_TMP/shop"
    STUBS="$TEST_TMP/stubs"
    mkdir -p "$APP" "$STUBS"
    git -C "$APP" init -q

    cat > "$STUBS/npx" <<'SH'
#!/bin/bash
echo "npx $*" >> "$STUB_LOG"
mkdir -p .agents/skills/codely-doc-create && echo "---" > .agents/skills/codely-doc-create/SKILL.md
SH
    chmod +x "$STUBS/npx"
    export STUB_LOG="$TEST_TMP/stub.log"
    export PATH="$STUBS:$PATH"
}

teardown() {
    rm -rf "$TEST_TMP"
}

init() {
    "$PROJECT_DIR/bin/harness" init "$APP" "$@"
}

laravel_app() {
    echo '{"require": {"laravel/framework": "^12.0"}}' > "$APP/composer.json"
    echo '{"scripts": {"build": "vite build", "lint": "eslint ."}}' > "$APP/package.json"
}

@test "init detects Laravel and seeds its docs, AGENTS.md, and CLAUDE.md" {
    laravel_app
    run init
    [[ "$status" -eq 0 ]] || return 1
    [[ "$output" == *"stack: laravel"* ]] || return 1
    [[ -f "$APP/docs/backend/thin-controllers-and-actions.md" ]] || return 1
    [[ -f "$APP/docs/testing/pest-feature-tests.md" ]] || return 1
    [[ ! -e "$APP/docs/mobile" ]] || return 1
    [[ "$(readlink "$APP/CLAUDE.md")" == "AGENTS.md" ]] || return 1
    grep -q 'php artisan test' "$APP/AGENTS.md" || return 1
    grep -q 'package.json scripts: `build`, `lint`' "$APP/AGENTS.md" || return 1
    grep -q '(docs/backend/form-requests-and-policies.md)' "$APP/AGENTS.md" || return 1
}

@test "init names the project's check script in AGENTS.md, with the lockfile's runner" {
    echo '{"scripts": {"check": "tsc && jest"}}' > "$APP/package.json"
    for pair in "bun.lock:bun run check" "pnpm-lock.yaml:pnpm run check" "yarn.lock:yarn run check" ":npm run check"; do
        rm -f "$APP/AGENTS.md" "$APP/bun.lock" "$APP/pnpm-lock.yaml" "$APP/yarn.lock"
        [[ -z "${pair%%:*}" ]] || touch "$APP/${pair%%:*}"
        run init --no-codely
        grep -qF "**Check:** \`${pair#*:}\`" "$APP/AGENTS.md" || { echo "$pair"; return 1; }
    done
}

@test "with check scripts in composer.json and package.json, AGENTS.md names both" {
    echo '{"require": {"laravel/framework": "^12.0"}, "scripts": {"check": "@test"}}' > "$APP/composer.json"
    echo '{"scripts": {"check": "tsc"}}' > "$APP/package.json"
    run init --no-codely
    grep -qF '**Check:** `composer check && npm run check`' "$APP/AGENTS.md" || return 1
}

@test "init asks for a check script when there is none, even with a broken package.json" {
    laravel_app
    echo '{ not json' > "$APP/package.json"
    run init --no-codely
    [[ "$status" -eq 0 ]] || return 1
    grep -q 'TODO: add one `check` script' "$APP/AGENTS.md" || return 1
    [[ "$output" == *"no 'check' script"* ]] || return 1
}

@test "on a repo with its own AGENTS.md, init flags a check command it doesn't name" {
    echo '{"require": {"laravel/framework": "^12.0"}, "scripts": {"check": "@test"}}' > "$APP/composer.json"
    echo 'Run `composer check-platform-reqs` first.' > "$APP/AGENTS.md"
    run init --no-codely
    [[ "$output" == *'AGENTS.md doesn'\''t name the check command. Add: **Check:** `composer check`'* ]] || return 1
    echo '**Check:** `composer check`' >> "$APP/AGENTS.md"
    run init --no-codely
    [[ "$output" != *"doesn't name the check command"* ]] || return 1
}

@test "with Laravel Boost, a check command named in a guideline counts" {
    echo '{"require": {"laravel/framework": "^12.0"}, "scripts": {"check": "@test"}}' > "$APP/composer.json"
    echo '{"agents": ["claude_code"]}' > "$APP/boost.json"
    echo "<laravel-boost-guidelines>x</laravel-boost-guidelines>" > "$APP/AGENTS.md"
    mkdir -p "$APP/.ai/guidelines"
    echo 'Run `composer check` before finishing.' > "$APP/.ai/guidelines/checks.md"
    run init --no-codely
    [[ "$output" != *"doesn't name the check command"* ]] || return 1
}

@test "init detects Expo and seeds only mobile docs" {
    echo '{"dependencies": {"expo": "~55.0.0"}}' > "$APP/package.json"
    run init
    [[ "$output" == *"stack: expo"* ]] || return 1
    [[ -f "$APP/docs/mobile/expo-router-screens.md" ]] || return 1
    [[ -f "$APP/docs/mobile/native-config-and-permissions.md" && -f "$APP/docs/mobile/server-and-client-state.md" ]] || return 1
    grep -q "docs/mobile/server-and-client-state.md" "$APP/AGENTS.md" || return 1
    [[ ! -e "$APP/docs/backend" ]] || return 1
    grep -q 'npx expo start' "$APP/AGENTS.md" || return 1
}

@test "--stack overrides detection and rejects unknown stacks" {
    run init --stack laravel,expo
    [[ -f "$APP/docs/backend/form-requests-and-policies.md" && -f "$APP/docs/mobile/secure-storage-and-api.md" ]] || return 1
    run init --stack rails
    [[ "$status" -ne 0 ]] || return 1
    [[ "$output" == *"unknown stack 'rails'"* ]] || return 1
}

@test "init installs the Codely doc skill and no OpenSpec" {
    laravel_app
    run init
    grep -q 'skills@1.8.1 add CodelyTV/agent-harness --skill codely-doc-create -a claude-code -a codex -y' "$STUB_LOG" || return 1
    [[ ! -e "$APP/openspec" ]] || return 1
    ! grep -qi 'openspec' "$APP/AGENTS.md" || return 1
}

@test "init leaves a project's existing openspec folder alone" {
    laravel_app
    mkdir -p "$APP/openspec/specs/orders"
    echo "spec" > "$APP/openspec/specs/orders/spec.md"
    run init --no-codely
    [[ "$status" -eq 0 ]] || return 1
    grep -qx "spec" "$APP/openspec/specs/orders/spec.md" || return 1
}

@test "init never overwrites existing files and a re-run changes nothing" {
    laravel_app
    echo "# my agents" > "$APP/AGENTS.md"
    mkdir -p "$APP/docs/backend"
    echo "mine" > "$APP/docs/backend/thin-controllers-and-actions.md"

    run init
    [[ "$status" -eq 0 ]] || return 1
    grep -qx "# my agents" "$APP/AGENTS.md" || return 1
    grep -qx "mine" "$APP/docs/backend/thin-controllers-and-actions.md" || return 1

    run init
    [[ "$output" != *"✓ "* ]] || return 1
    [[ "$(grep -c 'skills@' "$STUB_LOG")" -eq 1 ]] || return 1
}

@test "init refuses to merge a standalone CLAUDE.md" {
    echo "# old claude instructions" > "$APP/CLAUDE.md"
    run init --no-codely
    [[ "$output" == *"CLAUDE.md exists as its own file"* ]] || return 1
    grep -q "old claude instructions" "$APP/CLAUDE.md" || return 1
}

@test "dry run changes nothing" {
    laravel_app
    run init --dry-run
    [[ "$status" -eq 0 ]] || return 1
    [[ "$output" == *"[dry-run] would create AGENTS.md"* ]] || return 1
    [[ ! -e "$APP/AGENTS.md" && ! -e "$APP/docs" ]] || return 1
    [[ ! -e "$STUB_LOG" ]] || return 1
}

@test "dry run on a repo with its own AGENTS.md lists the docs map it should have" {
    laravel_app
    echo "# mine" > "$APP/AGENTS.md"
    mkdir -p "$APP/docs/backend"
    printf '# Thin controllers\n' > "$APP/docs/backend/thin-controllers.md"
    run init --dry-run
    [[ "$output" == *"Make sure its docs map lists these existing docs"* ]] || return 1
    [[ "$output" == *"- [Thin controllers](docs/backend/thin-controllers.md)"* ]] || return 1
}

@test "with Laravel Boost, init writes the docs map as a Boost guideline and keeps it" {
    laravel_app
    echo '{"agents": ["claude_code"], "guidelines": true}' > "$APP/boost.json"
    echo "<laravel-boost-guidelines>x</laravel-boost-guidelines>" > "$APP/AGENTS.md"
    run init --no-codely
    [[ "$status" -eq 0 ]] || return 1
    grep -qx '# Docs map' "$APP/.ai/guidelines/docs-map.md" || return 1
    grep -q '(docs/backend/form-requests-and-policies.md)' "$APP/.ai/guidelines/docs-map.md" || return 1
    [[ "$output" == *"php artisan boost:update"* ]] || return 1
    [[ "$output" != *"Make sure its docs map lists"* ]] || return 1
    grep -qx "<laravel-boost-guidelines>x</laravel-boost-guidelines>" "$APP/AGENTS.md" || return 1

    echo "custom" > "$APP/.ai/guidelines/docs-map.md"
    run init --no-codely
    grep -qx "custom" "$APP/.ai/guidelines/docs-map.md" || return 1
}

@test "dry run finishes on a repo with AGENTS.md and no docs folder" {
    laravel_app
    echo "# mine" > "$APP/AGENTS.md"
    run init --dry-run
    [[ "$status" -eq 0 ]] || { echo "$output"; return 1; }
    [[ "$output" == *"Run without --dry-run to apply."* ]] || return 1
}

@test "with Laravel Boost and no AGENTS.md, the docs map is written once, as the Boost guideline" {
    laravel_app
    echo '{"agents": ["claude_code"]}' > "$APP/boost.json"
    run init --no-codely
    [[ "$status" -eq 0 ]] || return 1
    grep -q '(docs/backend/form-requests-and-policies.md)' "$APP/.ai/guidelines/docs-map.md" || return 1
    ! grep -q '(docs/backend/form-requests-and-policies.md)' "$APP/AGENTS.md" || return 1
    grep -q '.ai/guidelines/docs-map.md' "$APP/AGENTS.md" || return 1
}

@test "with Laravel Boost, a dry run writes no guideline" {
    laravel_app
    echo '{"agents": ["claude_code"]}' > "$APP/boost.json"
    run init --dry-run
    [[ "$output" == *"would write .ai/guidelines/docs-map.md"* ]] || return 1
    [[ ! -e "$APP/.ai" ]] || return 1
}

@test "init refuses to run on the harness itself" {
    run "$PROJECT_DIR/bin/harness" init "$PROJECT_DIR"
    [[ "$status" -ne 0 ]] || return 1
}
