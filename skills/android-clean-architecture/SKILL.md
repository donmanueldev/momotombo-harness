---
name: android-clean-architecture
description: Evolve Android and Kotlin Multiplatform modules with explicit domain, data, presentation, and side-effect boundaries. Use for repositories, use cases, dependency direction, and data flow.
metadata:
  source: https://github.com/affaan-m/ecc/tree/main/skills/android-clean-architecture
  local_adaptation: agent-config
---

# Android Clean Architecture

Use architecture to make business behavior and side effects easier to understand and test, not to force every Android project into a fixed module diagram.

## Workflow

1. Trace the feature from UI entry point through state holder, use case or service, repository, and concrete data source before moving responsibilities.
2. Keep domain rules independent from Android framework, transport DTOs, persistence entities, and UI models. Map at the boundary where representations change.
3. Depend on stable interfaces at module boundaries only when a real consumer or substitute requires it. Avoid one-interface-per-class and speculative use cases.
4. Keep transactions, retries, network calls, database access, and platform services explicit and injectable. Preserve idempotency and failure semantics for user-visible mutations.
5. Change the smallest boundary that explains the problem; do not reorganize modules or introduce DI libraries during a targeted fix.

## Validation

- Cover pure domain and orchestration decisions with deterministic tests.
- Add integration coverage for persistence, network, transactions, or Android framework behavior only when that boundary changes.
- Build the relevant Android/KMP targets and separately validate runtime behavior on the affected platform.

## Boundaries

- Follow the repository's existing layering and naming where it is coherent.
- Never expose persistence entities directly as transport or UI contracts.
