# Kitchen: Python

Verified against the ruff and pyright docs (November 2026).

## Tools

| Tool | Catches |
|---|---|
| ruff (format + lint) | formatting, bugs, outdated syntax, security smells (`S` rules) |
| pyright `strict` | type errors |
| pytest | behavior |

## Setup

1. Ask the user, then install: `uv add --dev ruff pyright pytest`.
2. In `pyproject.toml`:
   ```toml
   [tool.ruff.lint]
   select = ["E", "F", "I", "B", "UP", "SIM", "RUF", "S"]

   [tool.ruff.lint.per-file-ignores]
   "tests/**" = ["S101"]

   [tool.pyright]
   typeCheckingMode = "strict"
   ```
3. One check command, in the project's task runner or a `Makefile`:
   - check: `uv run ruff format --check . && uv run ruff check . && uv run pyright && uv run pytest`
   - fast check: `uv run ruff check . && uv run pytest --lf`
4. Name both in the project's `AGENTS.md`.
5. Prove it: add a function with an untyped parameter and a bare `except:`; the check must fail. Revert it.
