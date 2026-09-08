---
name: deslop
description: Remove demonstrably redundant or AI-generated code from an approved scope.
---

# Focused cleanup

Use this skill when the user asks to simplify an approved diff or scope. Do not treat every unfamiliar pattern as slop.

## Workflow

1. Identify the exact scope and the real comparison base from the repository; do not assume `main` when the project uses another base branch.
2. Read the surrounding code, callers, tests, and repository instructions before proposing a deletion or simplification.
3. Remove only demonstrably redundant comments, dead branches, duplicate abstractions, defensive code that contradicts an established boundary, or type workarounds hiding a real defect.
4. Preserve intentional behavior, public contracts, required validation, accessibility, error reporting, and existing work outside the approved scope.
5. Run the smallest relevant validation and report both what changed and what remains unverified.

## Boundaries

- Do not modify code merely because it looks generated or verbose.
- Do not add abstractions, rewrite architecture, or change formatting outside the scope to make a diff look cleaner.
- When deletion affects a public API, dynamic loading, framework convention, runtime configuration, or uncertain consumer, stop and ask before changing it.
