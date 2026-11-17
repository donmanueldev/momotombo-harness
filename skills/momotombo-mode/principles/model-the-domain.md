# Model the domain

**When:** writing stateful logic, branching a lot, or repeating the same shape assumption in several files.

- Name the data shape before writing logic. Get the core types and structures right and the code that follows becomes obvious.
- Encode the domain in one structure: a state machine or enum instead of several booleans, a table or map instead of a chain of conditionals, a typed model instead of loose arrays.
- Use the domain's words for names, from the business, not from the framework.

**Bad:** `isPaid`, `isShipped`, `isCancelled` booleans that can all be true. **Good:** one `status` with the allowed transitions defined in one place.
