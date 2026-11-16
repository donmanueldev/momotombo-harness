#!/usr/bin/env bats
# Tests for `harness cleanup`. xcrun, adb, docker, and ps are stubbed, and the harness runs with only the
# stubs and basic tools on PATH, so the machine's real simulators, emulators, and containers are never touched.

setup() {
    PROJECT_DIR="$(cd "$BATS_TEST_DIRNAME/.." && pwd)"
    TEST_TMP="$(cd "$(mktemp -d)" && pwd -P)"
    STUBS="$TEST_TMP/stubs"
    TOOLS="$TEST_TMP/tools"
    mkdir -p "$STUBS" "$TOOLS"
    for tool in readlink dirname awk sed sort cat; do ln -s "$(command -v "$tool")" "$TOOLS/$tool"; done

    cat > "$STUBS/xcrun" <<'SH'
#!/bin/bash
echo "xcrun $*" >> "$STUB_LOG"
if [ "$*" = "simctl list devices booted" ]; then
    printf '== Devices ==\n-- iOS 18.0 --\n    iPhone 16 Pro (11111111-2222-3333-4444-555555555555) (Booted)\n'
fi
SH
    cat > "$STUBS/adb" <<'SH'
#!/bin/bash
echo "adb $*" >> "$STUB_LOG"
if [ "$*" = "devices" ]; then printf 'List of devices attached\nemulator-5554\tdevice\n\n'; fi
SH
    cat > "$STUBS/docker" <<'SH'
#!/bin/bash
echo "docker $*" >> "$STUB_LOG"
if [ "$1" = ps ]; then printf 'abc123\tshop-db\tpostgres:17\n'; fi
SH
    cat > "$STUBS/ps" <<'SH'
#!/bin/bash
if [ "$1" = -o ]; then
    if [ -n "${PS_REUSED:-}" ]; then echo "/usr/libexec/another-program"; exit 0; fi
    awk -v p="$4" '$1 == p { sub(/^ *[0-9]+ +[0-9]+ +[0-9]+ +[0-9]+ +/, ""); print; found = 1 } END { exit !found }' "$PS_TABLE"
    exit
fi
cat "$PS_TABLE"
if [ -n "${PS_ANCESTORS:-}" ]; then
    up=$(/bin/ps -o ppid= -p "$PPID"); up=${up// /}
    top=$(/bin/ps -o ppid= -p "$up"); top=${top// /}
    printf '%s %s %s 1024 node /caller/.bin/vite\n' "$PPID" "$up" "$UID"
    printf '%s %s %s 1024 node /parent/.bin/vite\n' "$up" "$top" "$UID"
    printf '%s 1 %s 1024 node /grandparent/.bin/vite\n' "$top" "$UID"
fi
SH
    chmod +x "$STUBS"/*
    # A real process the dev-server stub reports, so `kill` is checked against a live pid.
    DEV_PID=$(sleep 60 >/dev/null 2>&1 & echo $!)
    export PS_TABLE="$TEST_TMP/ps-table"
    {
        echo "$DEV_PID 1 $UID 2048 node /app/node_modules/.bin/vite"
        echo "102 1 $UID 9999999 /Applications/Browser.app/Contents/MacOS/Browser"
        echo "103 1 0 4096 php artisan serve"
    } > "$PS_TABLE"
    export DEV_PID
    export STUB_LOG="$TEST_TMP/stub.log"
    touch "$STUB_LOG"
}

teardown() {
    kill "$DEV_PID" 2>/dev/null || true
    rm -rf "$TEST_TMP"
}

harness_cleanup() {
    PATH="$STUBS:$TOOLS" "$PROJECT_DIR/bin/harness" cleanup "$@"
}

answer() {
    printf '%s\n' "$@" > "$TEST_TMP/answers"
    export HARNESS_TTY="$TEST_TMP/answers"
}

dev_server_alive() {
    local i
    for i in 1 2 3 4 5 6 7 8 9 10; do
        kill -0 "$DEV_PID" 2>/dev/null || return 1
        /bin/sleep 0.1
    done
}

@test "cleanup lists every category and the top memory users" {
    run harness_cleanup --dry-run
    [[ "$status" -eq 0 ]] || return 1
    [[ "$output" == *"iPhone 16 Pro (11111111-2222-3333-4444-555555555555)"* ]] || return 1
    [[ "$output" == *"emulator-5554"* ]] || return 1
    [[ "$output" == *"shop-db (postgres:17, abc123)"* ]] || return 1
    [[ "$output" == *"$DEV_PID"*"2 MB  node /app/node_modules/.bin/vite"* ]] || return 1
    [[ "$output" == *"Browser.app"* ]] || return 1
    [[ "$output" == *"0 stopped, 4 kept, 0 failed"* ]] || return 1
}

@test "cleanup never offers another user's process or a process outside the categories" {
    answer y y y y y y
    run harness_cleanup
    [[ "$output" == *"Stop process $DEV_PID?"* ]] || return 1
    [[ "${output#*Dev servers}" != *"artisan serve"* ]] || return 1
    [[ "$output" != *"Stop process 102"* && "$output" != *"Stop process 103"* ]] || return 1
}

@test "--dry-run stops nothing" {
    answer y y y y
    run harness_cleanup --dry-run
    [[ "$status" -eq 0 ]] || return 1
    [[ "$output" != *"Stop "* ]] || return 1
    ! grep -qE 'shutdown|emu kill|docker stop' "$STUB_LOG" || return 1
    dev_server_alive || return 1
}

@test "answering no, or nothing, keeps everything" {
    answer n n
    run harness_cleanup
    [[ "$status" -eq 0 ]] || return 1
    [[ "$output" == *"Stop simulator iPhone 16 Pro? [y/N]"* ]] || return 1
    [[ "$output" == *"Stop process $DEV_PID? [y/N]"* ]] || return 1
    ! grep -qE 'shutdown|emu kill|docker stop' "$STUB_LOG" || return 1
    dev_server_alive || return 1
    [[ "$output" == *"0 stopped, 4 kept, 0 failed"* ]] || return 1
}

@test "answering yes stops each item with its own command and id" {
    answer y y y y
    run harness_cleanup
    [[ "$status" -eq 0 ]] || return 1
    grep -qx 'xcrun simctl shutdown 11111111-2222-3333-4444-555555555555' "$STUB_LOG" || return 1
    grep -qx 'adb -s emulator-5554 emu kill' "$STUB_LOG" || return 1
    grep -qx 'docker stop abc123' "$STUB_LOG" || return 1
    ! dev_server_alive || return 1
    [[ "$output" == *"4 stopped, 0 kept, 0 failed"* ]] || return 1
}

@test "only the items answered yes are stopped" {
    answer n y n n
    run harness_cleanup
    grep -qx 'adb -s emulator-5554 emu kill' "$STUB_LOG" || return 1
    ! grep -qE 'shutdown|docker stop' "$STUB_LOG" || return 1
    dev_server_alive || return 1
    [[ "$output" == *"1 stopped, 3 kept, 0 failed"* ]] || return 1
}

@test "a category whose CLI is missing is skipped without error" {
    rm "$STUBS/xcrun" "$STUBS/adb" "$STUBS/docker"
    answer n
    run harness_cleanup
    [[ "$status" -eq 0 ]] || return 1
    [[ "$output" == *"xcrun not found; skipped"* ]] || return 1
    [[ "$output" == *"adb not found; skipped"* ]] || return 1
    [[ "$output" == *"docker not found; skipped"* ]] || return 1
    [[ "$output" == *"Stop process $DEV_PID?"* ]] || return 1
}

@test "with Docker installed but not running, containers are skipped" {
    printf '#!/bin/bash\necho "docker $*" >> "$STUB_LOG"\n[ "$1" != info ]\n' > "$STUBS/docker"
    run harness_cleanup --dry-run
    [[ "$status" -eq 0 ]] || return 1
    [[ "$output" == *"Docker isn't running; skipped"* ]] || return 1
    ! grep -q 'docker ps' "$STUB_LOG" || return 1
}

@test "without a terminal to ask on, cleanup only lists" {
    export HARNESS_TTY="$TEST_TMP/no-terminal"
    run harness_cleanup
    [[ "$status" -eq 0 ]] || return 1
    [[ "$output" == *"no terminal to ask on; listing only"* ]] || return 1
    [[ "$output" == *"emulator-5554"* ]] || return 1
    ! grep -qE 'shutdown|emu kill|docker stop' "$STUB_LOG" || return 1
    dev_server_alive || return 1
}

@test "cleanup refuses --yes" {
    run harness_cleanup --yes
    [[ "$status" -ne 0 ]] || return 1
    [[ "$output" == *"has no --yes"* ]] || return 1
    [[ ! -s "$STUB_LOG" ]] || return 1
}

@test "only node, bun, or php processes count as dev servers, never wrappers or tools that mention them" {
    {
        echo "201 1 $UID 1024 /bin/zsh -c source /tmp/shell-snapshots/s.sh && eval 'npx expo start'"
        echo "202 1 $UID 1024 npx expo start --port 8081"
        echo "203 1 $UID 1024 /bin/sh /usr/local/bin/npx expo start --port 8081"
        echo "204 1 $UID 1024 npm exec expo start --port 8081"
        echo "205 1 $UID 1024 vim vite"
        echo "206 1 $UID 1024 grep -r metro ."
        echo "$DEV_PID 1 $UID 1024 node /app/node_modules/.bin/expo start --port 8081"
        echo "207 1 $UID 1024 /opt/homebrew/bin/php artisan queue:work"
    } > "$PS_TABLE"
    run harness_cleanup --dry-run
    [[ "$status" -eq 0 ]] || return 1
    servers="${output#*Dev servers}"
    [[ "$servers" == *"node /app/node_modules/.bin/expo start"* ]] || return 1
    [[ "$servers" == *"php artisan queue:work"* ]] || return 1
    for pid in 201 202 203 204 205 206; do
        [[ "$servers" != *" $pid  "* ]] || { echo "offered $pid"; return 1; }
    done
}

@test "a pid that runs another program by the time of the answer is not killed" {
    answer n n n y
    export PS_REUSED=1
    run harness_cleanup
    [[ "$status" -eq 0 ]] || return 1
    [[ "$output" == *"process $DEV_PID already exited; nothing stopped"* ]] || return 1
    dev_server_alive || return 1
}

@test "cleanup never offers its own process or the shells that started it" {
    answer n n n n n n n n
    export PS_ANCESTORS=1
    run harness_cleanup
    [[ "$status" -eq 0 ]] || return 1
    servers="${output#*Dev servers}"
    [[ "$servers" == *"node /app/node_modules/.bin/vite"* ]] || return 1
    [[ "$servers" != *"/caller/.bin/vite"* ]] || return 1
    [[ "$servers" != *"/parent/.bin/vite"* ]] || return 1
    [[ "$servers" != *"/grandparent/.bin/vite"* ]] || return 1
}

@test "with an answers source it can't read, cleanup only lists" {
    answer y y y y
    chmod 000 "$HARNESS_TTY"
    run harness_cleanup
    [[ "$status" -eq 0 ]] || return 1
    [[ "$output" == *"no terminal to ask on; listing only"* ]] || return 1
    ! grep -qE 'shutdown|emu kill|docker stop' "$STUB_LOG" || return 1
    dev_server_alive || return 1
}

@test "a stop that fails is reported as failed, not kept" {
    printf '#!/bin/bash\necho "docker $*" >> "$STUB_LOG"\nif [ "$1" = ps ]; then printf "abc123\\tshop-db\\tpostgres:17\\n"; fi\n[ "$1" != stop ]\n' > "$STUBS/docker"
    answer n n y n
    run harness_cleanup
    [[ "$output" == *"couldn't stop container shop-db"* ]] || return 1
    [[ "$output" == *"0 stopped, 3 kept, 1 failed"* ]] || return 1
}
