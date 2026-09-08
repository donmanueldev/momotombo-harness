# Fullstack App

Use this when the work spans frontend, backend, and infrastructure layers in one app.

Goal:
- <describe the feature or app change>

Instructions:
1. Start with `specifier`, approve the spec, then use `planner`.
2. Preserve `REQ-*` IDs through vertical slices, specialist assignments, tests, and review.
3. The root agent executes the route; planner and orchestrator do not create nested agents.
4. Use `web` for UI, `backend` for API logic and PostgreSQL persistence, and `infra` for Docker dependencies.
5. Use `reviewer` after the slices are implemented.
6. Prefer a narrow first slice that proves the end-to-end path.

Output:
- slices
- dependencies
- tests
- verification
