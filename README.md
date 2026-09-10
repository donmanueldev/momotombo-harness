# agent-config

My Codex configuration for skills, subagents, and synced workflow files.

## Quick start

```bash
git clone https://github.com/manuelduarte077/agent-config.git
cd agent-config
./install.sh
```

## What's included

### Codex
- `.codex/config.toml` - Project overlay for subagents and stack-specific plugin toggles
- `.codex/agents/*.toml` - Codex native subagents for specification, planning, orchestration, web, backend, infrastructure, mobile, and review
- `rules/agent-config.rules` - Safe command allowlist installed at `~/.codex/rules/agent-config.rules`

### Prompts
- `prompts/` - Copy-paste task starters for zero-to-one work, tickets, backend, frontend, fullstack, fixes, features, and SEO/PageSpeed launch preparation

### Skills
Reusable capabilities that your coding agents can invoke.

Matt Pocock, official Android, and official Kotlin skills are managed globally by the official `skills.sh` installer, outside this repository. They are never copied into this repository, which avoids duplicate local copies.

| External source | Synchronize | Setup opt-in |
|---|---|---|
| Matt Pocock | `./sync.sh matt` | `./install.sh --with-matt` |
| [Android](https://github.com/android/skills) | `./sync.sh android` | `./install.sh --with-mobile` |
| [Kotlin](https://github.com/Kotlin/kotlin-agent-skills) | `./sync.sh kotlin` | `./install.sh --with-mobile` |

Run `./sync.sh mobile` to reconcile both official mobile sources. Local community skills remain in this repository so their Codex integration can be reviewed and adapted; they do not automatically update from their upstream repositories.

| Skill | Description |
|-------|-------------|
| `agent-browser` | Evidence-based browser verification for web interfaces |
| `commit-english` | Write git commit messages in English |
| `fix-sentry-issues` | Investigate and address Sentry issues |
| `pr-comments` | Draft concise PR comments and review feedback |
| `rams` | Run accessibility and visual design review |
| `deslop` | Remove verified redundant code from an approved scope |
| `favicon` (optional) | Generate favicons from a source image |
| `knip` (optional) | Investigate potentially unused Node.js code and dependencies |
| `mobile-android-design` | Local Android Material 3 and adaptive UI guidance |
| `compose-multiplatform-patterns` | Local Compose Multiplatform state and platform-boundary guidance |
| `android-clean-architecture` | Local Android/KMP layering and data-flow guidance |
| `write-swift` | Modern Swift language, value semantics, generics, ARC, testing, and performance guidance |
| `swift-api-design-guidelines` | Swift API naming, argument labels, documentation, and call-site clarity |
| `swift-architecture` | Apple-platform architecture selection, boundaries, and incremental migration |
| `swift-codable` | Codable contracts, JSON encoding and decoding, and persistence boundaries |
| `swift-testing-pro` | Modern Swift Testing patterns, async tests, and XCTest migration |
| `website-seo-finalization` | Automated URL reports plus pre-launch SEO, AEO and GEO readiness, metadata, structured data, crawler controls, answers, and citations |
| `pagespeed-optimization` | Reproducible Lighthouse medians, CI budgets, CrUX interpretation, and React, Next.js, Astro, or static HTML adapters |
| `frontend-design` | Creative direction and implementation guidance for distinctive React, Inertia.js, HTML, and CSS interfaces |
| `ui-ux-pro-max` | Offline UI/UX lookup for React, Laravel/Inertia, and HTML/Tailwind decisions |
| `web-design-guidelines` | Versioned source audit for accessibility and web-interface quality |

SwiftUI Expert, Swift Concurrency, Build iOS Apps, and Build macOS Apps remain installed as Codex plugins and are intentionally not duplicated here. The vendored Swift skills use these reviewed upstream snapshots:

| Skills | Upstream | Revision |
|--------|----------|----------|
| `write-swift` | [`emilkowalski/skills`](https://github.com/emilkowalski/skills) | `d23d7f88a2e21c9e4b1418c7abe420f5c1052ba7` |
| `swift-api-design-guidelines`, `swift-architecture`, `swift-codable` | [`dpearson2699/swift-ios-skills`](https://github.com/dpearson2699/swift-ios-skills) | `8d90fd121a263355a4fb44fd082af1416a5c1c2a` |
| `swift-testing-pro` | [`twostraws/swift-testing-agent-skill`](https://github.com/twostraws/swift-testing-agent-skill) | `2d6bba14a3c8bf3694f218b92fffe617c41ae43e` |

The SEO launch skills are also reviewed and versioned locally:

| Skill | Origin | Revision |
|-------|--------|----------|
| `website-seo-finalization` | Adapted from [`YankielDBC2/codex-skills-library`](https://github.com/YankielDBC2/codex-skills-library/tree/674d9e1c74bbf3a922bae483747bc89158bf1507/plugins/skills-pack/skills/website-seo-finalization) | `674d9e1c74bbf3a922bae483747bc89158bf1507` |
| `pagespeed-optimization` | Locally authored from official Google Search, Chrome, Lighthouse, CrUX, and web.dev guidance | Local |

OpenGraph.to is used by `website-seo-finalization` only as an optional public-URL preview check. It is not installed as a plugin and its score is not a release gate.

The web design skills are reviewed, narrowed to the supported web stack, and versioned locally:

| Skill | Origin | Revision |
|-------|--------|----------|
| `frontend-design` | Adapted from [`anthropics/skills`](https://github.com/anthropics/skills/tree/41bbe19d1a1a7eaab5e7bb9050a417e5c6cffc8f/skills/frontend-design) | `41bbe19d1a1a7eaab5e7bb9050a417e5c6cffc8f` |
| `ui-ux-pro-max` | Adapted from [`nextlevelbuilder/ui-ux-pro-max-skill`](https://github.com/nextlevelbuilder/ui-ux-pro-max-skill/tree/4aad0584d92131626b16d4ff4d77f0455385013c/.claude/skills/ui-ux-pro-max) | `4aad0584d92131626b16d4ff4d77f0455385013c` |
| `web-design-guidelines` | Adapted from [`vercel-labs/agent-skills`](https://github.com/vercel-labs/agent-skills/tree/063bee94c3f4df8453406c830b0a7df0f2860278/skills/web-design-guidelines); rules from [`web-interface-guidelines`](https://github.com/vercel-labs/web-interface-guidelines/blob/e3d624baaf29dc1fc645aff3e38f03e564d2d6b1/command.md) | `063bee94c3f4df8453406c830b0a7df0f2860278` / `e3d624baaf29dc1fc645aff3e38f03e564d2d6b1` |

For substantial web design work, `frontend-design` owns creative direction, `ui-ux-pro-max` supplies focused local research, `web-design-guidelines` audits the implemented source, and `agent-browser` provides separate rendered-browser evidence. Project instructions, an approved brief, and an existing design system always take precedence.

## Managing your config

```bash
# See what's synced vs local-only
./sync.sh

# Preview what install would do
./install.sh --dry-run

# Add a local skill to the repo
./sync.sh add skill my-skill
# Add a local subagent to the repo
./sync.sh add agent planner
./sync.sh push

# Delete a skill from repo and local machines
./sync.sh delete skill my-skill
./sync.sh push

# Pull changes on another machine
./sync.sh pull

# Update Matt Pocock skills only
./sync.sh matt

# Update official Android or Kotlin skills, or both
./sync.sh android
./sync.sh kotlin
./sync.sh mobile

# Install this configuration and explicitly reconcile Matt Pocock skills
./install.sh --with-matt

# Install this configuration and explicitly reconcile official Android + Kotlin skills
./install.sh --with-mobile

# Remove a skill or subagent from repo (keeps local copy)
./sync.sh remove skill my-skill
./sync.sh remove agent planner
./sync.sh push
```

### Safe operations with backups

All destructive operations create timestamped backups:

```bash
# List available backups
./sync.sh backups

# Restore from last backup
./sync.sh undo
```

### Validate skills

```bash
./sync.sh validate
```

Skills must have a `SKILL.md` with frontmatter containing `name` and `description`.

## Testing

Tests use [Bats](https://github.com/bats-core/bats-core) (Bash Automated Testing System).

```bash
# Install bats (one-time)
brew install bats-core

# Run all tests
bats tests/

# Run specific test file
bats tests/install.bats
bats tests/sync.bats
bats tests/validation.bats
```

Tests run in isolated temp directories and don't affect your actual `~/.codex` config.
Tests also cover Codex skills syncing in `~/.codex/skills` and Codex agents in `~/.codex/agents`.

## Local-only config

Not everything needs to be synced. The install script only creates symlinks for what's in this repo - it won't delete your local-only skills or agents.

Use `./sync.sh remove skill <name>` when you want to stop syncing a skill but keep the local copy.
Use `./sync.sh remove agent <name>` when you want to stop syncing a subagent but keep the local copy.
Use `./sync.sh delete skill <name>` when you want to remove the skill from both the repo and the local `~/.codex` directories.

If a skill keeps coming back, it is usually coming from a plugin or marketplace install rather than this repo. Disable that source in Codex first, then delete the local skill.

Codex skills are linked from this repo into `~/.codex/skills`.
Codex agents from `.codex/agents/` are linked into `~/.codex/agents` by `./install.sh`.
Matt Pocock, official Android, and official Kotlin skills are installed separately by `skills.sh` and reconciled with `./sync.sh matt`, `./sync.sh android`, `./sync.sh kotlin`, or `./sync.sh mobile`.

Project-specific Codex config lives in `.codex/config.toml` so the repo can narrow the skill and plugin surface without overwriting your home Codex config. For complex product work, use `specifier` to produce an approved, testable specification before `planner` and `orchestrator` return a route for the root agent to execute. The specification preserves requirement IDs through planning, routing, verification, and review.

For Laravel projects that use [Laravel Boost](https://laravel.com/framework/docs/boost), the backend agent first follows the project's Boost guidelines, `.ai/rules`, and package-detected skills. This keeps Inertia, Pest, Filament, and other package-specific conventions local to the application rather than duplicating or overriding them in this global configuration.

Prompt starters:
- `Turn this request into a spec with scope, non-goals, acceptance criteria, and a verification matrix. Wait for my approval before planning.`
- `Plan this feature, split it into web/backend/infra work, and tell me which subagents to run first.`
- `Create and approve a SPEC before planning this complex feature; preserve REQ IDs through tests and staff-level review.`
- `Diagnose this bug and build the smallest reproducible loop before changing code.`
- `Review this diff for regressions, missing tests, and any risky behavior.`
- `Write a concise English commit message for this diff and explain the grouping.`
- `Draft PR review comments for this diff, ordered by severity.`
- `Update the repo rules or Codex extensions without reintroducing Claude/Cursor files.`
- `Use $website-seo-finalization and $pagespeed-optimization to audit, improve, rebuild, and remeasure this site before launch.`
- `Audit this public site with $website-seo-finalization and $pagespeed-optimization without modifying files; return prioritized JSON/Markdown findings.`
- `Verify the existing SEO and Lighthouse reports against the current source, implement confirmed findings, and show before/after evidence.`

See [`prompts/07-seo-pagespeed-launch.md`](prompts/07-seo-pagespeed-launch.md)
for complete examples covering launch optimization, read-only audits, report-driven
remediation, route-level work, and configurable CI performance budgets.

## Creating your own

Fork this repo and customize! The structure is simple:

```
agent-config/
├── .codex/            # Codex config overlay and native agents
├── prompts/           # Reusable task starters
├── skills/            # Skills (subdirectories with SKILL.md)
└── tests/             # Bats tests
```
