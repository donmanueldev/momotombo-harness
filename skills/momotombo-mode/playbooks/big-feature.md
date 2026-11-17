# Big feature

For a new data model, an auth rule, or several screens.

**Outcome:** the whole feature built and verified after a single approval.

1. Investigate the affected areas (`playbooks/investigate.md`; use `analyst` for a wide sweep).
2. Write a short plan in the chat, no files:
   - **Data shape**: the models, types, and states.
   - **Contracts**: routes and payloads, commands, events, screen props. Public signatures only.
   - **Phases**: each a vertical slice that works and is verifiable on its own (`principles/sequence-verifiable-units.md`).
   - **Decisions**: choices not stated in the request, and what is out of scope.
   - **Security**: who may do what, and where it is enforced.
   - **States and edges**: loading, empty, error, and success per screen; permissions, retries, large data, slow network; a rollback for migrations.
3. Ask for approval **once**. This is the only stop before building.
4. Build every phase in order with `playbooks/feature.md`. Each phase ends with its checks passing. Phases that touch disjoint files may run side by side with `playbooks/parallel.md`.
5. If the code proves the plan wrong, change the plan, say what changed and why, and keep going. Stop only if the change crosses the approval boundary.

**Stop:** when every phase is built and verified.

**Reply:** what was built per phase, the deviations from the plan, the evidence, and what is left open.
