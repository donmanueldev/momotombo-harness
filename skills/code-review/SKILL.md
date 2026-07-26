---
name: code-review
description: Review changes on two axes: repo standards and spec conformance.
---

# Code Review

Use this before merge or after a work session when the user asks for review.

## Axes

- **Standards**: coding standards, patterns, and maintainability in this repo.
- **Spec**: behavior fidelity against the issue/PRD/spec.

## Process

1. Confirm fixed point (SHA, branch, tag, or merge base).
2. Capture the comparison once: `git diff <fixed-point>...HEAD` and `git log <fixed-point>..HEAD --oneline`.
3. Return findings in a machine-readable block (JSON or YAML) aligned with `docs/orchestration-output-contract.md`.
4. Run standards review and spec review in parallel, then synthesize findings.
