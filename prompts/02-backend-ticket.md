# Backend Ticket

Use this for backend-heavy work: Laravel/PHP, TypeScript, Java, APIs, or PostgreSQL.

Ticket:
- Link or title:
- Expected behavior:
- Data model:

Instructions:
1. For complex changes, start with `specifier`, get approval, then use `planner`.
2. Preserve `SPEC-*` and `REQ-*` IDs in implementation, tests, and review.
3. Use `backend` as the main implementation agent.
4. Use `infra` only when the ticket requires Docker or local service-environment work.
5. Keep transport adapters thin and put business rules behind a small, testable module interface; do not add abstractions without a real variation.
6. Finish with `reviewer`.

Output:
- scope
- implementation order
- tests
- risks
