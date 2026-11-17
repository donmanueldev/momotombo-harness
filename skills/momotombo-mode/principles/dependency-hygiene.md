# Dependency hygiene

**When:** adding, upgrading, or removing a package.

- Prefer no dependency: use the language, the framework, or a package the project already has.
- Ask the user before adding or upgrading one. Check it is maintained, widely used, and its license fits.
- Pin versions through the lockfile and commit it. Run the ecosystem's audit (`npm audit`, `composer audit`, `pip-audit`, …) after changes.
- Remove packages your change made unused.

**Bad:** adding a date library to format one date. **Good:** the platform's built-in date formatting.
