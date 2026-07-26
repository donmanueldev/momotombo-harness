---
name: to-spec
description: Turn the current conversation into a spec and publish it to the project tracker.
disable-model-invocation: true
---

# To Spec

Use this when the conversation is mature and you need a durable plan artifact.

## Process

- Do not interview further; synthesize what is already known.
- Explore the repo first if context is missing (CONTEXT, ADRs, recent architecture decisions).
- Define the testing seams and keep module boundaries explicit.
- Produce a spec with the exact headings below and publish it with `ready-for-agent` label.

## Required sections

## Problem Statement
What the user is actually trying to solve.

## Solution
The solution at the behavior level.

## User Stories
Long, numbered list in `As a ..., I want ..., so that ...` format.

## Implementation Decisions
- New seams and interfaces
- API contracts / schema changes
- Architecture constraints

## Testing Decisions
- What quality checks prove the behavior
- Which modules to test and why
- Precedents in the codebase

## Out of Scope
What is explicitly excluded.

## Further Notes
Extra clarifications, open questions, and risks.
