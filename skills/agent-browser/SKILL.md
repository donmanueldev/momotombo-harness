---
name: agent-browser
description: Verify web interfaces and browser flows with explicit, evidence-based automation.
---

# Browser verification

Use this skill only when a rendered web interface, interaction, or browser-only failure needs evidence.

## Before starting

1. State the page, behavior, viewport, and expected outcome to verify.
2. Prefer an already available browser automation surface. Do not install a browser, a global CLI, an extension, or dependencies unless the user explicitly asks.
3. For an authenticated or external site, use only the requested environment and restrict navigation to its known domains.
4. Do not expose credentials, cookies, tokens, personal data, or captured page contents in commands or summaries.

## Verification loop

1. Open the requested page and capture the initial state.
2. Inspect the relevant controls before interacting.
3. Perform the smallest action that exercises the requested behavior.
4. Capture the resulting state again after navigation, mutation, or asynchronous loading.
5. When layout matters, verify the requested desktop and mobile viewport; take a screenshot only when it is useful evidence.

## Boundaries

- Read-only inspection is the default.
- Do not submit forms, send messages, publish content, delete data, or trigger payments without explicit user authorization.
- Do not treat a screenshot, a successful build, or a synthetic event as proof of a different browser behavior.
- If the app cannot be reached, authentication is unavailable, or the browser surface is blocked, report that exact gate instead of claiming visual verification.

## Report

State the page and viewport tested, observed outcome, evidence collected, and any unverified runtime boundary.
