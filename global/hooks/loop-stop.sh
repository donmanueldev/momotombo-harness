#!/bin/bash
# Stop hook for Claude and Codex. While `harness loop start` has a loop running for this repository,
# each time the agent tries to stop it runs the loop's done command. If the command passes, or the
# round limit is reached, the loop ends and the agent stops; otherwise the agent is sent back to
# work with the goal and the command's failing output.
set -euo pipefail

input=$(cat)
cwd=$(jq -r '.cwd // empty' <<<"$input")
[ -n "$cwd" ] && [ -d "$cwd" ] || exit 0
cd "$cwd"
root=$(git rev-parse --show-toplevel 2>/dev/null || pwd -P)
file="${XDG_STATE_HOME:-$HOME/.local/state}/momotombo-harness/loops/$(tr -c 'A-Za-z0-9._\n' '-' <<<"$root").json"
[ -f "$file" ] || exit 0

goal=$(jq -r .goal "$file")
done_command=$(jq -r .done "$file")
max=$(jq -r .max "$file")
round=$(( $(jq -r .iteration "$file") + 1 ))

if result=$(cd "$root" && bash -c "$done_command" 2>&1); then
    rm -f "$file"
    echo "Loop finished in round $round: \`$done_command\` passes." >&2
    exit 0
fi

if [ "$round" -ge "$max" ]; then
    rm -f "$file"
    echo "Loop stopped after $max rounds; \`$done_command\` still fails. Report where it stands." >&2
    exit 0
fi

jq --argjson round "$round" '.iteration = $round' "$file" > "$file.tmp" && mv "$file.tmp" "$file"
reason="Autonomous loop, round $round of $max. The goal is not met yet: \`$done_command\` fails. Keep working toward it, following momotombo-mode; don't stop to ask.

Goal: $goal

Last output of \`$done_command\`:
$(tail -n 40 <<<"$result")"
jq -n --arg reason "$reason" '{decision: "block", reason: $reason}'
