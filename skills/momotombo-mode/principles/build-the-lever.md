# Build the lever

**When:** repetitive edits, migrations, audits, or anything a reviewer should be able to re-run.

- Write the tool that does or proves the work (a codemod, script, generator, query, or check) instead of doing it by hand.
- The tool is the artifact: the reviewer reruns it instead of trusting your edits.
- Prefer the ecosystem's own tools (its codemod, refactoring, or migration tooling) over a custom script.

**Bad:** editing 40 files by hand to rename a prop. **Good:** a codemod that renames it, run once, committed with the change.
