---
name: fix-sentry-issues
description: Triage and fix Sentry issues.
---

# Fix Sentry Issues

## Philosophy

**The Sentry error is not the problem. It's a signal.**

Your goal is not to close the Sentry issue. Your goal is to discover the root cause, understand what's wrong with the application, and fix the underlying defect. Closing the Sentry issue is a side effect of doing that correctly.

Ask **"Why does this fail?"** — not **"How do I make Sentry quiet?"** Never treat log level changes as fixes. A fallback path means degraded user experience; trace why the primary path fails and fix it upstream.

## Anti-patterns (do not do these)

- **Batch-classifying as "expected" without investigation.** Seeing a fallback does NOT mean you understand the failure. Trace the full input path.
- **Treating "has a fallback" as "not a problem."** Why does the primary path fail? Can we prevent it upstream?
- **Combining multiple issues into one PR.** Each has its own root cause. Fix individually (except when investigation proves identical cause).
- **Throwing away error details.** Never remove `error` from `catch (error)` or strip status codes. That data is how you understand failures.
- **Deciding the fix during triage.** Classify as "Investigate" or "Ignore" only. You don't know the fix until investigation is complete.

**Log level downgrade is valid ONLY for genuinely expected states** (e.g., optional column missing, resource deleted) — NOT for failures with fallbacks.

## Phase 1: Discover & Triage

If an authorized Sentry connector is available, discover its tools first and inspect the requested organization, project, issue, and event evidence. Otherwise, ask the user for the issue link or exported evidence; do not infer live Sentry state.

Build a triage table. Action = **Investigate** or **Ignore** only:

| ID | Title | Events | Action | Reason |
|----|-------|--------|--------|--------|
| PROJ-A | Error in save | 14 | Investigate | User-facing save failure |
| PROJ-B | GM_register... | 3 | Ignore | Greasemonkey extension |

**Investigate:** multiple events, degraded user experience, high-volume warnings, recurring on every run.
**Ignore:** browser extension code, `ChunkLoadError` (self-resolving), single-event transients, already fixed.

Do not change an issue's status during triage. Closing, ignoring, or resolving it is an external write and requires explicit user authorization after the root cause or completed fix is evidenced.

## Phase 2: Investigate (one issue at a time)

Work through these steps **in order**. Do not skip or batch issues.

1. **Pull event-level data** — Issue summaries hide details. Use `get_issue_details` and `search_issue_events` with `naturalLanguageQuery: "all events with extra data"`. Extract: URLs, params, stack traces, status codes, timestamps.

2. **Read the failing code path** — Follow the stack trace. Read every relevant file. Understand before proposing changes.

3. **Trace the input path upstream** *(most often skipped, most important)* — What data reaches the failing function? Should it have reached this path at all? Is there a missing filter? Is the input wrong (binary URL, redirect, bad format)? Can we prevent bad inputs upstream?

4. **Reproduce** — Use redacted, safe failing inputs where possible. Verify your understanding without replaying an external mutation or exposing sensitive event data.

5. **Identify root cause** — Why does this input fail? Why does it reach this path? What's the right fix? (e.g., "Filter binary URLs before Firecrawl" — not "suppress the log")

| Pattern | Real Fix |
|---------|----------|
| External API fails on certain URLs | Filter/validate inputs before sending |
| Timeout | Investigate what's slow; adjust timeout or input size |
| DB "invalid json" | Sanitize before insert |
| Stale reference on cron | Detect staleness, auto-clean |

## Phase 3: Fix

Work from the current repository checkout and its documented base branch. Do not switch branches, pull, or create a branch unless the user requests it.

- **Tests first** — Use data from actual Sentry events. Test fails before fix, passes after.
- **Implement** — Fix the root cause, not the symptom. If the fix is primarily a log level change, STOP: did you investigate why it fails, or just suppress?
- **Verify** — Tests pass, lint passes, fix handles actual failing inputs.
- **PR** — Include **Root cause** (upstream reason) and **Fix** (what changed and why it prevents the failure). Resolve in Sentry only after merge and explicit authorization.
