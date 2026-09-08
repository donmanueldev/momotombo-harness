# Java and Spring official alignment

Research date: 2026-09-08. This note supplies source-backed constraints for the
local Java, Android Java, and Spring Boot skills. It does not turn SOLID into a
framework rulebook: SOLID is a design heuristic rather than a Java or Spring
specification. The recommendations below are only those supported by a primary
source, or a conservative inference explicitly labelled as such.

## Java, POO, and SOLID-adjacent guidance

| Topic | Source-backed guidance | Source |
| --- | --- | --- |
| Encapsulation and cohesion | Model related state and behavior together in objects; Java's official learning material identifies data encapsulation as a benefit of that model. This supports keeping a class focused on one coherent responsibility (an SRP-adjacent inference), without treating every small method as its own type. | [Dev.java: Objects, Classes, Interfaces, Packages, and Inheritance](https://dev.java/learn/oop/) |
| Contracts and polymorphism | An interface is a contract enforced by the compiler: an implementing class must provide its declared methods. Use an interface where callers genuinely need an interchangeable behavior; do not introduce one merely to satisfy a pattern. This supports dependency inversion at a meaningful boundary (inference). | [Dev.java: Objects, Classes, Interfaces, Packages, and Inheritance](https://dev.java/learn/oop/) |
| Access and invariants | Classes and objects documentation includes access control as part of the language model. Prefer the narrowest visibility compatible with the required API, so callers cannot mutate state that the class must protect. | [Dev.java: Classes and Objects](https://dev.java/learn/classes-objects/) |
| Inheritance | Java supports multiple inheritance of type through interfaces but not multiple inheritance of state. Prefer composition or interface-based collaboration unless a subtype relationship preserves the parent contract; this is a conservative design inference from the language model. | [Oracle Java Tutorials: Multiple Inheritance of State, Implementation, and Type](https://docs.oracle.com/javase/tutorial/java/IandI/multipleinheritance.html) |
| Packages | Packages organize related types and make large projects manageable. Structure packages around cohesive application responsibilities rather than a generic catch-all layer. The second sentence is an inference, not an Oracle mandate. | [Dev.java: Objects, Classes, Interfaces, Packages, and Inheritance](https://dev.java/learn/oop/) |

## Spring and Spring Boot design guidance

| Topic | Source-backed guidance | Source |
| --- | --- | --- |
| Dependency injection | Spring defines DI as declaring dependencies through constructor/factory arguments or properties, with the container providing them. It says DI improves decoupling and testability, especially when dependencies are interfaces or abstract bases. | [Spring Framework: Dependency Injection](https://docs.spring.io/spring-framework/reference/core/beans/dependencies/factory-collaborators.html) |
| Mandatory dependencies | Spring advocates constructor injection for mandatory dependencies: it enables immutable, fully initialized components and avoids `null`. Use setters/configuration methods only for genuinely optional dependencies with valid defaults. | [Spring Framework: Dependency Injection](https://docs.spring.io/spring-framework/reference/core/beans/dependencies/factory-collaborators.html) |
| Responsibility signal | Spring calls a large constructor argument list a code smell that commonly means a class has too many responsibilities; refactor toward separation of concerns rather than obscuring the dependency graph. | [Spring Framework: Dependency Injection](https://docs.spring.io/spring-framework/reference/core/beans/dependencies/factory-collaborators.html) |
| Cycles | Constructor-injected cycles are unresolvable and detected at runtime. Remove the cycle by changing the collaboration/design; do not default to setter injection solely to conceal it. | [Spring Framework: Dependency Injection](https://docs.spring.io/spring-framework/reference/core/beans/dependencies/factory-collaborators.html) |

## Testing guidance

| Layer | Source-backed guidance | Source |
| --- | --- | --- |
| Plain Java unit tests | JUnit Jupiter provides the programming and extension model for writing tests. Tests should state observable behavior with clear assertions; do not use a green compile as behavioral proof. | [JUnit 5 User Guide](https://junit.org/junit5/docs/current/user-guide/) |
| Spring unit and integration tests | Spring's test documentation covers both unit-test best practices and integration testing, advocates TDD, and states IoC makes both easier by allowing collaborators to be wired without service locators. Test domain/use-case logic without a Spring context where feasible, then use the context for integration contracts. The second sentence is an implementation inference. | [Spring Framework: Testing](https://docs.spring.io/spring-framework/reference/testing.html) |
| Boot test support | Spring Boot's `spring-boot-starter-test` includes the general Boot test modules, JUnit Jupiter, AssertJ, Hamcrest, and focused support relevant to the application. Prefer the project-managed test starter over ad-hoc test-library versioning. | [Spring Boot: Testing](https://docs.spring.io/spring-boot/reference/testing/) |
| Web integration scope | `@SpringBootTest` uses a mock environment by default rather than starting the server; Spring MVC endpoints can be exercised with `MockMvc`. Use a real random port only when the HTTP server boundary is part of what must be proven. | [Spring Boot: Testing Spring Boot Applications](https://docs.spring.io/spring-boot/reference/testing/spring-boot-applications.html) |
| Test configuration isolation | `@TestConfiguration` can customize test configuration and is not picked up by component scanning when used as a top-level test configuration. Keep test-only wiring explicit and scoped to the test that needs it. | [Spring Boot: Testing Spring Boot Applications](https://docs.spring.io/spring-boot/reference/testing/spring-boot-applications.html) |

## Adoption boundaries for the local skills

- Treat SOLID as a review lens, not a source of mandatory boilerplate. Preserve
  public contracts unless the requested change requires an intentional versioned
  migration.
- Prefer constructor-injected collaborators and focused types, but first inspect
  the real call graph, existing test seams, and framework constraints.
- Use the smallest test layer that proves the requested behavior. Record
  compilation, unit, integration/context, web, device/emulator, and production
  evidence as distinct gates.
- These sources guide implementation only. They do not authorize edits to
  orchestration, agents, hooks, routing, project rules, credentials, or external
  services.
