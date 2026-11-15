# 🎯 Documentation standard

## 💡 Convention

Every project convention is a standalone Markdown file in `docs/`, grouped by area (`backend/`, `frontend/`, `database/`, `mobile/`, `testing/`, …). Each file has these sections, in order: Convention, Benefits, Examples (good and bad), Real world examples, and Related agreements. `AGENTS.md` keeps a map of every doc so agents open only the ones a task needs.

When an agent is corrected on an undocumented convention, run `codely-doc-create` (`/codely-doc-create` in Claude Code, `$codely-doc-create` in Codex) to turn the correction into a new doc. Then review it like code.

The seeded docs are Momotombo defaults. Edit them to match this project, or delete the ones that don't apply, and keep the `AGENTS.md` map in sync.

## 🏆 Benefits

- Agents load one short doc instead of the whole knowledge base, which cuts tokens and guessing.
- A correction made once stays fixed for every agent (Claude and Codex) and every teammate.
- Each convention change is reviewable in a PR.

## 👀 Examples

### ✅ Good: one convention per file with good and bad examples

```markdown
# 🎯 Thin controllers

## 💡 Convention
Controllers validate (Form Request), call one Action, and return a response.

## 🏆 Benefits
- ...

## 👀 Examples
### ✅ Good: ...
### ❌ Bad: ...

## 🧐 Real world examples
- [app/Http/Controllers/ProjectController.php](../../app/Http/Controllers/ProjectController.php)

## 🔗 Related agreements
- [Form Requests and Policies](form-requests-and-policies.md)
```

### ❌ Bad: every convention buried in one long file

```markdown
# Guidelines
## Architecture ...
## Testing ...
## Database ...
```

## 🧐 Real world examples

- Every file in this `docs/` folder.

## 🔗 Related agreements

- `AGENTS.md`: the documentation map.
