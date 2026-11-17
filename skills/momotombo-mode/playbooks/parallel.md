# Parallel

For work that splits into independent tasks: several bugs, a feature's disjoint slices, a sweep across modules.

**Outcome:** every task done and verified, without two agents ever touching the same file.

1. Split the work into tasks whose files don't overlap. Tasks that share a file, a migration, or a schema run one after another, never side by side (`principles/sequence-verifiable-units.md`).
2. Write one brief per task: the goal, the playbook to follow, the files it may touch, the check that proves it, and what to return.
3. Run them:
   - Claude Code: spawn one `worker` per task with `isolation: "worktree"`, in the background, all in one message.
   - Codex: one session per task, each in its own worktree: `git worktree add ../<repo>-<task> -b <task-branch>`.
4. Review every returned diff yourself, as if you wrote it. Reject and re-brief a task that left its files or skipped verification.
5. Bring the branches into the current branch locally, one at a time, running the fast check after each. Remove the worktrees when done.

**Stop:** when every task is merged locally and the checks pass. Push only with approval.

**Reply:** one line per task with its result and evidence, the conflicts you resolved, and anything left open.
