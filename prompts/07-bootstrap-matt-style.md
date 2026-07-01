# Bootstrap a Repo for Matt-Style Skills

Use this when you are starting a new repo and want the Matt-style engineering flow wired in from day one.

Copy this into Codex:

```text
Use setup-matt-pocock-skills.

Repo:
- Name:
- Remote:
- Primary stack:

Tracker:
- Issue tracker:
- Treat external PRs as request surfaces: yes/no

Triage labels:
- needs-triage:
- needs-info:
- ready-for-agent:
- ready-for-human:
- wontfix:

Domain docs:
- Layout: single-context / multi-context
- Main context file:
- ADR location:

Rules:
- Use AGENTS.md if it exists, otherwise ask before creating a new root instruction file.
- Keep the docs ready for to-prd, to-issues, triage, grill-with-docs, improve-codebase-architecture, and domain-modeling.

Output:
- what exists already
- what will be written
- what the default flow should be for this repo
```

Use this when you want to bootstrap a repo without rewriting the setup prompt every time.
