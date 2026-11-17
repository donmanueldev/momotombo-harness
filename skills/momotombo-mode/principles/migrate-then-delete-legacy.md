# Migrate callers, then delete the legacy

**When:** introducing a new internal API, pattern, or structure while old callers exist.

- Move every caller to the new path and delete the old one in the same change.
- No compatibility shims, deprecated aliases, or "keep both for now". Two ways to do one thing doubles what everyone reads.
- Use the ecosystem's current, documented patterns, not the deprecated ones a framework still tolerates.
- If the migration is too big for one change, write the lever that does it (`build-the-lever.md`) rather than leaving both paths.

**Bad:** a new `PaymentService` beside the old helper, both still called. **Good:** every call site moved, the helper deleted.
