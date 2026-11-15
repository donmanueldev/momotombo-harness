#!/bin/bash
# PreToolUse hook for the Bash tool (Claude and Codex, main session and subagents).
# Blocks commands that bypass safeguards or destroy work. Exit 2 = block; stderr goes back to the agent.
# Bash 3.2 compatible: no associative arrays.
set -u

block() {
    echo "Blocked by the harness guard: $1. If it is really intended, ask the user to run it themselves." >&2
    exit 2
}

command -v jq >/dev/null 2>&1 || block "jq is not installed, so the guard can't read the command (brew install jq)"
input=$(cat)
command=$(jq -r '.tool_input.command // empty' <<<"$input" 2>/dev/null) || block "the hook input is not valid JSON"
cwd=$(jq -r '.cwd // empty' <<<"$input" 2>/dev/null)
[ -n "$command" ] || exit 0
[ -n "$cwd" ] || cwd=$PWD

# Normalize into ';'-separated segments: drop quotes, split on shell operators and
# subshell/substitution brackets, and reduce /abs/path/git and /abs/path/rm to git and rm.
flat=$(tr '\n' ' ' <<<"$command" | sed -E \
    -e 's/\$\{HOME\}/$HOME/g' \
    -e "s/[\"']//g" \
    -e 's/(\&\&|\|\||\||;|\$\(|[()`{}])/;/g' \
    -e 's#(^|[[:space:];])/[^[:space:];]*/(git|rm)([[:space:]]|;|$)#\1\2\3#g')

resolve_dir() {
    local base=$1 target=$2
    target=${target/#\~/$HOME}
    target=${target/#\$HOME/$HOME}
    [[ "$target" = /* ]] || target="$base/$target"
    (cd "$target" 2>/dev/null && pwd) || echo "$base"
}

# A `git checkout/switch <branch>` earlier in the same command changes the branch later commits land on.
switched_repo=""
switched_branch=""
branch_of() {
    if [ -n "$switched_repo" ] && [ "$1" = "$switched_repo" ]; then
        echo "$switched_branch"
    else
        git -C "$1" branch --show-current 2>/dev/null
    fi
}

is_protected() { case "$1" in main|master|develop) return 0 ;; esac; return 1; }

check_rm() {
    local recursive=0 dangerous="" endopts=0 t
    for t in "$@"; do
        if [ $endopts -eq 0 ] && [ "$t" = "--" ]; then endopts=1; continue; fi
        if [ $endopts -eq 0 ] && [[ "$t" = --* ]]; then
            [ "$t" = "--recursive" ] && recursive=1
            continue
        fi
        if [ $endopts -eq 0 ] && [[ "$t" = -* ]]; then
            [[ "$t" = *[rR]* ]] && recursive=1
            continue
        fi
        case "$t" in
            / | /\* | \~ | \~/ | \~/\* | \$HOME | \$HOME/ | \$HOME/\* | "$HOME" | "$HOME/" | \* | . | ./ | ./\* | .. | ../) dangerous=$t ;;
        esac
    done
    [ $recursive -eq 1 ] && [ -n "$dangerous" ] && block "recursive rm on $dangerous"
    return 0
}

check_git() {
    local repo=$dir sub="" t
    # Global options before the subcommand.
    while [ $# -gt 0 ]; do
        case "$1" in
            -C) repo=$(resolve_dir "$dir" "${2:-.}"); shift 2 || shift ;;
            -c) [[ "${2:-}" = core.hooksPath* || "${2:-}" = core.hookspath* ]] && block "overriding core.hooksPath skips the repository's hooks"
                shift 2 || shift ;;
            -*) shift ;;
            *) sub=$1; shift; break ;;
        esac
    done

    for t in "$@"; do
        [ "$t" = "--no-verify" ] && block "--no-verify skips the repository's hooks"
    done

    case "$sub" in
        push)
            local positional=() refspec
            for t in "$@"; do
                case "$t" in
                    --force | --force=* | --force-with-lease | --force-with-lease=* | --force-if-includes) block "force-push rewrites shared history" ;;
                    --*) ;;
                    -*) [[ "$t" = *f* ]] && block "force-push rewrites shared history" ;;
                    +*) block "a +refspec force-pushes and rewrites shared history" ;;
                    *) positional+=("$t") ;;
                esac
            done
            local current
            current=$(branch_of "$repo")
            if [ ${#positional[@]} -le 1 ]; then
                [[ "$current" = main || "$current" = master ]] &&
                    block "pushing directly to $current; open a pull request instead"
            else
                for refspec in "${positional[@]:1}"; do
                    case "${refspec##*:}" in
                        main | master | refs/heads/main | refs/heads/master) block "pushing directly to main/master; open a pull request instead" ;;
                        HEAD) [[ "$current" = main || "$current" = master ]] &&
                            block "pushing directly to main/master; open a pull request instead" ;;
                    esac
                done
            fi
            ;;
        reset)
            for t in "$@"; do [ "$t" = "--hard" ] && block "git reset --hard discards uncommitted work"; done
            ;;
        clean)
            for t in "$@"; do
                case "$t" in
                    --force) block "git clean --force deletes untracked files" ;;
                    --*) ;;
                    -*) [[ "$t" = *f* ]] && block "git clean -f deletes untracked files" ;;
                esac
            done
            ;;
        commit)
            for t in "$@"; do
                case "$t" in
                    --*) [[ "$t" = --message* || "$t" = --file* ]] && { message=1; break; } ;;
                    -*) [[ "$t" = *n* ]] && block "git commit -n is --no-verify and skips the repository's hooks"
                        [[ "$t" = *[mF]* ]] && { message=1; break; } ;; # the rest is the message
                esac
            done
            local branch
            branch=$(branch_of "$repo")
            is_protected "$branch" && block "committing directly on '$branch'; create a feature/bugfix branch first"
            ;;
        checkout | switch)
            local last="" files=0
            for t in "$@"; do
                [ "$t" = "--" ] && files=1
                [[ "$t" = -* ]] || last=$t
            done
            if [ $files -eq 0 ] && [ -n "$last" ]; then
                switched_repo=$repo
                switched_branch=$last
            fi
            ;;
    esac
    return 0
}

# A dotenv file holding real secrets: .env or .env.<anything>, except committed templates.
is_secret_env() {
    local base=${1##*/}
    case "$base" in
        .env.example | .env.sample | .env.template | .env.dist) return 1 ;;
        .env | .env.*) return 0 ;;
    esac
    return 1
}

# Commands that print or load a file's contents.
check_read() {
    local t
    for t in "$@"; do
        is_secret_env "$t" && block "reading $t would expose secrets; ask the user for the values you need"
    done
    return 0
}

dir=$cwd
IFS=';' read -ra segments <<<"$flat"
for segment in "${segments[@]}"; do
    read -ra toks <<<"$segment"
    [ ${#toks[@]} -gt 0 ] || continue
    if [ "${toks[0]}" = "cd" ]; then
        dir=$(resolve_dir "$dir" "${toks[1]:-$HOME}")
        continue
    fi
    for ((i = 0; i < ${#toks[@]}; i++)); do
        case "${toks[$i]}" in
            git)
                message=0
                check_git "${toks[@]:$((i + 1))}"
                [ $message -eq 1 ] && break # the rest of the segment is a commit message
                ;;
            rm) check_rm "${toks[@]:$((i + 1))}" ;;
            cat | less | more | head | tail | bat | nl | strings | xxd | od | base64 | grep | egrep | rg | sed | awk | source | .)
                check_read "${toks[@]:$((i + 1))}" ;;
        esac
    done
done

exit 0
