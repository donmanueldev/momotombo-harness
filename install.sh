#!/bin/bash
set -e

CONFIG_DIR="$(cd "$(dirname "$0")" && pwd)"
BACKUP_DIR="$CONFIG_DIR/.backup"
DRY_RUN=false
FORCE=false
CLAUDE_HOME="$HOME/.claude"

# Colors
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
BOLD='\033[1m'
RESET='\033[0m'

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -n|--dry-run)
            DRY_RUN=true
            shift
            ;;
        -f|--force)
            FORCE=true
            shift
            ;;
        -h|--help)
            echo "Usage: ./install.sh [options]"
            echo ""
            echo "Options:"
            echo "  -n, --dry-run  Show what would be done without making changes"
            echo "  -f, --force    Overwrite conflicts without prompting"
            echo "  -h, --help     Show this help message"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

# Create backup with timestamp
create_backup() {
    local timestamp=$(date +%Y%m%d_%H%M%S)
    local backup_path="$BACKUP_DIR/$timestamp"
    mkdir -p "$backup_path"
    echo "$backup_path"
}

# Backup a file or directory
backup_item() {
    local src="$1"
    local backup_path="$2"
    local relative_path="${src#$HOME/}"
    local dest="$backup_path/$relative_path"

    mkdir -p "$(dirname "$dest")"
    if [ -L "$src" ]; then
        # For symlinks, store the target
        echo "$(readlink "$src")" > "$dest.symlink"
    elif [ -d "$src" ]; then
        cp -r "$src" "$dest"
    else
        cp "$src" "$dest"
    fi
}

# Write manifest
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

# Check if item has conflict (exists locally and is not a symlink to our repo)
has_conflict() {
    local path="$1"
    if [ -e "$path" ] && [ ! -L "$path" ]; then
        return 0
    fi
    if [ -L "$path" ]; then
        local target=$(readlink "$path")
        if [[ "$target" != "$CONFIG_DIR"* ]]; then
            return 0
        fi
    fi
    return 1
}

# Handle conflict interactively
handle_conflict() {
    local src="$1"
    local dest="$2"
    local backup_path="$3"
    local item_name="$4"

    if $FORCE; then
        backup_item "$dest" "$backup_path"
        return 0  # Proceed with overwrite
    fi

    echo ""
    echo -e "${YELLOW}Conflict:${RESET} $item_name exists locally and differs from repo"
    echo "  Local: $dest"
    echo "  Repo:  $src"
    echo ""
    echo "Options:"
    echo "  [r] Use repo version (backup local)"
    echo "  [l] Keep local version (skip)"
    echo "  [d] Show diff"
    echo "  [q] Quit"

    while true; do
        read -p "Choice [r/l/d/q]: " choice
        case $choice in
            r|R)
                backup_item "$dest" "$backup_path"
                return 0  # Proceed with overwrite
                ;;
            l|L)
                return 1  # Skip this item
                ;;
            d|D)
                echo ""
                if [ -d "$src" ]; then
                    diff -r "$dest" "$src" 2>/dev/null || true
                else
                    diff "$dest" "$src" 2>/dev/null || true
                fi
                echo ""
                ;;
            q|Q)
                echo "Aborted."
                exit 1
                ;;
            *)
                echo "Invalid choice. Use r, l, d, or q."
                ;;
        esac
    done
}

# Dry run output
dry_run_msg() {
    echo -e "${BLUE}[dry-run]${RESET} $1"
}

# Link one item into ~/.claude, handling conflicts and backups
link_item() {
    local src="$1"
    local dest="$2"
    local label="${dest/#$HOME/~}"

    if $DRY_RUN; then
        if has_conflict "$dest"; then
            dry_run_msg "Would backup and replace $label"
        else
            dry_run_msg "Would link $label"
        fi
        return
    fi

    mkdir -p "$(dirname "$dest")"
    if has_conflict "$dest"; then
        [ -z "$backup_path" ] && backup_path=$(create_backup)
        if handle_conflict "$src" "$dest" "$backup_path" "$label"; then
            backed_up_items+=("${dest#$HOME/}")
            rm -rf "$dest"
            ln -sfn "$src" "$dest"
            echo -e "${GREEN}✓${RESET} $label (replaced, backup saved)"
        else
            echo -e "${YELLOW}○${RESET} $label (kept local)"
        fi
    else
        ln -sfn "$src" "$dest"
        echo -e "${GREEN}✓${RESET} $label"
    fi
}

# Build the Rust CLI tools that skills call through their bin/ symlinks
build_tools() {
    local manifest="$CONFIG_DIR/tools/Cargo.toml"
    [ -f "$manifest" ] || return 0

    if $DRY_RUN; then
        dry_run_msg "Would build Rust tools (cargo build --release)"
        return
    fi
    if ! command -v cargo &>/dev/null; then
        echo -e "${YELLOW}○${RESET} Rust tools skipped: cargo not found (install from https://rustup.rs)"
        return
    fi
    if cargo build --release --quiet --manifest-path "$manifest" --target-dir "$CONFIG_DIR/tools/target"; then
        echo -e "${GREEN}✓${RESET} Rust tools built (tools/target/release)"
    else
        echo -e "${YELLOW}⚠${RESET} Rust tools failed to build; SEO and Lighthouse skills will not run"
    fi
}

# Merge the versioned policy in home/settings.json into ~/.claude/settings.json.
# Repo keys win; permission lists are unioned; machine-specific keys (theme, autoMode, ...) are kept.
merge_settings() {
    local repo_settings="$CONFIG_DIR/home/settings.json"
    local dest="$CLAUDE_HOME/settings.json"
    [ -f "$repo_settings" ] || return 0

    if $DRY_RUN; then
        dry_run_msg "Would merge home/settings.json into ~/.claude/settings.json"
        return
    fi
    if ! command -v jq &>/dev/null; then
        echo -e "${YELLOW}○${RESET} settings.json skipped: jq not found"
        return
    fi

    local current="{}"
    [ -f "$dest" ] && current=$(cat "$dest")
    local merged
    if ! merged=$(jq -n --argjson cur "$current" --slurpfile repo "$repo_settings" '
        $repo[0] as $r
        | ($cur * $r)
        | reduce ("allow", "ask", "deny") as $k (.;
            (($cur.permissions[$k] // []) + ($r.permissions[$k] // []) | unique) as $list
            | if ($list | length) > 0 then .permissions[$k] = $list else . end)
    ' 2>/dev/null); then
        echo -e "${YELLOW}⚠${RESET} ~/.claude/settings.json is not valid JSON; left unchanged"
        return
    fi

    if [ -f "$dest" ] && [ "$(jq -S . <<<"$merged")" = "$(jq -S . "$dest")" ]; then
        echo -e "${GREEN}✓${RESET} ~/.claude/settings.json (up to date)"
        return
    fi
    if [ -f "$dest" ]; then
        [ -z "$backup_path" ] && backup_path=$(create_backup)
        backup_item "$dest" "$backup_path"
        backed_up_items+=("${dest#$HOME/}")
    fi
    printf '%s\n' "$merged" > "$dest"
    echo -e "${GREEN}✓${RESET} ~/.claude/settings.json (merged, backup saved)"
}

# Main installation
main() {
    if $DRY_RUN; then
        echo -e "${BOLD}Dry run - showing what would be done:${RESET}"
        echo ""
    else
        echo "Installing agent config from $CONFIG_DIR"
        echo ""
        mkdir -p "$CLAUDE_HOME"
    fi

    backup_path=""
    backed_up_items=()

    build_tools

    # Skills (directory symlink per skill)
    local skill
    for skill in "$CONFIG_DIR/skills"/*/; do
        [ -d "$skill" ] || continue
        link_item "${skill%/}" "$CLAUDE_HOME/skills/$(basename "$skill")"
    done

    # Subagents (file symlink per agent)
    local agent
    for agent in "$CONFIG_DIR/agents"/*.md; do
        [ -f "$agent" ] || continue
        link_item "$agent" "$CLAUDE_HOME/agents/$(basename "$agent")"
    done

    # Global instructions, rules, hooks, and status line
    if [ -f "$CONFIG_DIR/home/CLAUDE.md" ]; then
        link_item "$CONFIG_DIR/home/CLAUDE.md" "$CLAUDE_HOME/CLAUDE.md"
    fi
    local rule
    for rule in "$CONFIG_DIR/home/rules"/*.md; do
        [ -f "$rule" ] || continue
        link_item "$rule" "$CLAUDE_HOME/rules/$(basename "$rule")"
    done
    local hook
    for hook in "$CONFIG_DIR/home/hooks"/*; do
        [ -f "$hook" ] || continue
        link_item "$hook" "$CLAUDE_HOME/hooks/$(basename "$hook")"
    done
    if [ -f "$CONFIG_DIR/home/statusline.sh" ]; then
        link_item "$CONFIG_DIR/home/statusline.sh" "$CLAUDE_HOME/statusline.sh"
    fi

    merge_settings

    echo ""

    if $DRY_RUN; then
        echo "Run without --dry-run to apply changes."
    else
        if [ -n "$backup_path" ] && [ ${#backed_up_items[@]} -gt 0 ]; then
            write_manifest "$backup_path" "install" "${backed_up_items[@]}"
            echo -e "${BLUE}Backup saved:${RESET} $backup_path"
            echo "Run './sync.sh undo' to restore."
            echo ""
        fi

        echo "Done! Agent config installed."
        echo ""
        echo "Local-only items in ~/.claude/ are preserved."
        echo "Use ./sync.sh to manage synced skills and subagents."
    fi
}

main
