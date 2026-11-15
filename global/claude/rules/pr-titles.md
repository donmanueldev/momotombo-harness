# Pull Request Rules

## Language
- PR **titles must always be written in English**, even if the conversation, the commits discussion or the PR description are in Spanish.
- The PR **description/body is written in Spanish** (summary, context, testing notes, checklist).
- Never translate code identifiers, branch names, commit prefixes or technical terms.

## Title format (Conventional Commits)

```
<type>(<optional scope>): <short imperative description>
```

- Imperative mood, present tense: `add`, `fix`, `update`, `remove` (not `added`, `fixes`, `adding`).
- Lowercase after the colon, no trailing period.
- Maximum 72 characters.
- Describe the *what/why*, not the *how*. Be specific, avoid vague titles like `fix bug` or `changes`.
- Scope is optional and short (module, screen, package): `auth`, `api`, `ui`, `deps`.
- Breaking changes: add `!` after the type/scope (`feat(api)!: remove v1 endpoints`) and explain it in the body under `BREAKING CHANGE:`.

### Allowed types
| Type | Use for |
|------|---------|
| `feat` | New feature |
| `fix` | Bug fix |
| `docs` | Documentation only |
| `style` | Formatting, no logic change |
| `refactor` | Code change that neither fixes a bug nor adds a feature |
| `perf` | Performance improvement |
| `test` | Adding or fixing tests |
| `build` | Build system or dependencies |
| `ci` | CI/CD configuration |
| `chore` | Maintenance tasks |
| `revert` | Reverting a previous change |

## Git Flow conventions

| Branch | Target | Title pattern |
|--------|--------|---------------|
| `feature/*` | `develop` | `feat(scope): add user login with google` |
| `bugfix/*` | `develop` | `fix(scope): handle empty response in checkout` |
| `release/x.y.z` | `main` (and back-merge to `develop`) | `release: v1.4.0` |
| `hotfix/*` | `main` (and back-merge to `develop`) | `fix(scope): prevent crash on expired token` |
| `chore/*`, `docs/*`, `refactor/*` | `develop` | `chore(deps): bump expo to 52` |

- Derive the `type` from the branch prefix when possible (`feature/` -> `feat`, `bugfix/` and `hotfix/` -> `fix`).
- Back-merge PRs (`main` -> `develop`) use: `chore: merge main into develop`.
- Include the ticket/issue reference at the end of the title only if the team uses it: `feat(auth): add google login (#123)`.
- Keep one logical change per PR; if the title needs "and", consider splitting the PR.

## Examples

Good:
- `feat(auth): add google sign-in`
- `fix(api): return 404 when route is not found`
- `refactor(ui): extract card component`
- `release: v2.0.0`

Bad:
- `Agrega login con Google` (Spanish)
- `Fixed stuff` (vague, past tense)
- `Feat: Add Login.` (capitalized, trailing period)
- `feature/login` (branch name used as title)

## Before creating a PR
1. Check the branch name and the commits to pick the correct `type` and `scope`.
2. Write the title in English following the format above.
3. Write the description in Spanish.
4. If the title is in Spanish or breaks the format, fix it before opening the PR (`gh pr edit --title "..."` for existing ones).
