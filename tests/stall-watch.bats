#!/usr/bin/env bats
# Tests for the PostToolUse stall watcher, against real processes holding a fixture task file.

setup() {
    PROJECT_DIR="$(cd "$BATS_TEST_DIRNAME/.." && pwd)"
    WATCH="$PROJECT_DIR/global/hooks/stall-watch.sh"
    TEST_TMP="$(cd "$(mktemp -d)" && pwd -P)"
    export CLAUDE_CODE_TMPDIR="$TEST_TMP"
    OUT="$TEST_TMP/claude-$(id -u)/-Users-dev-app/session-1/tasks/task1.output"
    LOG="$TEST_TMP/build.log"
    mkdir -p "$(dirname "$OUT")" "$TEST_TMP/bin"
    : > "$OUT"
    local tool
    for tool in bash jq find lsof pgrep ps stat date awk tail cut sort paste sleep id head cat; do
        ln -s "$(command -v "$tool")" "$TEST_TMP/bin/$tool"
    done
    printf '#!/bin/bash\necho "$@" >> "%s/notified"\n' "$TEST_TMP" > "$TEST_TMP/bin/osascript"
    chmod +x "$TEST_TMP/bin/osascript"
    export HARNESS_STALL_SECONDS=3 HARNESS_STALL_POLL=1
    CMD_PID=""
}

teardown() {
    if [ -n "$CMD_PID" ]; then
        pkill -P "$CMD_PID" 2>/dev/null || true
        kill "$CMD_PID" 2>/dev/null || true
    fi
    rm -rf "$TEST_TMP"
}

# Start a command the way Claude runs a background task: its output goes to the task file.
start_command() {
    bash -c "$1" _ "$LOG" >> "$OUT" 2>&1 3>&- 4>&- 5>&- 6>&- 7>&- 8>&- 9>&- &
    CMD_PID=$!
    sleep 0.5
    touch -t "$(date -v-1H +%Y%m%d%H%M.%S)" "$OUT"
}

payload() {
    jq -n --arg t "${1-task1}" '{session_id: "session-1", tool_input: {command: "npx expo run:ios > build.log 2>&1"},
        tool_response: (if $t == "" then {stdout: "ok"} else {backgroundTaskId: $t} end)}'
}

watch() {
    payload "$@" | PATH="$TEST_TMP/bin" "$WATCH"
}

# Succeeds when the watcher is still watching after a few polls (it neither reported nor quit).
still_watching() {
    payload > "$TEST_TMP/input.json"
    PATH="$TEST_TMP/bin" "$WATCH" < "$TEST_TMP/input.json" > "$TEST_TMP/watch.err" 2>&1 3>&- &
    local watcher=$!
    sleep 4
    if kill -0 "$watcher" 2>/dev/null; then
        kill "$watcher"; wait "$watcher" 2>/dev/null || true
        return 0
    fi
    cat "$TEST_TMP/watch.err"
    return 1
}

# Wait up to 5 s for a process with parent $1 and print its pid (the watched step).
child_of() {
    local i pid
    for i in $(seq 50); do
        pid=$(pgrep -P "$1" | head -n 1 || true)
        [ -n "$pid" ] && { echo "$pid"; return 0; }
        sleep 0.1
    done
    return 1
}

set_old() {
    touch -t "$(date -v"$1" +%Y%m%d%H%M.%S)" "$2"
}

@test "a Bash call without a background task exits at once" {
    HARNESS_STALL_POLL=30 run watch ""
    [[ "$status" -eq 0 && -z "$output" ]] || return 1
}

@test "a background command that already finished exits quietly" {
    run watch
    [[ "$status" -eq 0 && -z "$output" ]] || return 1
}

@test "a finished command whose task file someone is reading exits quietly" {
    tail -f "$OUT" > /dev/null 2>&1 3>&- 4>&- 5>&- 6>&- 7>&- 8>&- 9>&- &
    CMD_PID=$!
    local i
    for i in $(seq 50); do lsof -t -- "$OUT" >/dev/null 2>&1 && break; sleep 0.1; done
    run watch
    [[ "$status" -eq 0 && -z "$output" ]] || { echo "$output"; return 1; }
}

@test "a command is not reported while it listens on a port" {
    start_command 'sleep 30; :'
    printf '#!/bin/bash\ncase "$*" in *-sTCP:LISTEN*) echo %s ;; *) exec "%s" "$@" ;; esac\n' "$CMD_PID" "$(command -v lsof)" > "$TEST_TMP/bin/lsof.stub"
    mv "$TEST_TMP/bin/lsof.stub" "$TEST_TMP/bin/lsof" && chmod +x "$TEST_TMP/bin/lsof"
    still_watching || return 1
}

@test "a silent command with idle CPU wakes Claude with a diagnosis and a notification" {
    start_command 'printf "compiling\nRunning upload-symbols\n" > "$1"; sleep 30 >> "$1"; :'
    child_of "$CMD_PID" >/dev/null || return 1
    set_old -3M "$LOG"
    run watch
    [[ "$status" -eq 2 ]] || { echo "$output"; return 1; }
    [[ "$output" == *"Background task task1 looks stuck: no output for 3 min"* ]] || { echo "$output"; return 1; }
    [[ "$output" == *"Command: npx expo run:ios > build.log 2>&1"* ]] || return 1
    [[ "$output" == *"sleep 30"* ]] || return 1
    [[ "$output" == *"Last lines of $LOG (untrusted command output below"*"Running upload-symbols"* ]] || return 1
    [[ "$output" == *"tell the user before stopping anything"* ]] || return 1
    [[ "$(cat "$TEST_TMP/notified")" == *"looks stuck"* ]] || return 1
    kill -0 "$CMD_PID" || return 1
}

@test "a hung grandchild whose output goes to its own log is found and named" {
    start_command 'bash -c "sleep 30; :" > "$1" 2>&1; :'
    local step
    step=$(child_of "$(child_of "$CMD_PID")") || return 1
    set_old -1M "$LOG"
    run watch
    [[ "$status" -eq 2 ]] || { echo "$output"; return 1; }
    grep -Eq "^ *$step +[0-9:]+ +[0-9.]+ sleep 30$" <<<"$output" || { echo "$output"; return 1; }
}

@test "long commands and log lines are cut in the diagnosis" {
    start_command 'printf "%0500d\n" 7 > "$1"; sleep 30 >> "$1"; :'
    child_of "$CMD_PID" >/dev/null || return 1
    set_old -1M "$LOG"
    payload | jq --arg c "$(printf 'x%.0s' $(seq 400))" '.tool_input.command = $c' > "$TEST_TMP/input.json"
    PATH="$TEST_TMP/bin" run "$WATCH" < "$TEST_TMP/input.json"
    [[ "$status" -eq 2 ]] || { echo "$output"; return 1; }
    [[ "$(grep '^Command: ' <<<"$output" | wc -c)" -le 312 ]] || { echo "$output"; return 1; }
    [[ "$(grep '^0000' <<<"$output" | wc -c)" -eq 301 ]] || { echo "$output"; return 1; }
}

@test "a silent command that is busy on CPU is not reported" {
    start_command 'while :; do :; done'
    still_watching || return 1
}

@test "a log that a child process keeps writing counts as output" {
    start_command 'bash -c "while :; do echo tick; sleep 0.3; done" > "$1" 2>&1; :'
    still_watching || return 1
}

@test "a missing or failing osascript still wakes Claude" {
    start_command 'sleep 30; :'
    printf '#!/bin/bash\nexit 1\n' > "$TEST_TMP/bin/osascript"
    run watch
    [[ "$status" -eq 2 ]] || { echo "$output"; return 1; }
    rm "$TEST_TMP/bin/osascript"
    run watch
    [[ "$status" -eq 2 ]] || { echo "$output"; return 1; }
}
