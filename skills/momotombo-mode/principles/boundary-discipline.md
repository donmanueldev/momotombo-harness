# Boundary discipline

**When:** wiring validation, error handling, or a framework or vendor adapter.

- Concentrate guards at the system's edges: user input, network, files, storage, environment, external APIs.
- Parse and validate once at the edge into trusted types; inside, trust them. No defensive re-checks, re-normalizing, or re-sanitizing of data that is already valid.
- Keep business rules in plain functions free of framework and I/O, so they are easy to test.
- Handle errors where they can be handled. Don't wrap everything in try/catch.

**Bad:** every function checks `if (!email || typeof email !== 'string')`. **Good:** the request is validated once into an `Email`; the rest of the code just uses it.
