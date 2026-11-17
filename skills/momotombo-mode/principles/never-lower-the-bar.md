# Never lower the bar

**When:** a check, test, linter, or type checker fails and you are tempted to make it quiet instead of making it pass.

- Fix the code, not the check. Getting to green by weakening what measures green is not done.
- These moves lower the bar and are off limits without the user's explicit approval:
  - a new suppression: `@ts-ignore`, `@ts-expect-error`, `eslint-disable`, `# noqa`, `# type: ignore`, `@phpstan-ignore`, a baseline entry;
  - a test made easier: `.skip`, `.only`, `markTestSkipped`, a deleted test, an assertion removed or loosened, an expected value edited to match a wrong result;
  - unfinished work: a "not implemented" stub, an empty `catch`, a swallowed error, a `TODO` where the behavior should be;
  - a weaker config: a lowered coverage or performance threshold, a disabled rule, a lower static-analysis level, a check removed from CI or the check command.
- When the check itself is wrong, say so, show why, and fix the check in its own visible change. Never fold it into the change it unblocks.
- Tightening needs no permission; loosening always does.

**Bad:** the type error goes away with `// @ts-ignore` and the test that caught the bug gets `.skip`. **Good:** the function's input type is fixed, the test passes unchanged, and the reply shows both.
