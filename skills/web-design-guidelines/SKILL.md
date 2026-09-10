---
name: web-design-guidelines
description: Audit implemented React, Inertia.js, HTML, and CSS against versioned web interface guidelines. Use for review and compliance findings, not creative direction, implementation, backend work, or native UI.
license: MIT; see LICENSE.txt
metadata:
  author: vercel
  version: "1.0.0"
  source: https://github.com/vercel-labs/agent-skills/tree/063bee94c3f4df8453406c830b0a7df0f2860278/skills/web-design-guidelines
  guidelines_revision: e3d624baaf29dc1fc645aff3e38f03e564d2d6b1
  argument-hint: <file-or-pattern>
---

# Web Interface Guidelines

Review implemented web UI for compliance with the versioned guidelines stored with this skill.

## How It Works

1. Read the specified files or discover the affected web UI files from the approved scope.
2. Read [`references/web-interface-guidelines.md`](references/web-interface-guidelines.md) completely.
3. Apply only rules relevant to the observed element and product context. Do not report hypothetical violations.
4. Output findings in the terse `file:line` format required by the reference.

## Boundaries

- This is an audit skill. Do not edit files unless the user separately asks for fixes.
- Repository conventions and product requirements take precedence over generic guidance.
- Treat contextual recommendations as heuristics. A rule is not a finding without evidence in the reviewed code or rendered behavior.
- Do not fetch replacement instructions at runtime. The local reference is pinned to `vercel-labs/web-interface-guidelines@e3d624baaf29dc1fc645aff3e38f03e564d2d6b1`.
- Use `frontend-design` for creative direction and `agent-browser` when browser execution is needed to confirm behavior.

## Usage

When a file or pattern is supplied, audit it directly. Otherwise inspect the current diff and likely React, Inertia, HTML, and CSS entry points first; ask for a scope only if repository inspection leaves multiple materially different candidates.
