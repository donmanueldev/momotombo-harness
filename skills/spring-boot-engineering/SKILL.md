---
name: spring-boot-engineering
description: Build, diagnose, and review production Spring Boot services. Use for Spring MVC or WebFlux APIs, security, configuration, JPA, transactions, observability, and Spring tests.
---

# Spring Boot engineering

Inspect the current Spring Boot version, build configuration, active profiles, module boundaries, and existing security model before selecting APIs or dependencies. Match the application’s MVC or reactive stack; do not mix blocking JPA work into a reactive request path.

## Service design

- Keep controllers as adapters: validate request DTOs, authorize through the established policy, and return explicit response contracts. Never expose persistence entities directly.
- Use constructor injection for required collaborators so components are fully initialized and independently testable. Treat a long constructor or a dependency cycle as a design signal: simplify the collaboration instead of masking it with field injection, setters, or `@Lazy` by default.
- Apply SOLID as a review lens: keep each type responsible for a coherent behavior, program against a contract only at a genuine interchangeable boundary, and preserve substitutability for existing clients. Do not introduce interfaces or layers solely to satisfy an acronym.
- Place transactions at service/use-case boundaries. Define rollback and idempotency behavior for mutations that can be retried; use migrations for schema changes.
- Bind configuration through typed properties and environment-specific configuration. Keep secrets external, redact sensitive values from logs, and fail clearly when required configuration is absent.
- Use pagination, projections, and targeted fetch strategies for lists. Measure and address N+1 queries instead of adding broad eager loading.

## Security and operations

- Extend the existing `SecurityFilterChain` and authorization model rather than disabling CSRF, authentication, or method security by default. Expose only intentional Actuator endpoints and protect sensitive management surfaces.
- Prefer structured, contextual logs and health checks that distinguish liveness from dependency readiness when the application supports them.

## Verification

- Test domain and use-case objects as plain Java where feasible, using explicit collaborators or test doubles. Add focused web or persistence slices for framework contracts.
- For MVC endpoints, use `MockMvc` when no running server is required; for WebFlux, use `WebTestClient`. Use `@SpringBootTest` with a random port only when the HTTP-server boundary itself must be proven, and keep test-only wiring scoped with `@TestConfiguration` where needed.
- Use integration tests with real dependencies only when the repository provides that path; report unit, context, web, integration, and external-environment gates separately.
- Run the narrowest relevant Maven or Gradle verification command. Do not require arbitrary coverage thresholds or scaffold databases, Redis, MongoDB, Docker Compose, or OpenAPI dependencies unless the product needs them.

## Boundaries

- Respect active repository instructions, user approvals, and the repository's orchestration contract. This skill does not create agents, mutate agent routing, install hooks, or run external services without authorization.

For the official-source rationale behind these constraints, read [the Java and Spring alignment note](../../docs/research/java-spring-official-alignment.md).
