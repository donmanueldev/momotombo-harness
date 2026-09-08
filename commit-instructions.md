# Commit Instructions

Use this file when generating a commit message or proposing how to split a change into commits.

## Evidence first

1. Inspect the intended diff and its target branch before suggesting a message.
2. Group files by one user-visible behavior, contract, or maintenance intent.
3. Separate unrelated changes into distinct proposed commits.
4. Report the exact validation that was run; mark unrun checks as not run.

Never infer issue IDs, tests, migrations, or deployment status from filenames alone.

## Message format

Use Conventional Commits in English:

```
type(scope): imperative summary
```

- Keep the subject lowercase after the colon, imperative, specific, and without a final period.
- Prefer 72 characters or fewer for the subject.
- Use a narrow scope that identifies the affected surface, such as `agents`, `sync`, `skills`, `docs`, or `tests`.
- Use one of `feat`, `fix`, `refactor`, `test`, `docs`, `chore`, `build`, `ci`, `perf`, or `revert`.
- Add a body only when the subject cannot convey a material decision, compatibility change, migration, or risk.

When a body is needed:

```
type(scope): imperative summary

Why: <reason for the change>
Validation: <commands actually run>
Risk: <risk and rollback or mitigation>
```

## Commit plan output

When more than one commit is appropriate, provide:

1. The ordered commit groups and the paths in each group.
2. One proposed message per group.
3. The dependency between groups, if ordering matters.
4. The validation that applies to each group or to the complete series.

Do not stage, commit, amend, or push while generating messages. Ask for confirmation before any Git write.

## Examples

- `feat(agents): add native orchestration routing`
- `fix(sync): reject unsafe item names`
- `docs(pr): clarify validation evidence`
- `test(sync): cover Matt skill synchronization`
