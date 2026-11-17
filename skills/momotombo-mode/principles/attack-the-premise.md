# Attack the premise

**When:** two or more fixes that share one assumption have failed the same check.

- Stop fixing. Write the shared assumption down in one sentence.
- Test the assumption directly, with a minimal experiment, before writing a third fix that relies on it.
- If it is false, the earlier fixes were aimed at the wrong thing: revert them.

**Bad:** a third cache-busting tweak because "the stale value must come from the cache". **Good:** log where the value is read and learn it comes from a second, uncached source.
