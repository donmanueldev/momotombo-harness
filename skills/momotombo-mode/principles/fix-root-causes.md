# Fix root causes

**When:** debugging anything.

- Trace the symptom back to the first place the system went wrong, and fix it there.
- A guard (null check, try/catch, retry, default value) is a fix only when the state it handles is legitimate. Otherwise it hides the bug.
- Every line you ship traces to evidence. A change that "might help" is a hypothesis; it doesn't ship.
- When evidence refutes a hypothesis, revert what it motivated.

**Bad:** `total ?? 0` in the view because the total is sometimes missing. **Good:** find why the order is saved without a total and fix that write.
