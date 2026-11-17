# Migrate callers, then delete the legacy

**When:** introducing a new API, pattern, structure, or schema while old callers exist.

- Internal code: move every caller to the new path and delete the old one in the same change.
- No compatibility shims, deprecated aliases, or "keep both for now" for internal code. Two ways to do one thing doubles what everyone reads.
- Use the ecosystem's current, documented patterns, not the deprecated ones a framework still tolerates.
- If the migration is too big for one change, write the lever that does it (`build-the-lever.md`) rather than leaving both paths.

**Exception: callers you don't deploy.** A database that running code reads during a deploy, and an API that shipped mobile apps, other services, or webhooks still call, can't switch in one change. Use expand, migrate, contract:

1. **Expand:** add the new column, field, or endpoint beside the old one. Write to both when data must stay consistent.
2. **Migrate:** backfill the data, move every caller you own, and measure who still uses the old path (logs or metrics, `make-it-observable.md`).
3. **Contract:** delete the old path in its own change once no caller uses it, or the oldest supported app version no longer needs it.

Each step deploys on its own and is safe to roll back. Write down when the contract step is due, so the old path doesn't become permanent.

**Bad:** a new `PaymentService` beside the old helper, both still called; or renaming a column in one migration while the old code is still serving requests. **Good:** every internal call site moved and the helper deleted; the column added, backfilled, read from, and the old one dropped in a later release.
