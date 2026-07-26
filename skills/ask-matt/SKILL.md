---
name: ask-matt
description: Route a task to the right Matt-style workflow skill.
---

# Ask Matt

Use this when you are unsure which workflow skill fits the task best.

## Route to the right skill

- If user asks to fix a bug, first route to `diagnosing-bugs`.
- If user asks for a specific enhancement, route to `to-spec` (and `to-tickets` if it needs slicing).
- If user asks for an assessment or "estado", route to `grill-with-docs` (if repo/docs exist) or `grill-me` (no repo context).
- Idea shaping in a real repo or with docs: `grill-with-docs`
- Idea shaping without a codebase: `grill-me`
- Repo bootstrap for issue tracker, labels, and domain docs: `setup-matt-pocock-skills`
- Settled conversation into a spec: `to-spec`
- Spec into execution slices: `to-tickets`
- Incoming issue or PR triage: `triage`
- Architecture friction or shallow modules: `improve-codebase-architecture`
- Domain vocabulary, glossary, or `CONTEXT.md`: `domain-modeling`
- Broad implementation or long effort: `implement`, `code-review`, or `tdd`
- Decision-heavy ambiguity: `wayfinder`

If blockers are conflict-heavy, route to `resolving-merge-conflicts`.

If the user already named the right skill, use it directly.

For bug, improvement, or state-analysis requests, include/ask for:

- objetivo claro
- alcance
- impacto/riesgo
- evidencia (logs, pantallazos, enlaces, reproducciones)
- criterios de éxito medibles

If the request is too short to act on (`"mejora el login"`, `"arregla X"`), do this first:

- classify it as `mejora` unless explicit breakage is described (`bug`)
- ask for the canonical intake format in `docs/orchestration-task-requests.md`
- collect the minimum required fields before any implementation handoff

Use `docs/orchestration-task-requests.md` as the canonical intake format.
