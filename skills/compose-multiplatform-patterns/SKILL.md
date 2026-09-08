---
name: compose-multiplatform-patterns
description: Build and evolve Compose Multiplatform UI with explicit state, navigation, theming, performance, and platform boundaries. Use for shared Compose UI in Kotlin Multiplatform projects.
metadata:
  source: https://github.com/affaan-m/ecc/tree/main/skills/compose-multiplatform-patterns
  local_adaptation: agent-config
---

# Compose Multiplatform Patterns

Use shared Compose only for behavior and UI that genuinely belong to multiple targets. Keep each platform's lifecycle, accessibility, navigation integration, and system UI explicit.

## Workflow

1. Identify source-set ownership and the current state/navigation contract before editing: `commonMain` for portable behavior, platform source sets for platform integration.
2. Model each screen with immutable state and explicit events. Keep composables stateless where practical; retain side effects in controllers, view models, or injected services.
3. Prefer stable UI models, keyed lazy content, and narrowly scoped state reads. Measure or profile before introducing performance-oriented abstractions.
4. Reuse the project theme and navigation implementation. Do not prescribe a DI framework, navigation library, or `expect`/`actual` split when the existing project has a valid alternative.
5. Keep platform-specific APIs behind an explicit boundary and test shared domain/state behavior without requiring a device.

## Validation

- Run focused common tests and compile every modified target.
- Validate Android and iOS/Desktop behavior separately when a shared UI or platform boundary changes.
- Treat a common-source test or one platform's build as partial evidence, never cross-platform acceptance.

## Boundaries

- Preserve existing module boundaries and public contracts unless the requested work explicitly changes them.
- Favor a small, understandable composition over a new cross-platform framework.
