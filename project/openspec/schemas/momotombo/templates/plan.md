---
name: "<!-- change name -->"
description: "<!-- one sentence -->"
created_at: "<!-- YYYY-MM-DDTHH:MM:SSZ -->"

created_by:
  tool: "<!-- Claude Code | Codex -->"
  model:
    name: "<!-- e.g. Claude Opus -->"
    version: "<!-- e.g. 5.5 -->"
    reasoning_effort: "<!-- low | medium | high -->"
---

# <!-- Plan title -->

## Goal

<!-- 1-3 sentences: what gets built, for whom, and why it matters -->

## Decisions

- <!-- choice or assumption (instead of <discarded alternative>, when not obvious) -->
- Out of scope: <!-- what this change does not do -->

## Phases

### Phase 1: <!-- vertical slice a user can complete a goal with -->

<!-- What this phase delivers -->

**Contracts**

```<!-- project language -->
// <!-- Contract type -->
<!-- signature -->
```

**UX states**

- <!-- screen -->: loading <!-- ... -->, empty <!-- ... -->, error <!-- ... and how the user recovers -->, success <!-- ... -->

**Tests**

- `<!-- test suite -->`: <!-- behaviours it covers -->

- [ ] <!-- task (cite the existing code to copy patterns from) -->
- [ ] Verify: run the Check command from AGENTS.md (or its typecheck, lint, format, and test commands if it has none), check the phase's behaviour in the running app, and fix any issue.

## Next step

Run `openspec-apply-change` (`/openspec-apply-change` in Claude Code, `$openspec-apply-change` in Codex) to implement Phase 1.
