---
name: knip
description: Find and verify potentially unused files, exports, and dependencies.
---

# Knip Code Cleanup

Use Knip to find candidates for cleanup. Its output is evidence to investigate, not permission to delete.

## Setup

1. Check if knip is available:
   - Run `npx knip --version` to test
   - If it fails or is very slow, check if `knip` is in package.json devDependencies
   - If it is not installed locally, report that prerequisite and ask before adding a dependency or changing a lockfile

2. Knip does NOT remove unused imports/variables inside files — that's a linter's job. Knip finds unused files, dependencies, and exports across the project.

## Workflow

Always inspect the project and existing configuration before acting on reported issues. Do not create or rewrite a Knip configuration without evidence that the current one is wrong.

### Step 1: Understand the project

- Check what frameworks and tools the project uses (look at package.json)
- Check if a knip config exists (`knip.json`, `knip.jsonc`, or `knip` key in package.json)
- If a config exists, review it for improvements (see Configuration Best Practices below)

### Step 2: Run knip and read configuration hints first

```bash
npx knip
```

Focus on **configuration hints** before anything else. These appear at the top of the output and suggest config adjustments to reduce false positives.

### Step 3: Evaluate configuration hints

Only adjust configuration when the hint maps to a confirmed framework entry point or project convention. Common adjustments:
- Enable/disable plugins for detected frameworks
- Add entry patterns for non-standard entry points
- Configure workspace settings for monorepos

### Step 4: Repeat steps 2-3

Re-run knip after each config change. Repeat until configuration hints are resolved and false positives are minimized.

### Step 5: Address actual issues

Once the configuration is settled, trace each reported issue through repository references, runtime entry points, package scripts, framework conventions, and public API boundaries. Prioritize the investigation in this order:

1. **Unused files** — address these first ("inbox zero" approach removes the most noise)
2. **Unused dependencies** — remove from package.json
3. **Unused devDependencies** — remove from package.json
4. **Unused exports** — remove or mark as internal
5. **Unused types** — remove, or configure `ignoreExportsUsedInFile` (see below)

### Step 6: Re-run and repeat

Re-run knip after each batch of fixes. Removing unused files often exposes newly-unused exports and dependencies.

## Configuration Best Practices

When reviewing or creating a knip config, follow these rules:

- **Never use `ignore` patterns** — `ignore` hides real issues and should almost never be used. Always prefer specific solutions. Other `ignore*` options (like `ignoreDependencies`, `ignoreExportsUsedInFile`) are fine because they target specific issue types.
- **Many unused exported types?** Add `ignoreExportsUsedInFile: { interface: true, type: true }` — this handles the common case of types only used in the same file. Prefer this over broader ignore options.
- **Remove redundant patterns** — Knip already respects `.gitignore`, so ignoring `node_modules`, `dist`, `build`, `.git` is redundant.
- **Remove entry patterns covered by defaults** — Auto-detected plugins already add standard entry points. Don't duplicate them.
- **Config files showing as unused** (e.g. `vite.config.ts`) — Enable or disable the corresponding plugin explicitly rather than ignoring the file.
- **Dependencies matching Node.js builtins** (e.g. `buffer`, `process`) — Add to `ignoreDependencies`.
- **Unresolved imports from path aliases** — Add `paths` to knip config (uses tsconfig.json semantics).

## Production Mode

Use `--production` to focus on production code only:

```bash
npx knip --production
```

This excludes test files, config files, and other non-production entry points. Do NOT use `project` or `ignore` patterns to exclude test files — use `--production` instead.

## Cleanup Confidence Levels

### Eligible for a proposed deletion:
- Clearly internal exports with no static, dynamic, generated, or public consumer
- Dependencies confirmed unused by manifests, scripts, CI, tooling, and runtime configuration
- Files confirmed orphaned and not framework entry points, generated inputs, or configuration

### Ask first (needs clarification):
- Files that might be entry points or dynamically imported
- Exports that might be part of a public API (index.ts, lib exports)
- Dependencies that might be used via CLI or peer dependencies
- Anything in paths like `src/index`, `lib/`, or files with "public" or "api" in the name

Ask before deleting candidates or changing dependencies. Explain the consumer search and validation that supports each removal.

## Auto-fix

Do not use `--fix` or `--allow-remove-files`. Make reviewed removals as small explicit changes, then rerun the relevant checks.

## Error Handling

If knip exits with code 2 (unexpected error like "error loading file"):
- Check if a config file exists — if not, create `knip.json` in the project root
- Check for known issues at knip.dev
- Review the configuration reference for syntax/option errors
- Run knip again after fixes

## Common Commands

```bash
# Basic run
npx knip

# Production only (excludes test/config entry points)
npx knip --production

# Auto-fix what's safe
npx knip --fix

# Auto-fix including file deletion
npx knip --fix --allow-remove-files

# JSON output for parsing
npx knip --reporter json
```

## Notes

- Watch for monorepo setups — may need `--workspace` flag
- Some frameworks need plugins enabled in config
- Knip does not handle unused imports/variables inside files — use ESLint or Biome for that
