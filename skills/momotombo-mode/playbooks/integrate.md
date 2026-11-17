# Integrate

Run only when the user asks to commit, open a PR, or merge.

**Outcome:** reviewed, verified work committed or proposed, with no comments or debt added.

1. Run the project's full check command in the background. It must pass.
2. Run the reviews in parallel, on the diff against the base branch:
   - `reviewer`, always.
   - `security-reviewer`, only if auth, payments, personal data, uploads, or integrations changed.
   - The `no-comments` skill, always.
3. Fix the **Blocking** findings and the accepted comment deletions yourself. List the rest for the user; don't loop. Re-run a reviewer at most once, only if a blocking fix changed logic.
4. Commit with `/commit` (Claude) or `git-ops` (Codex): small, ordered Conventional Commits. Open the PR with `/open-pr` when asked. Push and merge only with the user's approval.

**Stop:** when the commits or PR exist and the checks are green.

**Reply:** the commits or PR link, the check results, what the reviews found and what you fixed, and what is left for the user.
