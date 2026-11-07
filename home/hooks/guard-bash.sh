#!/bin/bash
# PreToolUse hook for the Bash tool (main session and subagents).
# Blocks commands that bypass safeguards or destroy work. Exit 2 = block; stderr goes back to Claude.
set -u

input=$(cat)
command=$(jq -r '.tool_input.command // empty' <<<"$input" 2>/dev/null) || exit 0
cwd=$(jq -r '.cwd // empty' <<<"$input" 2>/dev/null)
[ -n "$command" ] || exit 0

block() {
    echo "Blocked by agent-config guard: $1. If it is really intended, ask the user to run it themselves." >&2
    exit 2
}

# One line, each shell segment separated by ';' so patterns can't span `&&`, `||`, `|`.
flat=$(tr '\n' ' ' <<<"$command" | sed -E 's/(\&\&|\|\||\||;)/;/g')
seg='[^;]*'
sp='[[:space:]]'

[[ "$flat" =~ (^|;|$sp)git$sp$seg(push)$seg$sp(-f|--force|--force-with-lease)($sp|=|;|$) ]] \
    && block "force-push rewrites shared history"
[[ "$flat" =~ (^|;|$sp)git$sp$seg$sp--no-verify($sp|;|$) ]] \
    && block "--no-verify skips the repository's hooks"
[[ "$flat" =~ (^|;|$sp)git$sp$seg(reset)$seg$sp--hard($sp|;|$) ]] \
    && block "git reset --hard discards uncommitted work"
[[ "$flat" =~ (^|;|$sp)git$sp$seg(clean)$seg$sp-[a-zA-Z]*f ]] \
    && block "git clean -f deletes untracked files"
[[ "$flat" =~ (^|;|$sp)git$sp$seg(push)$sp$seg(origin$sp+)?(main|master)($sp|;|$) ]] \
    && block "pushing directly to main/master; open a pull request instead"
[[ "$flat" =~ (^|;|$sp)rm$sp+(-[a-zA-Z]*[rR][a-zA-Z]*$sp+)+(/|/\*|~|~/|~/\*|\$HOME|\$HOME/|\$HOME/\*|\*|\.|\./)($sp|;|$) ]] \
    && block "recursive rm on /, ~, \$HOME, . or *"

# Commits on protected branches: check the branch of the directory the command runs in.
if [[ "$flat" =~ (^|;|$sp)git$sp+(-[a-zA-Z]+$sp+)*commit($sp|;|$) ]] && [ -n "$cwd" ]; then
    branch=$(git -C "$cwd" branch --show-current 2>/dev/null)
    case "$branch" in
        main|master|develop) block "committing directly on '$branch'; create a feature/bugfix branch first" ;;
    esac
fi

exit 0
