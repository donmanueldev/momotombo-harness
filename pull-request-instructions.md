# Pull Request Instructions (agent-config)

## PR title and summary

- Title: concise, imperative, English.
- Summary: 2–4 bullets with:
  - objective
  - changed surface
  - risk level

## PR body template

1. **Summary**
   - What problem this PR solves.
2. **What changed**
   - File-level list by group (`AGENTS.md`, `skills`, `prompts`, `docs`).
3. **Validation**
   - Commands executed and results.
4. **Risk / rollback**
   - Risk rating (low/med/high), blast radius, rollback path.
5. **Notes**
   - Any follow-ups, known limitations, or environment caveats.

## Required proof

- Include at least one verification line (for example: `bats tests/`, `git diff --check`, `./sync.sh validate`).
- Mention if evidence is partial (e.g., local-only scope, no full-suite run).

## Review expectations

- Keep language factual and concise.
- Call out trade-offs and why alternatives were not selected.
- Explicitly list dependency/behavior coupling outside this repo if any.
- Link to related notes/rollout context when relevant.

## Merge criteria

- No unrelated files or scope creep.
- No broken instructions or conflicting process docs.
- Validation attached to the PR body (or clearly marked as pending with reason).
