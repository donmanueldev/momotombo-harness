---
name: no-comments
description: Remove the comments a change added and fix the code they were excusing. Spawns the comment-sicko subagent on the diff, then applies its accepted findings. Run before review or commit, or when the user asks to strip comments.
license: MIT
metadata:
  author: momotombodevs
  version: "1.0.0"
  adapted-from: https://github.com/cursor/plugins/tree/main/pstack (MIT)
---

# No comments

**Scope:** the files the user names, otherwise the diff against the base branch (default `main`), including uncommitted changes.

1. Spawn `comment-sicko` with the scope. Don't restate its rules.
2. Check its report against the diff. Reject findings outside the scope, and deletions of comments its keep list protects. Everything else stands.
3. Delete the accepted comments.
4. For each `MUST KILL` symbol, make the code obvious without prose: rename it, extract it, give it a type, or remove the workaround the comment was excusing (`momotombo-mode` principles `code-says-what` and `fix-root-causes`). Stay inside the scope; list larger reshapes as open work.
5. For a comment that guards a constraint ("do not change", "must stay in this order"), encode the constraint as a test, type, or lint rule, then delete the comment. If that's out of scope, delete it and report the constraint as open.
6. Run the checks that cover the changed files.

**Reply:** comments removed (count), `MUST KILL` symbols reshaped, constraints encoded, and what is left open.
