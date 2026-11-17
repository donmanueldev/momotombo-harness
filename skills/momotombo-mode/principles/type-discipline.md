# Type discipline

**When:** designing types or signatures, in any typed language.

- Make illegal states unrepresentable: unions and enums over flags, required over optional when the value always exists.
- Give meaning to primitives that carry it (ids, money, emails) instead of passing bare strings and numbers.
- Handle every variant exhaustively, so a new case fails to compile instead of failing at runtime.
- Derive types from the authoritative schema instead of redefining them by hand. No `any` or casts to silence the checker.

**Bad:** `{ loading: boolean; error?: string; data?: User }`. **Good:** `{ state: 'loading' } | { state: 'error'; message: string } | { state: 'ready'; user: User }`.
