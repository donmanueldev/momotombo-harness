# Work a Ticket

Use this when a ticket already exists and you need to turn it into execution.

Ticket:
- Link or title:
- Acceptance criteria:
- Related docs:

Instructions:
1. Read the ticket and any linked context first.
2. For complex product changes, use `specifier` first; obtain approval before `planner` creates the execution route.
3. Keep `SPEC-*` and `REQ-*` IDs in the plan, specialist assignments, verification, and review.
4. The root agent delegates one independent concern per agent; planner and orchestrator only return the route.
5. Use `backend` for API, PostgreSQL, schema, and persistence work; use `infra` only for Docker or local service environments.
6. Finish with `reviewer` before calling it done.

Output:
- execution order
- first task
- risks
- verification
