# Migrate callers, then delete the legacy

**When:** a new API, pattern, structure, or schema while old callers exist.

- Move every caller and delete the old path in the same change. No shims, deprecated aliases, or "keep both for now": two ways to do one thing doubles what everyone reads. Too big for one change? Build the lever (`build-the-lever.md`).
- Use the ecosystem's current, documented patterns, not deprecated ones a framework still tolerates.
- Exception, callers you don't deploy (a database read mid-deploy, an API that shipped mobile apps or other services call): expand (add the new beside the old), migrate (backfill, move your callers, measure who still uses the old path), contract (delete it in a later release). Note when the contract step is due.

**Bad:** a new `PaymentService` beside the old helper, both called; a column renamed while old code still reads it. **Good:** the helper deleted with every call site moved; the column added, backfilled, switched, and the old one dropped next release.
