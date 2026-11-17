# Never lower the bar

**When:** a check, test, linter, or type checker fails.

- Fix the code, not the check. Without the user's approval, never add a suppression (`@ts-ignore`, `eslint-disable`, `# noqa`, `@phpstan-ignore`, a baseline entry), skip or delete a test, loosen an assertion or expected value, leave a stub or empty `catch`, or lower a threshold, rule, or analysis level.
- Exception: a one-line suppression of a style rule with its reason on the line (`code-says-what.md`). Correctness and safety rules are never suppressed.
- A wrong check gets fixed in its own visible change. Tightening needs no permission; loosening always does.

**Bad:** `// @ts-ignore` on the type error and `.skip` on the test that caught it. **Good:** the input type fixed and the test passing unchanged.
