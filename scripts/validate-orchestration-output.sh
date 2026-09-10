#!/usr/bin/env bash
set -euo pipefail

AGENT=""
INPUT=""

usage() {
    echo "Usage: $0 --agent <specifier|planner|orchestrator|reviewer> --input <path-to-json>"
    exit 1
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --agent) AGENT="$2"; shift 2 ;;
        --input) INPUT="$2"; shift 2 ;;
        -h|--help) usage ;;
        *) usage ;;
    esac
done

[[ -n "$AGENT" && -n "$INPUT" ]] || usage
[[ -f "$INPUT" ]] || { echo "input file not found: $INPUT"; exit 1; }

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
python3 "$SCRIPT_DIR/orchestration_contract.py" --agent "$AGENT" --input "$INPUT"
