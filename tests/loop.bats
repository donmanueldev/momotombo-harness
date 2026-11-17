#!/usr/bin/env bats
# Tests for `harness loop` and the Stop hook that keeps the agent working until the done command passes.

setup() {
    PROJECT_DIR="$(cd "$BATS_TEST_DIRNAME/.." && pwd)"
    TEST_TMP="$(cd "$(mktemp -d)" && pwd -P)"
    export XDG_STATE_HOME="$TEST_TMP/state"
    APP="$TEST_TMP/app"
    mkdir -p "$APP" && git -C "$APP" init -q
}

teardown() {
    rm -rf "$TEST_TMP"
}

loop() {
    (cd "$APP" && "$PROJECT_DIR/bin/harness" loop "$@")
}

stop_hook() {
    jq -n --arg d "$APP" '{hook_event_name: "Stop", cwd: $d}' | "$PROJECT_DIR/global/hooks/loop-stop.sh"
}

@test "without a loop the Stop hook lets the agent stop" {
    run stop_hook
    [[ "$status" -eq 0 && -z "$output" ]] || return 1
}

@test "loop start needs a bounded round count, a done command, and a goal" {
    run loop start --done "true" "goal"
    [[ "$status" -ne 0 && "$output" == *"--max must be a number from 1 to 50"* ]] || return 1
    run loop start --max 99 --done "true" "goal"
    [[ "$status" -ne 0 ]] || return 1
    run loop start --max 3 "goal"
    [[ "$status" -ne 0 && "$output" == *"--done needs the command"* ]] || return 1
    run loop start --max 3 --done "true"
    [[ "$status" -ne 0 && "$output" == *"give the goal"* ]] || return 1
}

@test "a failing done command sends the agent back with the goal and the output, round by round" {
    loop start --max 3 --done "echo tests red; exit 1" "make the tests pass"
    run stop_hook
    [[ "$(jq -r .decision <<<"$output")" == "block" ]] || return 1
    [[ "$(jq -r .reason <<<"$output")" == *"round 1 of 3"*"make the tests pass"*"tests red"* ]] || return 1
    run loop status
    [[ "$output" == *"round 1 of 3"* ]] || return 1

    run stop_hook
    [[ "$(jq -r .reason <<<"$output")" == *"round 2 of 3"* ]] || return 1
    run stop_hook
    [[ "$status" -eq 0 && "$output" == *"Loop stopped after 3 rounds"* ]] || return 1
    run loop status
    [[ "$output" == "no loop running here" ]] || return 1
}

@test "a passing done command ends the loop and lets the agent stop" {
    loop start --max 5 --done "test -f done.txt" "create done.txt"
    run stop_hook
    [[ "$(jq -r .decision <<<"$output")" == "block" ]] || return 1
    touch "$APP/done.txt"
    run stop_hook
    [[ "$status" -eq 0 && "$output" == *"Loop finished in round 2"* ]] || return 1
    run loop status
    [[ "$output" == "no loop running here" ]] || return 1
}

@test "a loop belongs to one repository and loop stop ends it" {
    loop start --max 5 --done "false" "goal"
    run loop start --max 5 --done "false" "again"
    [[ "$status" -ne 0 && "$output" == *"already running"* ]] || return 1
    other="$TEST_TMP/other" && mkdir -p "$other" && git -C "$other" init -q
    run bash -c "jq -n --arg d '$other' '{cwd: \$d}' | '$PROJECT_DIR/global/hooks/loop-stop.sh'"
    [[ -z "$output" ]] || return 1
    run loop stop
    [[ "$output" == *"loop stopped"* ]] || return 1
    run stop_hook
    [[ -z "$output" ]] || return 1
}
