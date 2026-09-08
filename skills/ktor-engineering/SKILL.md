---
name: ktor-engineering
description: Build, diagnose, and review Ktor Server applications. Use for Kotlin server APIs, routing, dependency injection, coroutines, error handling, security, and Ktor server tests; exclude Ktor Client and multiplatform work.
---

# Ktor Server engineering

Inspect the Ktor version, server engine, modules, existing plugins, configuration, and build conventions before choosing an API or dependency. Preserve the application's established dependency injection and serialization approach.

## Service design

- Keep routes as HTTP adapters: parse and validate input, authorize through the existing policy, invoke a use case, and return an explicit response contract. Keep business rules and persistence details outside route handlers.
- Use SOLID as a review lens, not a mandate for interfaces or layers. Introduce an abstraction only at a genuine changing-policy, I/O, or test boundary; keep each type responsible for a coherent behavior.
- Prefer Ktor's built-in DI or the project’s current DI mechanism. Do not introduce Koin, Exposed, HikariCP, a database, or a version constraint merely to satisfy this skill.
- Centralize intended application errors through `StatusPages` or the existing equivalent. Do not expose stack traces, internal exception messages, credentials, or implementation details in responses or logs.

## Coroutines and lifecycle

- Make I/O and long-running work `suspend` where the stack supports it. Preserve structured concurrency so child work remains owned by a request or explicit application scope, with predictable cancellation and failure propagation.
- Do not introduce detached work, blocking calls in request handling, or a custom dispatcher without first establishing its ownership, cancellation, and shutdown behavior.

## Security and operations

- Extend the existing authentication and authorization configuration rather than creating permissive defaults. Validate untrusted input and keep secrets in external configuration.
- Add contextual, non-sensitive logs and health/metrics endpoints only when the application already supports those operational contracts or the request requires them.

## Verification

- Test pure domain and use-case behavior with the project's existing Kotlin test framework. Cover requested behavior plus relevant failure and boundary paths.
- Use Ktor `testApplication {}` with its provided client for routes and plugin behavior that can be proven in an isolated server test environment. Reuse production module wiring where appropriate and substitute external dependencies explicitly.
- Report unit, isolated-server, integration, and external-service evidence as separate gates. Run the narrowest relevant Gradle or Maven verification command defined by the project.

## Boundaries

- Do not install dependencies, start services, call external APIs, publish artifacts, or change runtime credentials without explicit authorization.
- Respect active repository instructions and orchestration contracts. This skill does not create agents, mutate agent routing, install hooks, or expand tool permissions.

References: [Ktor server testing](https://ktor.io/docs/server-testing.html), [Ktor dependency injection](https://ktor.io/docs/server-dependency-injection.html), and [Kotlin structured concurrency](https://kotlinlang.org/docs/coroutines-basics.html).
