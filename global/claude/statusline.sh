#!/bin/bash
# Claude Code status line: model · directory · git branch (* = dirty) · context used · session cost
input=$(cat)
IFS=$'\t' read -r model dir context cost < <(
    jq -r '[
        (.model.display_name // "?"),
        (.workspace.current_dir // .cwd // ""),
        (.context_window.used_percentage // "" | tostring),
        (.cost.total_cost_usd // "" | tostring)
    ] | @tsv' <<<"$input" 2>/dev/null
)

parts=("$model" "${dir##*/}")

if [ -n "$dir" ] && branch=$(git -C "$dir" branch --show-current 2>/dev/null) && [ -n "$branch" ]; then
    [ -n "$(git -C "$dir" status --porcelain 2>/dev/null | head -n 1)" ] && branch="$branch*"
    parts+=("$branch")
fi
[ -n "$context" ] && parts+=("ctx $(printf '%.0f' "$context")%")
[ -n "$cost" ] && parts+=("\$$(printf '%.2f' "$cost")")

out="${parts[0]}"
for part in "${parts[@]:1}"; do out="$out · $part"; done
printf '%s' "$out"
