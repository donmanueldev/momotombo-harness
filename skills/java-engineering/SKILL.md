---
name: java-engineering
description: Build, diagnose, and safely refactor production Java applications. Use for Java domain logic, APIs, persistence, concurrency, and targeted Java refactors; defer Spring-specific work to spring-boot-engineering.
---

# Java engineering

Use the repository's build tool, Java version, style, and module boundaries. Inspect the current code and tests before changing a public method, a domain model, or a concurrent path.

## Implementation

- Keep related state and behavior together, with encapsulated invariants. Prefer immutable value objects, records where they fit the repository baseline, constructor injection, and explicit error contracts.
- Use SOLID as a review lens, not a requirement to add types. Introduce an interface only when callers need a genuine interchangeable behavior or a stable test seam; prefer composition unless the subtype preserves the parent contract.
- Treat method signatures, serialized fields, and exceptions as compatibility contracts. Find all callers before changing or removing a parameter; update them atomically and preserve observable behavior unless the request explicitly changes it.
- Extract a method only when it makes a coherent responsibility or test seam clearer. Do not apply a size threshold mechanically, force an extraction, or narrow visibility without checking callers and subclasses.
- For concurrency, identify ownership, cancellation, thread safety, and blocking behavior before changing executors, shared state, or asynchronous composition.

## Data and quality

- Validate at public boundaries. Keep persistence details out of API DTOs and avoid N+1 queries by selecting the needed shape deliberately.
- Add focused JUnit tests for changed observable behavior, including relevant failures and boundary cases. A successful compilation is not behavioral proof.
- Run the narrowest relevant Maven or Gradle verification command defined by the project; report unit, integration, device, and external-service evidence as separate gates.

## Boundaries

- Do not introduce framework dependencies, infrastructure, code generators, credentials, or network calls merely to satisfy this skill.
- Respect the active repository instructions and approval boundaries. This skill neither creates agents nor changes orchestration, routing, hooks, or global configuration.

For the official-source rationale behind these constraints, read [the Java and Spring alignment note](../../docs/research/java-spring-official-alignment.md).
