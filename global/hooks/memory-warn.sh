#!/bin/bash
# SessionStart and PreToolUse(Bash) hook for Claude and Codex. When memory is under pressure
# (see memory-pressure.sh), it adds the memory report to the agent's context, so the agent
# tells the user what uses the memory and offers to stop what isn't needed. It never blocks,
# approves, or rewrites a command. It runs on every Bash call, so a command that isn't
# heavy leaves before any probe or fork.
set -uo pipefail

# Builds, test suites, installs, containers, and simulator boots.
HEAVY='(^|[^[:alnum:]_.-])(xcodebuild|expo run:(ios|android)|eas build[^;&|]* --local|gradlew|gradle|pod install|artisan test|pest|phpunit|vitest|jest|playwright|pytest|composer (install|update)|(npm|pnpm|yarn|bun) (install|ci|i)|docker[ -]compose( [^;&|]*)? up|simctl boot)([^[:alnum:]_.-]|$)'
# Before heavy commands it warns, with the context and a notification, at most once per 30 minutes.
WARN_EVERY=1800

IFS= read -r -d '' input || true
# The JSON escapes newlines in the command; read them as spaces so `cd app\nnpm install` matches.
[[ ${input//\\[ntr]/ } =~ $HEAVY || $input == *'"SessionStart"'* ]] || exit 0

IFS=$'\t' read -r event command < <(jq -r '[.hook_event_name // "", (.tool_input.command // "" | gsub("[\t\n\r]"; " "))] | @tsv' <<<"$input" 2>/dev/null)
case "${event:-}" in
    SessionStart) intro="Memory is under pressure at the start of this session." ;;
    PreToolUse)
        [[ $command =~ $HEAVY ]] || exit 0
        intro="Memory is under pressure, and the next command is a heavy build, test, install, or container start: ${command:0:160}"$'\n'"The command runs as requested; this note doesn't block it." ;;
    *) exit 0 ;;
esac

state="${XDG_STATE_HOME:-$HOME/.local/state}/momotombo-harness"
stamp="$state/memory-warned"
if [ "$event" = PreToolUse ]; then
    last=$(stat -f %m "$stamp" 2>/dev/null || echo 0)
    [ $(( $(date +%s) - last )) -ge "$WARN_EVERY" ] || exit 0
fi

probe="${BASH_SOURCE[0]%/*}/memory-pressure.sh"
line=$("$probe")
[ $? -eq 1 ] || exit 0
report=$("$probe" --report)

if [ "$event" = PreToolUse ]; then
    mkdir -p "$state" && touch "$stamp"
    osascript -e "display notification \"$line. Ask the agent what is using it.\" with title \"Memory under pressure\"" >/dev/null 2>&1 || true
fi

jq -n --arg event "$event" --arg context "$intro"$'\n'"$report" \
    '{hookSpecificOutput: {hookEventName: $event, additionalContext: $context}}'
