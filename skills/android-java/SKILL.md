---
name: android-java
description: Build and debug Android applications written in Java. Use for Android Java screens, lifecycle behavior, permissions, background work, persistence, and device-specific regressions.
---

# Android Java

Start from the affected Android module, its manifest, resources, build variant, and the reported device or API-level behavior. Preserve the app's existing architectural and UI conventions.

## Implementation

- Keep UI work on the main thread and run I/O, database work, and long computation on the project's established background mechanism. Make lifecycle ownership explicit; cancel or detach work that must not outlive its screen or service.
- Keep platform-independent business behavior out of Activities, Fragments, Services, and other framework entry points so it has a small, deterministic test seam. Use abstractions only at a real platform, I/O, or changing-policy boundary; SOLID is a design lens, not a mandate for wrapper classes.
- Validate inputs at the boundary and surface recoverable failures in the UI. Do not replace runtime permission checks with manifest declarations, and do not request permissions that the feature does not need.
- Treat configuration changes, process death, navigation restoration, offline behavior, and Android API-level differences as explicit cases for stateful flows.
- Use resources for user-visible strings, dimensions, colors, and accessibility labels. Do not hardcode secrets, endpoints, or production-only configuration in the app.

## Verification

- Put fast platform-independent tests in the local JVM test source set. Use instrumented tests only where Android framework or real-device behavior is part of the contract; a local unit test does not prove that behavior.
- Cover the relevant normal and edge cases in the smallest test layer. State separately whether emulator, physical-device, release-variant, notification, deep-link, permission, accessibility, or API-level behavior was exercised.
- Run the smallest relevant Gradle test, lint, or module build already configured by the project.

## Boundaries

- Do not install SDK components, start emulators, publish builds, or alter signing and distribution settings without explicit authorization.
- Respect active repository instructions and orchestration contracts. This skill does not create agents, alter agent routing, or register hooks.

For the official-source rationale behind these constraints, read [the Java and Spring alignment note](../../docs/research/java-spring-official-alignment.md).
