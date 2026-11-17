# Feature

**Outcome:** the behavior works for the user on the real surface, with tests and no new debt.

1. Trace the part of the system you'll touch and find the existing patterns to copy (`file:line`).
2. Name the data shape first: the types, models, or state the feature needs (`principles/model-the-domain.md`, `principles/derive-dont-sync.md`). Pick the structure before writing logic.
3. Subtract first: delete what the feature makes dead or duplicate (`principles/subtract-before-add.md`).
4. Build it as one vertical slice through every layer it needs: input validation and authorization at the boundary (`principles/authorize-at-the-boundary.md`), logic, persistence, and the screen with its loading, empty, error, and success states.
5. Test the behavior: one test per user goal through the public interface, plus unit tests for business rules (`principles/test-behavior-not-implementation.md`).
6. Verify on the surface: run the checks that cover the change, then exercise the feature (request, command, or screen).

**Stop:** when it works on the surface and the affected checks pass. Don't commit unless the user asks.

**Reply:** what you built, the decisions you made and why, the evidence, and anything left open.
