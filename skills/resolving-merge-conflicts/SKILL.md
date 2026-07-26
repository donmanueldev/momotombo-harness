---
name: resolving-merge-conflicts
description: Resolve in-progress merge/rebase conflicts by intent from each side.
---

# Resolving Merge Conflicts

Use this when `git merge` or `git rebase` leaves conflicts that block progress.

- Open conflicts with a short diff window and resolve hunk-by-hunk.
- For each hunk, reason from both source sides and preserve intent.
- Keep code style and tests consistent; avoid `--abort` unless explicitly asked.
- Continue the sequence (`continue`) only after conflict resolution is correct.
