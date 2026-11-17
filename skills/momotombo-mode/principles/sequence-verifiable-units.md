# Sequence verifiable units

**When:** multi-step work: features with phases, migrations, sweeps, runs of similar edits.

- Break the work into small units that each end in a passing check.
- Check each unit before starting the next, so a failure points at one small change.
- Order them so the sequence proves itself to a reviewer: the failing test before the fix, the data model before the screen that uses it.
- Each unit leaves the system working, never in a broken business state.

**Bad:** 30 files changed, then the first test run. **Good:** model and its test, then the endpoint and its test, then the screen, each green before the next.
