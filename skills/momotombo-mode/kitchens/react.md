# Kitchen: React (web, with Inertia or Vite)

Verified in November 2026 against react.dev (React Compiler, You Might Not Need an Effect), `eslint-plugin-react-hooks` 7.x, `eslint-plugin-react-you-might-not-need-an-effect`, typescriptlang.org and typescript-eslint.io, tailwindcss.com (v4), tanstack.com/query (v5), and inertiajs.com (v3). The ESLint config below was run on a test project: it fails each bad example and passes the good one. Check the installed versions' docs before changing this setup.

## Tools

| Tool | Catches |
|---|---|
| React Compiler | memoizes for you, so new code needs no manual `useMemo`, `useCallback`, or `memo` |
| `eslint-plugin-react-hooks` (`recommended`) | Rules of Hooks, plus the compiler rules as errors: `set-state-in-effect`, `use-memo`, `preserve-manual-memoization`, `purity`, `refs`, `immutability` |
| `eslint-plugin-react-you-might-not-need-an-effect` (`strict`) | effects used for derived state, event logic, chained updates, or passing data to parents |
| TypeScript + `typescript-eslint` `strictTypeChecked` | `any`, unsafe calls, floating promises, needless conditions, non-exhaustive `switch` |
| `@tanstack/eslint-plugin-query` | query keys missing the values the query depends on, unstable query clients |
| Vitest | behavior |

## Setup

1. Ask the user, then install: `npm install -D babel-plugin-react-compiler @babel/core eslint-plugin-react-hooks@latest eslint-plugin-react-you-might-not-need-an-effect typescript-eslint`, plus `@tanstack/eslint-plugin-query` when the project uses TanStack Query.
2. Enable the compiler in `vite.config.*`:
   - `@vitejs/plugin-react` 6 or later (Vite 8): also install `@rolldown/plugin-babel`, and add `babel({ presets: [reactCompilerPreset()] })` next to `react()`, importing `reactCompilerPreset` from `@vitejs/plugin-react` and `babel` from `@rolldown/plugin-babel`.
   - `@vitejs/plugin-react` 5 or earlier: `react({ babel: { plugins: ['babel-plugin-react-compiler'] } })`.
3. TypeScript: pin `"typescript": "~6.0"`. TypeScript 7 (the native `tsc`) ships no API yet, and typescript-eslint supports only up to 6.0. In `tsconfig.json`:
   ```json
   "strict": true,
   "noUncheckedIndexedAccess": true,
   "isolatedModules": true,
   "types": ["vite/client"]
   ```
   Add `"exactOptionalPropertyTypes": true` in new projects.
4. In `eslint.config.*`, add to the exported config array:
   ```js
   import tseslint from 'typescript-eslint';
   import reactHooks from 'eslint-plugin-react-hooks';
   import reactYouMightNotNeedAnEffect from 'eslint-plugin-react-you-might-not-need-an-effect';
   import pluginQuery from '@tanstack/eslint-plugin-query';

   const manualMemo = 'React Compiler memoizes for you. Remove it, or measure and justify it in the PR.';

   ...tseslint.configs.strictTypeChecked,
   { languageOptions: { parserOptions: { projectService: true } } },
   reactHooks.configs.flat.recommended,
   reactYouMightNotNeedAnEffect.configs.strict,
   ...pluginQuery.configs['flat/recommended'],
   {
     rules: {
       'react-hooks/exhaustive-deps': 'error',
       '@typescript-eslint/switch-exhaustiveness-check': 'error',
     },
   },
   {
     ignores: ['**/components/ui/**'],
     rules: {
       'no-restricted-imports': ['error', { paths: [{ name: 'react', importNames: ['useMemo', 'useCallback', 'memo'], message: manualMemo }] }],
       'no-restricted-properties': ['error',
         { object: 'React', property: 'useMemo', message: manualMemo },
         { object: 'React', property: 'useCallback', message: manualMemo },
         { object: 'React', property: 'memo', message: manualMemo },
       ],
     },
   },
   ```
   The memoization ban skips `components/ui/`, where shadcn/ui generates components that use `React.useMemo`. Leave out the TanStack line when the project doesn't use it. Run `npx eslint --print-config <a .tsx file>` and confirm `react-hooks/set-state-in-effect` and `react-hooks/use-memo` are `error`.
5. Scripts in `package.json`:
   ```json
   "check": "tsc --noEmit && eslint . --max-warnings 0 && vitest run",
   "check:fast": "eslint --cache --max-warnings 0 . && vitest run --changed --passWithNoTests"
   ```
   Drop the `vitest` parts if the project has no Vitest yet, and say so.
6. Existing code: add no new manual memoization. Remove existing `useMemo`, `useCallback`, and `memo` only file by file with tests around them, because removing it can change what the compiler outputs (react.dev). Fix the effects the rules flag the same way. Don't disable the rules.
7. Prove it: add a component that copies a prop into state with `useEffect`, and a `switch` that misses a union member; `npm run check` must fail. Revert them.

## Tailwind CSS v4

- CSS-first config: `@import "tailwindcss";` and design tokens in `@theme { ... }`, with the `@tailwindcss/vite` plugin. No `@tailwind` directives; a `tailwind.config.js` only through `@config` when a legacy project still needs it.
- Custom utilities with `@utility`, not `@layer utilities`. The important modifier goes last (`flex!`), and CSS variables read as `bg-(--brand)`.
- Never build a class name from pieces (`` `bg-${color}-600` ``): Tailwind scans files as plain text and won't generate it. Map each value to a complete class string.
- Use the v4 names: `shadow-xs`, `rounded-sm`, `ring-3`, `outline-hidden`, `shrink-*`, and opacity as `bg-black/50`.
- Style with the project's shadcn/ui components and `@theme` tokens, not one-off values.

## TanStack Query v5

- With Inertia, page data comes from Inertia props and refreshes with partial reloads (`router.reload({ only: [...] })`). TanStack Query is only for client-fetched data that isn't a page prop: live search, polling, infinite lists. Never copy props into its cache.
- Object signature only: `useQuery({ queryKey, queryFn })`. The query key holds every value the `queryFn` uses (the lint plugin checks it).
- v5 names: `isPending` for the first load (`isLoading` means pending and fetching), `gcTime` (not `cacheTime`), `placeholderData` (not `keepPreviousData`), `throwOnError`. `useQuery` has no `onSuccess`/`onError`; react to data in render or in the mutation.
- After a mutation, invalidate in `onSuccess` and return the promise, so `isPending` stays true until the fresh data is in.
- Set `staleTime` deliberately; the default `0` refetches on every mount.

## What this enforces

- Effects only for synchronizing with something outside React (`principles/derive-dont-sync.md`).
- No new manual memoization. react.dev keeps `useMemo` and `useCallback` as an escape hatch (for example a memoized value used as an effect dependency); a justified exception needs an inline `eslint-disable-next-line` with the reason.
- Exhaustive handling of every union member (`principles/type-discipline.md`).
