#!/usr/bin/env bats
# Tests for `harness init` (project layer). openspec and npx are stubbed so the suite runs offline.

setup() {
    PROJECT_DIR="$(cd "$BATS_TEST_DIRNAME/.." && pwd)"
    TEST_TMP="$(cd "$(mktemp -d)" && pwd -P)"
    APP="$TEST_TMP/shop"
    STUBS="$TEST_TMP/stubs"
    mkdir -p "$APP" "$STUBS"
    git -C "$APP" init -q

    cat > "$STUBS/openspec" <<'SH'
#!/bin/bash
echo "openspec $*" >> "$STUB_LOG"
mkdir -p openspec/specs openspec/changes .agents/skills/openspec-propose
printf 'schema: spec-driven\n\n# context: |\n#   example\n' > openspec/config.yaml
SH
    cat > "$STUBS/npx" <<'SH'
#!/bin/bash
echo "npx $*" >> "$STUB_LOG"
for skill in codely-doc-create codely-openspec-spec-create-from-code; do
    mkdir -p ".agents/skills/$skill" && echo "---" > ".agents/skills/$skill/SKILL.md"
done
SH
    chmod +x "$STUBS/openspec" "$STUBS/npx"
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

@test "init detects Expo and seeds only mobile docs" {
    echo '{"dependencies": {"expo": "~55.0.0"}}' > "$APP/package.json"
    run init
    [[ "$output" == *"stack: expo"* ]] || return 1
    [[ -f "$APP/docs/mobile/expo-router-screens.md" ]] || return 1
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

@test "init sets up OpenSpec with the momotombo schema and the Codely skills" {
    laravel_app
    run init
    grep -q 'openspec init --tools claude,codex' "$STUB_LOG" || return 1
    [[ -f "$APP/openspec/schemas/momotombo/schema.yaml" ]] || return 1
    grep -q '^schema: momotombo$' "$APP/openspec/config.yaml" || return 1
    grep -q 'skills@latest add CodelyTV/agent-harness --skill codely-doc-create --skill codely-openspec-spec-create-from-code -a claude-code -a codex -y' "$STUB_LOG" || return 1
}

@test "init never overwrites existing files and a re-run changes nothing" {
    laravel_app
    echo "# my agents" > "$APP/AGENTS.md"
    mkdir -p "$APP/docs/backend" "$APP/openspec"
    echo "mine" > "$APP/docs/backend/thin-controllers-and-actions.md"
    printf 'schema: spec-driven\ncontext: |\n  custom\n' > "$APP/openspec/config.yaml"

    run init
    [[ "$status" -eq 0 ]] || return 1
    grep -qx "# my agents" "$APP/AGENTS.md" || return 1
    grep -qx "mine" "$APP/docs/backend/thin-controllers-and-actions.md" || return 1
    grep -q 'custom' "$APP/openspec/config.yaml" || return 1
    [[ "$output" == *"set 'schema: momotombo'"* ]] || return 1
    ! grep -q 'openspec init' "$STUB_LOG" || return 1

    run init
    [[ "$output" != *"✓ "* ]] || return 1
    [[ "$(grep -c 'skills@latest' "$STUB_LOG")" -eq 1 ]] || return 1
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

@test "init refuses to run on the harness itself" {
    run "$PROJECT_DIR/bin/harness" init "$PROJECT_DIR"
    [[ "$status" -ne 0 ]] || return 1
}

@test "the momotombo schema and a change written from its templates pass OpenSpec strict validation" {
    real=$(PATH="${PATH#"$STUBS:"}" command -v openspec) || skip "openspec not installed"
    cd "$APP"
    export OPENSPEC_NO_UPDATE_CHECK=1 OPENSPEC_TELEMETRY=0 CI=true
    mkdir -p openspec/schemas openspec/changes/sample/specs/demo
    cp -R "$PROJECT_DIR/project/openspec/schemas/momotombo" openspec/schemas/
    cp "$PROJECT_DIR/project/openspec/config.yaml" openspec/config.yaml
    run "$real" schema validate momotombo
    [[ "$status" -eq 0 ]] || return 1
    sed -e 's/<!-- requirement name -->/Demo/' -e 's/<!-- one behaviour, in one or two sentences -->/show a demo/' \
        -e 's/<!-- action, with concrete values -->/the user opens the demo/' -e 's/<!-- observable outcome -->/the demo appears/' \
        -e '/^## Purpose/,/^## ADDED/{/^## ADDED/!d;}' \
        "$PROJECT_DIR/project/openspec/schemas/momotombo/templates/spec.md" > openspec/changes/sample/specs/demo/spec.md
    cp "$PROJECT_DIR/project/openspec/schemas/momotombo/templates/plan.md" openspec/changes/sample/plan.md
    run "$real" validate sample --strict
    [[ "$status" -eq 0 ]] || { echo "$output"; return 1; }
}
