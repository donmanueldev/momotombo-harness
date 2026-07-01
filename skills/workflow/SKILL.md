---
name: workflow
description: "Orchestrate complex coding tasks with plans, subagents, and verification."
---

## Workflow Orchestration

### 1. Plan Mode Default

- Enter plan mode for ANY non-trivial task (3+ steps or architectural decisions)
- If something goes sideways, STOP and re-plan immediately — don't keep pushing
- Use plan mode for verification steps, not just building
- Write detailed specs upfront to reduce ambiguity

### 2. Subagent Strategy

- Start with `ask-matt` when you want the router to choose the right flow.
- Start with `planner` for any task with more than one plausible path or more than one file group.
- Delegate one concern per agent. Keep each subagent focused.
- Use `web`, `backend`, `data`, `mobile`, or `reviewer` rather than mixing domains.
- Ask each agent for facts, concrete changes, tests, and risks.

### 3. Matt-Style Flows

- `grill-with-docs` for idea shaping in a real codebase
- `grill-me` for idea shaping without a codebase
- `setup-matt-pocock-skills` for new-repo bootstrap of tracker, labels, and domain docs
- `to-prd` once the conversation is settled and you need a spec
- `to-issues` when the PRD needs tracer-bullet slices
- `triage` for incoming bugs, requests, or PRs
- `improve-codebase-architecture` when the code feels shallow or hard to navigate
- `domain-modeling` when names drift or the glossary is missing

### 4. Stack Routing

- `web` -> React, Next.js, Tailwind, shadcn/ui, HeroUI, SEO, ecommerce
- `backend` -> TypeScript, Node.js, FastAPI, Spring Boot
- `data` -> Supabase, Postgres, Prisma, Drizzle, Redis, Docker
- `mobile` -> Expo, React Native, Swift, SwiftUI, Kotlin
- `reviewer` -> read-only diff review, regressions, security, performance, missing tests

### 5. Self-Improvement Loop

- After ANY correction from the user: update `tasks/lessons.md` with the pattern
- Write rules for yourself that prevent the same mistake
- Ruthlessly iterate on these lessons until mistake rate drops
- Review lessons at session start for relevant project

### 6. Verification Before Done

- Never mark a task complete without proving it works
- Diff behavior between main and your changes when relevant
- Ask yourself: "Would a staff engineer approve this?"
- Run tests, check logs, demonstrate correctness

### 7. Demand Elegance (Balanced)

- For non-trivial changes: pause and ask "is there a more elegant way?"
- If a fix feels hacky: "Knowing everything I know now, implement the elegant solution"
- Skip this for simple, obvious fixes — don't over-engineer
- Challenge your own work before presenting it

### 8. Autonomous Bug Fixing

- When given a bug report: just fix it. Don't ask for hand-holding
- Point at logs, errors, failing tests — then resolve them
- Zero context switching required from the user
- Go fix failing CI tests without being told how

## Task Management

1. **Plan First**: Write plan to `tasks/todo.md` with checkable items
2. **Verify Plan**: Check in before starting implementation
3. **Track Progress**: Mark items complete as you go
4. **Explain Changes**: High-level summary at each step
5. **Document Results**: Add review section to `tasks/todo.md`
6. **Capture Lessons**: Update `tasks/lessons.md` after corrections

## Core Principles

- **Simplicity First**: Make every change as simple as possible. Impact minimal code.
- **No Laziness**: Find root causes. No temporary fixes. Senior developer standards.
- **Minimal Impact**: Changes should only touch what's necessary. Avoid introducing bugs.
