# Encode lessons in structure

**When:** the user corrects you, or you notice the same instruction given a second time.

- Turn the lesson into something that runs: a lint rule, a type, a test, a script, a CI check, or a hook.
- A rule in text is the last resort. Write it as a `docs/` page (`codely-doc-create`) only when no tool can catch it.
- Prove the encoding works: it fails on an example of the mistake and passes on the fix.

**Bad:** adding "don't use X" to `AGENTS.md` for the third time. **Good:** enabling the lint rule that bans X, with a failing example in the commit.
