# Parallel

For work that splits into independent tasks: several bugs, a feature's disjoint slices, a sweep across modules.

**Outcome:** every task done and verified, without two agents ever touching the same file.

**Lead mode** (opt-in): for audits, bug hunts, perf problems, release prep, or large refactors that need the whole codebase read first. Never start it on your own: suggest it in one line and wait for approval. In lead mode this session writes no code: it audits, plans, delegates, verifies, and reports, and runs on the most capable model. Add steps 0 and 1b below.

0. **Audit (lead mode).** Read the project and list every problem with evidence: `file:line`, command output, measurements. Reproduce each bug before proposing a fix (`principles/fix-root-causes.md`).
1. Split the work into tasks whose files don't overlap. Tasks that share a file, a migration, or a schema run one after another, never side by side (`principles/sequence-verifiable-units.md`).
   - **1b (lead mode).** Size the number of workers to the project. Show the plan (each task's scope, allowed files, acceptance criteria) and wait for approval.
2. Write one brief per task: the goal, the playbook to follow, the files it may touch, the acceptance criteria, "run the type check and tests when done", and what to return.
3. Run them:
   - Claude Code: one `worker` per task with `isolation: "worktree"`, in the background, all in one message; or separate sessions with `start_session` when the user wants to watch each worker.
   - Codex: one session per task, each in its own worktree: `git worktree add ../<repo>-<task> -b <task-branch>`.
4. Read every report. Unblock a stuck worker with a message. Send defective work back to the same worker with what's wrong; in lead mode, never fix it yourself.
5. Don't trust reports: review every diff yourself and run the full type check, tests, and measurements (`principles/prove-it-works.md`). Mark each task accepted, returned, or rejected. Bring accepted branches into the current branch one at a time, running the fast check after each. Remove the worktrees when done.

**Stop:** when every task is accepted or rejected with evidence and the checks pass. Commit, push, or add dependencies only with approval.

**Reply:** one line per task with its result and evidence, the conflicts you resolved, what is left, and what you recommend next.
