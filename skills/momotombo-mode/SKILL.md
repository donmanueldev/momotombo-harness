---
name: momotombo-mode
description: How to do any engineering task in a Momotombo project, in any stack. Use for every task that investigates, changes, fixes, refactors, speeds up, or integrates code. Picks a playbook and applies the code, test, architecture, and security principles.
license: MIT
metadata:
  author: momotombodevs
  version: "1.0.0"
  adapted-from: https://github.com/cursor/plugins/tree/main/pstack (MIT), https://github.com/addyosmani/agent-skills (MIT)
---

# Momotombo mode

Ship less code of higher quality, and prove it works. The playbook, principle, and kitchen files live next to this file: `playbooks/<name>.md`, `principles/<name>.md`, and `kitchens/<stack>.md` (`kitchens/laravel.md`, `kitchens/react.md`, `kitchens/expo.md`, `kitchens/python.md`: the lint, analysis, and check setup per stack).

## Start

1. Say the goal and the smallest change that meets it in one line, then proceed.
2. Read the project's `AGENTS.md`, the rules in `.ai/rules/index.md` (Laravel Boost) that match the files you'll touch, and only the `docs/` its map lists for this task. Use the stack's official skills and, in Laravel, Boost's `search-docs`. The code, the docs, and live sources are the truth; memory is not.
3. Pick the playbook, copy its steps into your todo list, and follow them.

## Playbooks

| Playbook | When |
|---|---|
| `playbooks/investigate.md` | A question: how does X work, why is Y like this. No edits. |
| `playbooks/bug-fix.md` | Something is broken. |
| `playbooks/feature.md` | New or changed behavior that fits in existing modules. |
| `playbooks/big-feature.md` | A new data model, auth rule, or several screens. |
| `playbooks/refactor.md` | Change structure without changing behavior. |
| `playbooks/perf.md` | Something measured is slow. |
| `playbooks/integrate.md` | The user asks to commit, open a PR, or merge. |
| `playbooks/setup-kitchen.md` | The project lacks a linter, type checker, test runner, or check command, or the user corrects the same mistake twice. |
| `playbooks/autonomous-run.md` | "Run until done", "I'm going to bed": a long task whose goal a command can check. |
| `playbooks/parallel.md` | Work that splits into independent tasks on disjoint files. |

## Principles

Read a principle's file in full the first time it applies in a session. In your reply, name the principles that changed a decision.

**Core**
- `principles/smallest-change.md`: sizing any diff. The smallest change that solves the problem; no layers "for later".
- `principles/subtract-before-add.md`: before adding. Delete dead code, duplicates, and needless checks first.
- `principles/fix-root-causes.md`: debugging. Fix where the system first went wrong, never around the symptom.
- `principles/minimize-reader-load.md`: shaping code. Fewer layers, less hidden state, one-caller wrappers inlined.
- `principles/code-says-what.md`: writing code. Names carry the what; a comment only for a why the code can't show.
- `principles/build-the-lever.md`: repetitive or provable work. Write the script, codemod, or check instead of hand edits.
- `principles/attack-the-premise.md`: two fixes on one premise failed. Test the premise before a third fix.

**Architecture**
- `principles/model-the-domain.md`: stateful or branchy logic. Encode the domain in one structure, not scattered conditionals.
- `principles/derive-dont-sync.md`: state that copies other state. Compute it; don't keep copies in sync with effects, watchers, or observers.
- `principles/boundary-discipline.md`: validation and errors. Guard at the edges, trust types inside, keep logic pure.
- `principles/type-discipline.md`: types and signatures. Make illegal states unrepresentable; parse external data once.
- `principles/idempotent-operations.md`: commands, jobs, webhooks, retries. Same end state however many times it runs.
- `principles/migrate-then-delete-legacy.md`: a new API, pattern, or schema with old callers. Move internal callers and delete the old path in one change; expand, migrate, contract for databases and APIs that shipped apps still call.
- `principles/make-it-observable.md`: behavior that runs in production. Structured logs, metrics, and reported errors that show it works.

**Tests and verification**
- `principles/test-behavior-not-implementation.md`: writing or keeping a test. Assert what users of the code observe.
- `principles/prove-it-works.md`: before saying done. Verify on the real artifact, not "it compiles".
- `principles/sequence-verifiable-units.md`: multi-step work. Small units, each ending in a passing check.
- `principles/never-lower-the-bar.md`: a check fails. Fix the code; never suppress, skip, stub, or loosen a threshold to get green.

**Security**
- `principles/authorize-at-the-boundary.md`: any input or protected action. Validate and authorize on the server, every time.
- `principles/least-privilege.md`: tokens, roles, queries, permissions. Grant only what the task needs.
- `principles/secrets-and-pii-never-logged.md`: logs, errors, prompts, analytics. No secrets or personal data, ever.
- `principles/safe-defaults.md`: new endpoints, flags, config. Deny by default; fail closed.
- `principles/dependency-hygiene.md`: adding or upgrading a package. Prefer none; pin, audit, and ask first.

**Working style**
- `principles/guard-the-context-window.md`: bulk reading. Send it to `analyst`; keep summaries here.
- `principles/never-block-on-the-human.md`: tempted to ask. Proceed on reversible work; ask only at the approval boundary.
- `principles/encode-lessons-in-structure.md`: a correction or a repeated instruction. Turn it into a lint, test, type, or script.

## Autonomy

- Proceed without asking on anything local and reversible.
- Ask first only before what leaves the machine or is hard to undo: push, merge, deploy, external messages, production or shared data, migrations on shared data, new or upgraded dependencies.
- "Don't stop", "run until done", or "I'm going to bed" means `playbooks/autonomous-run.md`.
- Give your real judgment. Push back when a request or an approach doesn't earn its place.

## Subagents

- You write the code in this session. Subagents start cold and slow the user down.
- `analyst`: bulk reading or a wide search, so raw files stay out of this context.
- `worker`: only for independent tasks that touch disjoint files, run in parallel, each in its own worktree (`playbooks/parallel.md`). You review every diff it returns.
- `reviewer`, `security-reviewer`, `comment-sicko`: only in `playbooks/integrate.md`.
- A subagent sees nothing of this conversation. Give it the goal, file paths, constraints, and what to return.

## Verification

- While working, run only the project's fast check (`check:fast`, from its `AGENTS.md` or kitchen) or the tests that cover what you changed. Never the full suite after every edit.
- Check behavior on the surface it lives on, only for what changed:
  - API or backend: call the endpoint or command and read the response and the stored data.
  - Web screen: drive it in a browser (Playwright, the `webapp-testing` skill, or Claude in Chrome) and take a screenshot.
  - Expo screen: run it in a local simulator (`npx expo run:ios`) with the dev server started as `EXPO_UNSTABLE_MCP_SERVER=1 npx expo start` (needs `expo-mcp` as a dev dependency, SDK 54+), then drive it with the Expo MCP local tools `automation_find_view`, `automation_tap`, and `automation_take_screenshot`. `simulator_start` is a billed cloud simulator: use it only when no local one exists (`eas-simulator` skill).
  - "Inconclusive" or the wrong surface is not a pass.
- Run the full check command only in `playbooks/integrate.md`, in the background.
- The comment guard hook reports comments an edit adds. Act on it in the same turn: remove them, or keep one that fits a `code-says-what` exception and say which.

## Reply

Lead with the result. Give each claim its evidence (command output, a screenshot, a value read) or label it inferred. Short sentences. No option menus for work the user already asked for, and never hand the user a check you could run.
