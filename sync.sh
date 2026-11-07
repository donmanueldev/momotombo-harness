#!/bin/bash
set -euo pipefail

CONFIG_DIR="$(cd "$(dirname "$0")" && pwd)"
BACKUP_DIR="$CONFIG_DIR/.backup"
DRY_RUN=false
CLAUDE_HOME="$HOME/.claude"
SKILL_HOME="$CLAUDE_HOME/skills"
AGENT_HOME="$CLAUDE_HOME/agents"
REPO_SKILL_DIR="$CONFIG_DIR/skills"
REPO_AGENT_DIR="$CONFIG_DIR/agents"

shopt -s nullglob

# Colors for output
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
BOLD='\033[1m'
RESET='\033[0m'

is_repo_symlink() {
    local path="$1"
    if [ -L "$path" ] && [[ "$(readlink "$path")" == "$CONFIG_DIR"* ]]; then
        return 0
    fi
    return 1
}

normalize_path() {
    local value="$1"
    printf '%s' "${value%/}"
}

capitalize() {
    local str="$1"
    local first="${str:0:1}"
    local rest="${str:1}"
    printf '%s%s' "$(echo "$first" | tr '[:lower:]' '[:upper:]')" "$rest"
}

validate_item_name() {
    local name="$1"

    if [[ ! "$name" =~ ^[a-z0-9][a-z0-9-]*$ ]]; then
        echo "Invalid item name: '$name'. Use lowercase letters, digits, and hyphens only." >&2
        return 1
    fi
}

create_backup() {
    local timestamp
    timestamp=$(date +%Y%m%d_%H%M%S)
    local backup_path="$BACKUP_DIR/$timestamp"
    mkdir -p "$backup_path"
    echo "$backup_path"
}

backup_item() {
    local src="$1"
    local backup_path="$2"
    local scope="$3"
    local root="$4"
    local relative_path="${src#$root/}"
    local dest="$backup_path/$scope/$relative_path"

    mkdir -p "$(dirname "$dest")"
    if [ -L "$src" ]; then
        echo "$(readlink "$src")" > "$dest.symlink"
    elif [ -d "$src" ]; then
        cp -R "$src" "$dest"
    else
        cp "$src" "$dest"
    fi
}

backup_home_item() {
    backup_item "$1" "$2" "home" "$HOME"
}

backup_repo_item() {
    backup_item "$1" "$2" "repo" "$CONFIG_DIR"
}

write_manifest() {
    local backup_path="$1"
    local operation="$2"
    shift 2
    local items=("$@")

    cat > "$backup_path/manifest.json" << EOF
{
    "timestamp": "$(date -u +%Y-%m-%dT%H:%M:%SZ)",
    "operation": "$operation",
    "items": [$(printf '"%s",' "${items[@]}" | sed 's/,$//')],
    "user": "$USER",
    "hostname": "$(hostname)"
}
EOF
}

validate_skill() {
    local skill_path="$1"
    local skill_name
    skill_name="$(basename "$skill_path")"
    local skill_md="$skill_path/SKILL.md"
    local errors=()

    if [ ! -f "$skill_md" ]; then
        errors+=("Missing SKILL.md")
    else
        local delimiter_count
        delimiter_count=$(grep -c "^---$" "$skill_md" 2>/dev/null) || delimiter_count=0

        if [ "$delimiter_count" -lt 2 ]; then
            errors+=("Missing frontmatter (--- markers)")
        else
            local frontmatter
            frontmatter=$(sed -n '/^---$/,/^---$/p' "$skill_md" | sed '1d;$d')

            if [ -z "$frontmatter" ]; then
                errors+=("Empty frontmatter")
            else
                if ! echo "$frontmatter" | grep -q "^name:"; then
                    errors+=("Missing 'name' in frontmatter")
                fi
                if ! echo "$frontmatter" | grep -q "^description:"; then
                    errors+=("Missing 'description' in frontmatter")
                fi
            fi
        fi
    fi

    if [ "${#errors[@]}" -gt 0 ]; then
        echo -e "${YELLOW}⚠${RESET} $skill_name: ${errors[*]}"
        return 1
    fi

    return 0
}

validate_agent() {
    local agent_path="$1"
    local agent_name
    agent_name="$(basename "$agent_path")"
    local errors=()

    if [ ! -f "$agent_path" ]; then
        errors+=("Missing agent file")
    else
        local frontmatter=""
        if [ "$(head -n 1 "$agent_path")" = "---" ]; then
            frontmatter=$(awk 'NR == 1 { next } /^---$/ { exit } { print }' "$agent_path")
        fi
        if [ -z "$frontmatter" ]; then
            errors+=("Missing frontmatter (--- markers)")
        else
            echo "$frontmatter" | grep -q '^name:' || errors+=("Missing 'name'")
            echo "$frontmatter" | grep -q '^description:' || errors+=("Missing 'description'")
            if echo "$frontmatter" | grep -q '^model:'; then
                echo "$frontmatter" | grep -Eq '^model: *(haiku|sonnet|opus|inherit|claude-[a-z0-9.-]+) *$' \
                    || errors+=("Invalid 'model' (use haiku, sonnet, opus, inherit, or a claude-* id)")
            fi
        fi
    fi

    if [ "${#errors[@]}" -gt 0 ]; then
        echo -e "${YELLOW}⚠${RESET} $agent_name: ${errors[*]}"
        return 1
    fi

    return 0
}

show_dir_status() {
    local repo_dir="$1"
    local local_dir="$2"
    local seen_items="|"
    local item item_name local_path target

    if [ -d "$repo_dir" ]; then
        for item in "$repo_dir"/*/; do
            [ -d "$item" ] || continue
            item_name="$(basename "$item")"
            seen_items="${seen_items}${item_name}|"
            local_path="$local_dir/$item_name"
            local item_target
            item_target="$(normalize_path "$item")"

            if [ -L "$local_path" ]; then
                target="$(normalize_path "$(readlink "$local_path")")"
                if [[ "$target" == "$item_target" ]]; then
                    echo -e "  ${GREEN}✓${RESET} $item_name (synced)"
                else
                    echo -e "  ${BLUE}→${RESET} $item_name (symlink to elsewhere)"
                fi
            elif [ -d "$local_path" ]; then
                echo -e "  ${YELLOW}⚠${RESET} $item_name (exists in both - local copy)"
            else
                echo -e "  ${RESET}○${RESET} $item_name (repo only)"
            fi
        done
    fi

    if [ -d "$local_dir" ]; then
        for item in "$local_dir"/*/; do
            [ -d "$item" ] || continue
            item_name="$(basename "$item")"
            case "$seen_items" in
                *"|$item_name|"*) continue ;;
            esac

            if [ -L "$item" ]; then
                target="$(normalize_path "$(readlink "$item")")"
                if [[ "$target" == "$(normalize_path "$repo_dir/$item_name")" ]]; then
                    echo -e "  ${GREEN}✓${RESET} $item_name (synced)"
                else
                    echo -e "  ${BLUE}→${RESET} $item_name (symlink to elsewhere)"
                fi
            else
                echo -e "  ${RESET}○${RESET} $item_name (local only)"
            fi
        done
    fi
}

show_file_status() {
    local repo_dir="$1"
    local local_dir="$2"
    local extension="$3"
    local seen_items="|"
    local item item_name local_path target

    if [ -d "$repo_dir" ]; then
        for item in "$repo_dir"/*."$extension"; do
            [ -f "$item" ] || continue
            item_name="$(basename "$item")"
            seen_items="${seen_items}${item_name}|"
            local_path="$local_dir/$item_name"

            if [ -L "$local_path" ]; then
                target="$(readlink "$local_path")"
                if [[ "$target" == "$item" ]]; then
                    echo -e "  ${GREEN}✓${RESET} $item_name (synced)"
                else
                    echo -e "  ${BLUE}→${RESET} $item_name (symlink to elsewhere)"
                fi
            elif [ -f "$local_path" ]; then
                echo -e "  ${YELLOW}⚠${RESET} $item_name (exists in both - local copy)"
            else
                echo -e "  ${RESET}○${RESET} $item_name (repo only)"
            fi
        done
    fi

    if [ -d "$local_dir" ]; then
        for item in "$local_dir"/*."$extension"; do
            [ -e "$item" ] || continue
            item_name="$(basename "$item")"
            case "$seen_items" in
                *"|$item_name|"*) continue ;;
            esac

            if [ -L "$item" ]; then
                target="$(normalize_path "$(readlink "$item")")"
                if [[ "$target" == "$(normalize_path "$repo_dir/$item_name")" ]]; then
                    echo -e "  ${GREEN}✓${RESET} $item_name (synced)"
                else
                    echo -e "  ${BLUE}→${RESET} $item_name (symlink to elsewhere)"
                fi
            else
                echo -e "  ${RESET}○${RESET} $item_name (local only)"
            fi
        done
    fi
}

show_status() {
    echo -e "${BOLD}Claude Sync Status${RESET}"
    echo "=================="
    echo ""

    echo -e "${BOLD}Skills (~/.claude/skills):${RESET}"
    if [ -d "$REPO_SKILL_DIR" ] || [ -d "$SKILL_HOME" ]; then
        show_dir_status "$REPO_SKILL_DIR" "$SKILL_HOME"
    else
        echo "  (none)"
    fi
    echo ""

    echo -e "${BOLD}Subagents (~/.claude/agents):${RESET}"
    if [ -d "$REPO_AGENT_DIR" ] || [ -d "$AGENT_HOME" ]; then
        show_file_status "$REPO_AGENT_DIR" "$AGENT_HOME" "md"
    else
        echo "  (none)"
    fi
    echo ""

    echo "Legend: ✓ synced | ○ local only | ⚠ conflict | → external"
    echo ""
    echo "Usage:"
    echo "  ./sync.sh add <type> <name>     Add a local item to repo"
    echo "  ./sync.sh remove <type> <name>  Remove an item from repo (keeps local)"
    echo "  ./sync.sh delete <type> <name>  Remove item from repo and local copy"
    echo "  ./sync.sh pull                  Pull latest and reinstall"
    echo "  ./sync.sh push                  Commit and push changes"
    echo "  ./sync.sh undo                  Restore from last backup"
    echo "  ./sync.sh validate              Validate all skills"
    echo "  ./sync.sh backups               List available backups"
    echo ""
    echo "Types: skill, agent, subagent"
}

add_skill() {
    local name="$1"
    local src="$SKILL_HOME/$name"
    local dest="$REPO_SKILL_DIR/$name"

    if [ ! -e "$src" ]; then
        echo "Error: Skill '$name' not found in ~/.claude/skills"
        exit 1
    fi

    if [ -L "$src" ] && [[ "$(normalize_path "$(readlink "$src")")" == "$(normalize_path "$dest")" ]]; then
        echo "Error: '$name' is already synced"
        exit 1
    fi

    if [ -e "$dest" ]; then
        echo "Error: Skill '$name' already exists in repo at $dest"
        exit 1
    fi

    if ! validate_skill "$src"; then
        echo ""
        read -p "Add anyway? [y/N]: " confirm
        if [[ ! "$confirm" =~ ^[Yy]$ ]]; then
            echo "Aborted."
            exit 1
        fi
    fi

    if $DRY_RUN; then
        echo -e "${BLUE}[dry-run]${RESET} Would copy $src to $dest"
        echo -e "${BLUE}[dry-run]${RESET} Would create symlink $src -> $dest"
        return
    fi

    echo "Adding skill '$name' to repo..."

    local backup_path
    backup_path=$(create_backup)
    backup_home_item "$src" "$backup_path"
    write_manifest "$backup_path" "add-skill" "$name"

    mkdir -p "$REPO_SKILL_DIR"
    cp -R "$src" "$dest"
    rm -rf "$src"
    ln -s "$dest" "$src"

    echo -e "${GREEN}✓${RESET} Skill '$name' added and symlinked"
    echo -e "${BLUE}Backup saved:${RESET} $backup_path"
    echo "  Run: ./sync.sh push"
}

add_agent() {
    local name="$1"
    local filename="$name"
    [[ "$filename" == *.md ]] || filename="$name.md"
    local src="$AGENT_HOME/$filename"
    local dest="$REPO_AGENT_DIR/$filename"

    if [ ! -f "$src" ]; then
        echo "Error: Subagent '$name' not found in ~/.claude/agents"
        exit 1
    fi

    if [ -L "$src" ] && [[ "$(normalize_path "$(readlink "$src")")" == "$(normalize_path "$dest")" ]]; then
        echo "Error: '$name' is already synced"
        exit 1
    fi

    if [ -e "$dest" ]; then
        echo "Error: Subagent '$name' already exists in repo at $dest"
        exit 1
    fi

    if $DRY_RUN; then
        echo -e "${BLUE}[dry-run]${RESET} Would copy $src to $dest"
        echo -e "${BLUE}[dry-run]${RESET} Would create symlink $src -> $dest"
        return
    fi

    echo "Adding subagent '$name' to repo..."

    local backup_path
    backup_path=$(create_backup)
    backup_home_item "$src" "$backup_path"
    write_manifest "$backup_path" "add-agent" "$filename"

    mkdir -p "$REPO_AGENT_DIR"
    cp "$src" "$dest"
    rm -f "$src"
    ln -s "$dest" "$src"

    echo -e "${GREEN}✓${RESET} Subagent '$name' added and symlinked"
    echo -e "${BLUE}Backup saved:${RESET} $backup_path"
    echo "  Run: ./sync.sh push"
}

remove_skill() {
    local name="$1"
    local repo_skill="$REPO_SKILL_DIR/$name"
    local local_skill="$SKILL_HOME/$name"

    if [ ! -d "$repo_skill" ]; then
        echo "Error: Skill '$name' not in repo"
        exit 1
    fi

    if $DRY_RUN; then
        echo -e "${BLUE}[dry-run]${RESET} Would remove repo skill $repo_skill"
        if [ -e "$local_skill" ] || [ -L "$local_skill" ]; then
            echo -e "${BLUE}[dry-run]${RESET} Would keep local copy at $local_skill"
        fi
        return
    fi

    echo "Removing skill '$name' from repo..."

    local backup_path
    backup_path=$(create_backup)
    backup_repo_item "$repo_skill" "$backup_path"
    if [ -e "$local_skill" ] || [ -L "$local_skill" ]; then
        backup_home_item "$local_skill" "$backup_path"
    fi
    write_manifest "$backup_path" "remove-skill" "$name"

    if [ -L "$local_skill" ] && [[ "$(normalize_path "$(readlink "$local_skill")")" == "$(normalize_path "$repo_skill")" ]]; then
        rm "$local_skill"
        cp -R "$repo_skill" "$local_skill"
    fi

    rm -rf "$repo_skill"

    echo -e "${GREEN}✓${RESET} Skill '$name' removed from repo (kept local if it existed)"
    echo -e "${BLUE}Backup saved:${RESET} $backup_path"
    echo "  Run: ./sync.sh push"
}

remove_agent() {
    local name="$1"
    local filename="$name"
    [[ "$filename" == *.md ]] || filename="$name.md"
    local repo_agent="$REPO_AGENT_DIR/$filename"
    local local_agent="$AGENT_HOME/$filename"

    if [ ! -f "$repo_agent" ]; then
        echo "Error: Subagent '$name' not in repo"
        exit 1
    fi

    if $DRY_RUN; then
        echo -e "${BLUE}[dry-run]${RESET} Would remove repo subagent $repo_agent"
        if [ -e "$local_agent" ] || [ -L "$local_agent" ]; then
            echo -e "${BLUE}[dry-run]${RESET} Would keep local copy at $local_agent"
        fi
        return
    fi

    echo "Removing subagent '$name' from repo..."

    local backup_path
    backup_path=$(create_backup)
    backup_repo_item "$repo_agent" "$backup_path"
    if [ -e "$local_agent" ] || [ -L "$local_agent" ]; then
        backup_home_item "$local_agent" "$backup_path"
    fi
    write_manifest "$backup_path" "remove-agent" "$filename"

    if [ -L "$local_agent" ] && [[ "$(readlink "$local_agent")" == "$repo_agent" ]]; then
        rm "$local_agent"
        cp "$repo_agent" "$local_agent"
    fi

    rm -f "$repo_agent"

    echo -e "${GREEN}✓${RESET} Subagent '$name' removed from repo (kept local if it existed)"
    echo -e "${BLUE}Backup saved:${RESET} $backup_path"
    echo "  Run: ./sync.sh push"
}

delete_skill() {
    local name="$1"
    local repo_skill="$REPO_SKILL_DIR/$name"
    local local_skill="$SKILL_HOME/$name"
    local found=false

    if [ -d "$repo_skill" ] || [ -e "$local_skill" ] || [ -L "$local_skill" ]; then
        found=true
    fi

    if ! $found; then
        echo "Error: Skill '$name' not found in repo or local skills"
        exit 1
    fi

    if $DRY_RUN; then
        if [ -d "$repo_skill" ]; then
            echo -e "${BLUE}[dry-run]${RESET} Would delete repo skill $repo_skill"
        fi
        if [ -e "$local_skill" ] || [ -L "$local_skill" ]; then
            echo -e "${BLUE}[dry-run]${RESET} Would delete local skill $local_skill"
        fi
        return
    fi

    echo "Deleting skill '$name' from repo and local skills..."

    local backup_path
    backup_path=$(create_backup)
    if [ -d "$repo_skill" ]; then
        backup_repo_item "$repo_skill" "$backup_path"
    fi
    if [ -e "$local_skill" ] || [ -L "$local_skill" ]; then
        backup_home_item "$local_skill" "$backup_path"
    fi
    write_manifest "$backup_path" "delete-skill" "$name"

    [ -e "$local_skill" ] || [ -L "$local_skill" ] && rm -rf "$local_skill"
    [ -d "$repo_skill" ] && rm -rf "$repo_skill"

    echo -e "${GREEN}✓${RESET} Skill '$name' deleted from repo and local skill directories"
    echo -e "${BLUE}Backup saved:${RESET} $backup_path"
    echo "  Run: ./sync.sh push"
}

delete_agent() {
    local name="$1"
    local filename="$name"
    [[ "$filename" == *.md ]] || filename="$name.md"
    local repo_agent="$REPO_AGENT_DIR/$filename"
    local local_agent="$AGENT_HOME/$filename"
    local found=false

    if [ -f "$repo_agent" ] || [ -e "$local_agent" ] || [ -L "$local_agent" ]; then
        found=true
    fi

    if ! $found; then
        echo "Error: Subagent '$name' not found in repo or local agents"
        exit 1
    fi

    if $DRY_RUN; then
        if [ -f "$repo_agent" ]; then
            echo -e "${BLUE}[dry-run]${RESET} Would delete repo subagent $repo_agent"
        fi
        if [ -e "$local_agent" ] || [ -L "$local_agent" ]; then
            echo -e "${BLUE}[dry-run]${RESET} Would delete local subagent $local_agent"
        fi
        return
    fi

    echo "Deleting subagent '$name' from repo and local agents..."

    local backup_path
    backup_path=$(create_backup)
    if [ -f "$repo_agent" ]; then
        backup_repo_item "$repo_agent" "$backup_path"
    fi
    if [ -e "$local_agent" ] || [ -L "$local_agent" ]; then
        backup_home_item "$local_agent" "$backup_path"
    fi
    write_manifest "$backup_path" "delete-agent" "$filename"

    [ -e "$local_agent" ] || [ -L "$local_agent" ] && rm -f "$local_agent"
    [ -f "$repo_agent" ] && rm -f "$repo_agent"

    echo -e "${GREEN}✓${RESET} Subagent '$name' deleted from repo and local agent directories"
    echo -e "${BLUE}Backup saved:${RESET} $backup_path"
    echo "  Run: ./sync.sh push"
}

pull_changes() {
    if $DRY_RUN; then
        echo -e "${BLUE}[dry-run]${RESET} Would run: git pull"
        echo -e "${BLUE}[dry-run]${RESET} Would run: ./install.sh"
        return
    fi

    echo "Pulling latest changes..."
    cd "$CONFIG_DIR"
    git pull
    echo ""
    echo "Re-running install..."
    ./install.sh
}

push_changes() {
    cd "$CONFIG_DIR"

    if git diff --cached --quiet; then
        echo "No staged changes. Stage only the intended paths with git add <paths>."
        exit 0
    fi

    if ! git diff --quiet; then
        echo "Unstaged changes exist. Stage only the intended paths before pushing."
        git status --short
        exit 1
    fi

    if $DRY_RUN; then
        echo -e "${BLUE}[dry-run]${RESET} Would validate, sign, and push staged changes:"
        git diff --cached --stat
        return
    fi

    validate_all_skills
    git diff --cached --check

    read -p "Commit message (or Ctrl+C to cancel): " msg
    [ -n "$msg" ] || { echo "Commit message is required."; exit 1; }
    git commit -S -m "$msg"
    git push

    echo -e "${GREEN}✓${RESET} Pushed to remote"
}

list_backups() {
    echo -e "${BOLD}Available Backups${RESET}"
    echo "================="
    echo ""

    if [ ! -d "$BACKUP_DIR" ] || [ -z "$(ls -A "$BACKUP_DIR" 2>/dev/null)" ]; then
        echo "No backups found."
        return
    fi

    local backup
    for backup in "$BACKUP_DIR"/*/; do
        [ -d "$backup" ] || continue
        local name
        name="$(basename "$backup")"
        local manifest="$backup/manifest.json"

        if [ -f "$manifest" ]; then
            local operation
            operation=$(grep -o '"operation": *"[^"]*"' "$manifest" | cut -d'"' -f4)
            local timestamp
            timestamp=$(grep -o '"timestamp": *"[^"]*"' "$manifest" | cut -d'"' -f4)
            echo -e "  ${GREEN}$name${RESET}"
            echo "    Operation: $operation"
            echo "    Time: $timestamp"
        else
            echo -e "  ${YELLOW}$name${RESET} (no manifest)"
        fi
    done
    echo ""
}

restore_tree() {
    local root="$1"
    local dest_base="$2"
    local path="$3"
    local relative=""

    if [[ "$path" != "$root" ]]; then
        relative="${path#"$root"/}"
    fi

    local dest="$dest_base"
    if [ -n "$relative" ]; then
        dest="$dest_base/$relative"
    fi

    if [ -d "$path" ]; then
        if [ -L "$dest" ]; then
            rm -f "$dest"
        fi
        mkdir -p "$dest"
        local child
        for child in "$path"/.[!.]* "$path"/..?* "$path"/*; do
            [ -e "$child" ] || continue
            restore_tree "$root" "$dest_base" "$child"
        done
        return
    fi

    mkdir -p "$(dirname "$dest")"
    if [[ "$path" == *.symlink ]]; then
        local target
        target="$(cat "$path")"
        local real_dest="${dest%.symlink}"
        rm -rf "$real_dest"
        ln -s "$target" "$real_dest"
    else
        rm -rf "$dest"
        cp "$path" "$dest"
    fi
}

undo_last() {
    if [ ! -d "$BACKUP_DIR" ] || [ -z "$(ls -A "$BACKUP_DIR" 2>/dev/null)" ]; then
        echo "No backups found."
        exit 1
    fi

    local latest
    latest="$(ls -1t "$BACKUP_DIR" | head -1)"
    local backup_path="$BACKUP_DIR/$latest"
    local manifest="$backup_path/manifest.json"

    if [ ! -f "$manifest" ]; then
        echo "Error: No manifest found in $backup_path"
        exit 1
    fi

    local operation timestamp
    operation=$(grep -o '"operation": *"[^"]*"' "$manifest" | cut -d'"' -f4)
    timestamp=$(grep -o '"timestamp": *"[^"]*"' "$manifest" | cut -d'"' -f4)
    local add_repo_path=""
    local add_local_path=""
    local add_preserved_path=""

    if [[ "$operation" == "add-skill" || "$operation" == "add-agent" ]]; then
        local added_item
        added_item=$(jq -er 'if (.items | type) == "array" and (.items | length) == 1 and (.items[0] | type) == "string"
            then .items[0] else error("add backup manifest must contain exactly one item") end' "$manifest") || exit 1
        local item_pattern='^[a-z0-9][a-z0-9-]*$'
        [[ "$operation" == "add-agent" ]] && item_pattern='^[a-z0-9][a-z0-9-]*\.md$'
        if [[ ! "$added_item" =~ $item_pattern ]]; then
            echo "Error: add backup manifest contains an invalid item" >&2
            exit 1
        fi
        if [[ "$operation" == "add-skill" ]]; then
            add_repo_path="$REPO_SKILL_DIR/$added_item"
            add_local_path="$SKILL_HOME/$added_item"
            add_preserved_path="$backup_path/added-current/skills/$added_item"
        else
            add_repo_path="$REPO_AGENT_DIR/$added_item"
            add_local_path="$AGENT_HOME/$added_item"
            add_preserved_path="$backup_path/added-current/agents/$added_item"
        fi
        if [ ! -L "$add_local_path" ] || [[ "$(normalize_path "$(readlink "$add_local_path")")" != "$(normalize_path "$add_repo_path")" ]]; then
            echo "Error: Cannot safely undo $operation because the installed symlink changed" >&2
            exit 1
        fi
    fi

    echo -e "${BOLD}Last backup:${RESET} $latest"
    echo "  Operation: $operation"
    echo "  Time: $timestamp"
    echo ""
    echo "Contents:"
    ls -la "$backup_path" | tail -n +2 | sed 's/^/  /'
    echo ""

    if $DRY_RUN; then
        echo -e "${BLUE}[dry-run]${RESET} Would restore files from $backup_path"
        return
    fi

    read -p "Restore from this backup? [y/N]: " confirm
    if [[ ! "$confirm" =~ ^[Yy]$ ]]; then
        echo "Aborted."
        exit 0
    fi

    echo ""
    echo "Restoring..."

    if [ -n "$add_repo_path" ]; then
        if [ ! -L "$add_local_path" ] || [[ "$(normalize_path "$(readlink "$add_local_path")")" != "$(normalize_path "$add_repo_path")" ]]; then
            echo "Error: Cannot safely undo $operation because the installed symlink changed" >&2
            exit 1
        fi
        if [[ "$operation" == "add-skill" && ! -d "$add_repo_path" ]] || [[ "$operation" == "add-agent" && ! -f "$add_repo_path" ]]; then
            echo "Error: Cannot safely undo $operation because the added repository item is missing" >&2
            exit 1
        fi
        mkdir -p "$(dirname "$add_preserved_path")"
        mv "$add_repo_path" "$add_preserved_path"
    fi

    if [ -d "$backup_path/home" ] || [ -d "$backup_path/repo" ]; then
        [ -d "$backup_path/home" ] && restore_tree "$backup_path/home" "$HOME" "$backup_path/home"
        [ -d "$backup_path/repo" ] && restore_tree "$backup_path/repo" "$CONFIG_DIR" "$backup_path/repo"
    else
        local backup_root
        for backup_root in "$backup_path"/*; do
            [ -e "$backup_root" ] || continue
            case "$(basename "$backup_root")" in
                .claude|.codex)
                    restore_tree "$backup_path" "$HOME" "$backup_root"
                    ;;
                skills)
                    restore_tree "$backup_path" "$CONFIG_DIR" "$backup_root"
                    ;;
            esac
        done
    fi

    mv "$backup_path" "$backup_path.restored"

    echo ""
    echo -e "${GREEN}Done!${RESET} Backup restored and marked as used."
    echo "The backup is now at: $backup_path.restored"
    if [ -n "$add_preserved_path" ]; then
        echo "The removed repository item was preserved inside that backup under added-current/."
    fi
}

validate_all_skills() {
    echo -e "${BOLD}Validating Skills and Agents${RESET}"
    echo "==========================="
    echo ""

    local has_errors=false
    local checked_skills=0
    local checked_agents=0
    local seen=" "
    local seen_agents=" "

    if [ -d "$REPO_SKILL_DIR" ]; then
        local skill
        for skill in "$REPO_SKILL_DIR"/*/; do
            [ -d "$skill" ] || continue
            local skill_name
            skill_name="$(basename "$skill")"
            seen="$seen$skill_name "
            ((checked_skills++)) || true
            if ! validate_skill "$skill"; then
                has_errors=true
            else
                echo -e "${GREEN}✓${RESET} $skill_name"
            fi
        done
    fi

    if [ -d "$SKILL_HOME" ]; then
        local skill
        for skill in "$SKILL_HOME"/*/; do
            [ -d "$skill" ] || continue
            local skill_name
            skill_name="$(basename "$skill")"
            # Claude-managed container for claude.ai synced skills, not a skill itself
            [ "$skill_name" = "synced" ] && continue
            case "$seen" in
                *" $skill_name "*) [ ! -L "$skill" ] && continue ;;
            esac
            if is_repo_symlink "$skill"; then
                continue
            fi
            ((checked_skills++)) || true
            if ! validate_skill "$skill"; then
                has_errors=true
            else
                echo -e "${GREEN}✓${RESET} $skill_name (local)"
            fi
        done
    fi

    if [ -d "$REPO_AGENT_DIR" ]; then
        local agent
        for agent in "$REPO_AGENT_DIR"/*.md; do
            [ -f "$agent" ] || continue
            local agent_name
            agent_name="$(basename "$agent")"
            seen_agents="$seen_agents$agent_name "
            ((checked_agents++)) || true
            if ! validate_agent "$agent"; then
                has_errors=true
            else
                echo -e "${GREEN}✓${RESET} $agent_name"
            fi
        done
    fi

    if [ -d "$AGENT_HOME" ]; then
        local agent
        for agent in "$AGENT_HOME"/*.md; do
            [ -f "$agent" ] || continue
            local agent_name
            agent_name="$(basename "$agent")"
            case "$seen_agents" in
                *" $agent_name "*) [ ! -L "$agent" ] && continue ;;
            esac
            if is_repo_symlink "$agent"; then
                continue
            fi
            ((checked_agents++)) || true
            if ! validate_agent "$agent"; then
                has_errors=true
            else
                echo -e "${GREEN}✓${RESET} $agent_name (local)"
            fi
        done
    fi

    echo ""
    if [ "$checked_skills" -eq 0 ] && [ "$checked_agents" -eq 0 ]; then
        echo "No skills or agents found to validate."
    elif $has_errors; then
        echo -e "${YELLOW}Some items have issues.${RESET}"
        echo "Skills should have a SKILL.md with frontmatter containing 'name' and 'description'."
        echo "Agents should define 'name' and 'description'."
        exit 1
    else
        echo -e "${GREEN}All $checked_skills skills and $checked_agents agents valid.${RESET}"
    fi
}

args=("$@")
for i in "${!args[@]}"; do
    if [[ "${args[$i]}" == "-n" || "${args[$i]}" == "--dry-run" ]]; then
        DRY_RUN=true
        unset 'args[$i]'
    fi
done
if [ "${#args[@]}" -gt 0 ]; then
    set -- "${args[@]}"
else
    set --
fi

case "${1:-}" in
    add)
        type="${2:-}"
        name="${3:-}"
        [ -z "$type" ] || [ -z "$name" ] && { echo "Usage: ./sync.sh add <type> <name>"; echo "Types: skill, agent, subagent"; exit 1; }
        validate_item_name "$name"
        case "$type" in
            skill) add_skill "$name" ;;
            agent|subagent) add_agent "$name" ;;
            *) echo "Unknown type: $type (use: skill, agent, subagent)"; exit 1 ;;
        esac
        ;;
    remove)
        type="${2:-}"
        name="${3:-}"
        [ -z "$type" ] || [ -z "$name" ] && { echo "Usage: ./sync.sh remove <type> <name>"; echo "Types: skill, agent, subagent"; exit 1; }
        validate_item_name "$name"
        case "$type" in
            skill) remove_skill "$name" ;;
            agent|subagent) remove_agent "$name" ;;
            *) echo "Unknown type: $type (use: skill, agent, subagent)"; exit 1 ;;
        esac
        ;;
    delete)
        type="${2:-}"
        name="${3:-}"
        [ -z "$type" ] || [ -z "$name" ] && { echo "Usage: ./sync.sh delete <type> <name>"; echo "Types: skill, agent, subagent"; exit 1; }
        validate_item_name "$name"
        case "$type" in
            skill) delete_skill "$name" ;;
            agent|subagent) delete_agent "$name" ;;
            *) echo "Unknown type: $type (use: skill, agent, subagent)"; exit 1 ;;
        esac
        ;;
    pull)
        pull_changes
        ;;
    push)
        push_changes
        ;;
    undo)
        undo_last
        ;;
    backups)
        list_backups
        ;;
    validate)
        validate_all_skills
        ;;
    -h|--help)
        show_status
        ;;
    *)
        show_status
        ;;
esac
