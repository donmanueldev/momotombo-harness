#!/bin/bash
# PostToolUse hook for file edits: Claude's Edit, MultiEdit, and Write, and Codex's apply_patch
# (matched through its Edit|Write aliases). It lists the comments an edit added to source code and
# asks the agent to remove them, per momotombo-mode's code-says-what principle. Comments the edit
# only moved (present in the replaced text) don't count. The edit itself is never undone.
set -euo pipefail

input=$(cat)
tool=$(jq -r '.tool_name // empty' <<<"$input")

# One "path<TAB>+line" or "path<TAB>-line" per line the edit added or replaced.
edit_lines() {
    case "$tool" in
        Edit)
            jq -r '.tool_input | .file_path as $f
                | ((.new_string // "") | split("\n")[] | "\($f)\t+\(.)"),
                  ((.old_string // "") | split("\n")[] | "\($f)\t-\(.)")' <<<"$input" ;;
        MultiEdit)
            jq -r '.tool_input | .file_path as $f | .edits[]
                | ((.new_string // "") | split("\n")[] | "\($f)\t+\(.)"),
                  ((.old_string // "") | split("\n")[] | "\($f)\t-\(.)")' <<<"$input" ;;
        Write)
            local file
            file=$(jq -r '.tool_input.file_path' <<<"$input")
            jq -r '.tool_input | .file_path as $f | (.content // "") | split("\n")[] | "\($f)\t+\(.)"' <<<"$input"
            # The committed version, so rewriting a file doesn't count its old comments as new.
            { git -C "$(dirname "$file")" show "HEAD:./$(basename "$file")" 2>/dev/null || true; } | awk -v f="$file" '{ print f "\t-" $0 }' ;;
        apply_patch)
            jq -r '.tool_input.command // empty' <<<"$input" | awk '
                /^\*\*\* (Add|Update) File: / { file = $0; sub(/^\*\*\* (Add|Update) File: /, "", file); next }
                /^\*\*\* / { next }
                file != "" && /^[+-]/ { print file "\t" $0 }' ;;
    esac
}

findings=$(edit_lines | awk -F'\t' '
    function ext(path,    e) { e = tolower(path); sub(/.*\//, "", e); if (e ~ /\.blade\.php$/) return "blade"; sub(/.*\./, "", e); return e }
    function family(e) {
        if (e ~ /^(js|jsx|ts|tsx|mjs|cjs|go|rs|swift|kt|kts|java|dart|c|cc|cpp|h|cs)$/) return "c"
        if (e ~ /^(css|scss|less)$/) return "css"
        if (e == "php") return "php"
        if (e ~ /^(py|rb)$/) return "hash"
        if (e ~ /^(vue|svelte|html)$/) return "markup"
        if (e == "blade") return "blade"
        return ""
    }
    function unquoted(s,    d, q) { d = gsub(/"/, "&", s); q = gsub(sprintf("%c", 39), "&", s); return d % 2 == 0 && q % 2 == 0 }
    function comment(line, fam,    t) {
        t = line; sub(/^[ \t]+/, "", t)
        if (fam == "c" || fam == "php" || fam == "markup" || fam == "blade") {
            if (t ~ /^(\/\/|\/\*|\{\/\*|\*([ \t]|$))/) return t
            if (match(line, /([;,){}]|\])[ \t]+(\/\/[ \t]|\/\*)/)) return substr(line, RSTART + 1)
        }
        if (fam == "css" && t ~ /^(\/\*|\/\/)/) return t
        if (fam == "php") {
            if (t ~ /^#[^\[!]/) return t
            if (match(line, /[;,){}][ \t]+#[ \t]/)) return substr(line, RSTART + 1)
        }
        if (fam == "hash") {
            if (t ~ /^#/ && t !~ /^#!/) return t
            if (match(line, /[^ \t][ \t]+#[ \t]/) && unquoted(substr(line, 1, RSTART))) return substr(line, RSTART + 1)
        }
        if ((fam == "markup" || fam == "blade") && t ~ /^<!--/) return t
        if (fam == "blade" && t ~ /^\{\{--/) return t
        return ""
    }
    function allowed(c,    body) {
        body = c; gsub(/^[ \t]*(\/\/+|\/\*+|\*+|#+|<!--|\{\{--)[ \t]*/, "", body); gsub(/[ \t]*(\*\/|-->|--\}\})[ \t]*$/, "", body)
        if (body == "" || body == "*") return 1
        if (body ~ /^@(param|return|var|template|extends|implements|property|method|throws|use|mixin|see|deprecated|internal|api|phpstan|psalm)/) return 1
        if (body ~ /(eslint-|prettier-ignore|biome-ignore|@ts-|phpstan-|phpcs:|noqa|type: ignore|pyright:|ruff:|istanbul ignore|c8 ignore|SPDX-|[Cc]opyright|[Ll]icense|https?:\/\/)/) return 1
        if (body ~ /^-\*-/) return 1
        return 0
    }
    {
        path = $1; kind = substr($2, 1, 1); line = substr($2, 2)
        if (path ~ /(^|\/)(node_modules|vendor|dist|build|\.next|storage)\//) next
        fam = family(ext(path)); if (fam == "") next
        c = comment(line, fam); if (c == "") next
        key = path SUBSEP c
        if (kind == "-") { replaced[key] = 1; next }
        if (allowed(c)) next
        if (!(key in seen)) { seen[key] = 1; order[++n] = key; shown[key] = path ": " c }
    }
    END { for (i = 1; i <= n && i <= 10; i++) if (!(order[i] in replaced)) print "- " shown[order[i]] }')

[ -n "$findings" ] || exit 0

reason="This edit added comments. Momotombo code says what it does through names and structure (momotombo-mode, principles/code-says-what.md). Remove each comment below, or rename, extract, or type the code so it isn't needed. Keep one only if it is a license header, a public API doc, an issue link, or a why forced by something outside the code (vendor, platform, protocol), and say which in your reply.
$findings"
jq -n --arg reason "$reason" '{decision: "block", reason: $reason}'
