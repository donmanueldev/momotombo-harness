# Setup kitchen

The kitchen is what rejects bad code before a person sees it: linter, formatter, type checker, test runner, and one check command.

**Outcome:** the project has a fast check and a full check that catch the mistake at hand, and both pass.

1. If the stack has a kitchen guide, follow it: `kitchens/laravel.md`, `kitchens/react.md`, `kitchens/expo.md`, `kitchens/python.md`. Otherwise find what the ecosystem offers (official docs first): its linter, formatter, type checker, and test runner. Reuse what the project has before adding anything.
2. Wire one `check` command that runs all of them, and a fast variant that runs only what covers changed files. Name both in the project's `AGENTS.md`.
3. For a correction the user made twice, encode it (`principles/encode-lessons-in-structure.md`): enable the lint rule that catches it, add a test or a type, or write a small script. Only when none can, record it in text: Boost's `record-rule` in a Laravel project with Boost, otherwise a `docs/` page with `codely-doc-create`.
4. Ask before adding or upgrading dependencies. Pin versions.
5. Run the full check. Fix what it finds, or, for a large legacy backlog, ask the user before setting a baseline the tool supports, so new code is held to the rule (`principles/never-lower-the-bar.md`).

**Stop:** when both checks run green and the new rule fails on an example of the mistake.

**Reply:** what was added, the commands, and the proof that the rule catches the mistake.
