# Minimize reader load

**When:** shaping code, or code is hard to follow.

- Count the layers between a question and its answer, and remove the ones that add nothing. Inline one-caller wrappers.
- Shrink mutable scope and hidden state the reader must keep in their head.
- Prefer a plain function and data over a class hierarchy, and an early return over nesting.
- Keep related code together; split a file when it holds unrelated things, not when it is long.

**Bad:** controller → service → manager → helper, each forwarding one call. **Good:** the controller calls the one function that does the work.
