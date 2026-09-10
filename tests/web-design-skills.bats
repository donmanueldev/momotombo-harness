#!/usr/bin/env bats

load 'test_helper'

@test "versioned web design skills include their required runtime resources" {
    [[ -f "$PROJECT_DIR/skills/frontend-design/LICENSE.txt" ]]
    [[ -f "$PROJECT_DIR/skills/ui-ux-pro-max/LICENSE.txt" ]]
    [[ -f "$PROJECT_DIR/skills/web-design-guidelines/LICENSE.txt" ]]
    [[ -f "$PROJECT_DIR/skills/ui-ux-pro-max/references/quick-reference.md" ]]
    [[ -f "$PROJECT_DIR/skills/web-design-guidelines/references/web-interface-guidelines.md" ]]

    run grep -F "e3d624baaf29dc1fc645aff3e38f03e564d2d6b1" \
        "$PROJECT_DIR/skills/web-design-guidelines/SKILL.md"
    [[ "$status" -eq 0 ]]

    run grep -F "raw.githubusercontent.com/vercel-labs/web-interface-guidelines/main" \
        "$PROJECT_DIR/skills/web-design-guidelines/SKILL.md"
    [[ "$status" -ne 0 ]]
}

@test "ui ux search supports only the approved web stacks" {
    local search="$PROJECT_DIR/skills/ui-ux-pro-max/scripts/search.py"

    run env PYTHONDONTWRITEBYTECODE=1 python3 "$search" \
        "form validation rerender" --stack react --max-results 1
    [[ "$status" -eq 0 ]]
    [[ "$output" == *"**Stack:** react"* ]]

    run env PYTHONDONTWRITEBYTECODE=1 python3 "$search" \
        "Inertia useForm errors" --stack laravel --max-results 1
    [[ "$status" -eq 0 ]]
    [[ "$output" == *"**Stack:** laravel"* ]]
    [[ "$output" == *"**Category:** Inertia.js"* ]]

    run env PYTHONDONTWRITEBYTECODE=1 python3 "$search" \
        "responsive form layout" --stack html-tailwind --max-results 1
    [[ "$status" -eq 0 ]]
    [[ "$output" == *"**Stack:** html-tailwind"* ]]

    run env PYTHONDONTWRITEBYTECODE=1 python3 "$search" \
        "component" --stack vue
    [[ "$status" -eq 2 ]]
    [[ "$output" == *"invalid choice: 'vue'"* ]]
}

@test "ui ux search reports an explicit no-match result" {
    run env PYTHONDONTWRITEBYTECODE=1 python3 \
        "$PROJECT_DIR/skills/ui-ux-pro-max/scripts/search.py" \
        "zzzxqv" --domain product --max-results 1

    [[ "$status" -eq 0 ]]
    [[ "$output" == *"**Found:** 0 results"* ]]
    [[ "$output" == *"No matches."* ]]
}

@test "web guideline reference requires clickable source findings" {
    run grep -F 'Use `file:line` format' \
        "$PROJECT_DIR/skills/web-design-guidelines/references/web-interface-guidelines.md"
    [[ "$status" -eq 0 ]]
}
