# Kitchen: React (web, with Inertia or Vite)

Verified against react.dev (React Compiler), `eslint-plugin-react-hooks` 7.x, and `eslint-plugin-react-you-might-not-need-an-effect` (November 2026). Check the installed versions' docs before changing this setup.

## Tools

| Tool | Catches |
|---|---|
| React Compiler | memoizes for you, so manual `useMemo`, `useCallback`, and `memo` are not needed |
| `eslint-plugin-react-hooks` (`recommended`) | Rules of Hooks, plus the compiler rules: `set-state-in-effect`, `use-memo`, `preserve-manual-memoization`, `purity`, `refs`, `immutability` |
| `eslint-plugin-react-you-might-not-need-an-effect` (`strict`) | effects used for derived state, event logic, chained updates, or passing data to parents |
| TypeScript `strict` + `typescript-eslint` | untyped and unsafe code |
| Vitest | behavior |

## Setup

1. Ask the user, then install: `npm install -D babel-plugin-react-compiler eslint-plugin-react-hooks@latest eslint-plugin-react-you-might-not-need-an-effect`.
2. Enable the compiler in `vite.config.*`:
   - With `@vitejs/plugin-react` 6 or later, also install `@rolldown/plugin-babel` and add `babel({ presets: [reactCompilerPreset()] })` next to `react()`, importing `reactCompilerPreset` from `@vitejs/plugin-react`.
   - With an older version: `react({ babel: { plugins: ['babel-plugin-react-compiler'] } })`.
3. In `eslint.config.*`, add this to the exported config array (tested: it fails a component that copies props into state with an effect, and any manual `useMemo`, `useCallback`, or `memo`):
   ```js
   import reactHooks from 'eslint-plugin-react-hooks';
   import reactYouMightNotNeedAnEffect from 'eslint-plugin-react-you-might-not-need-an-effect';

   const manualMemo = 'React Compiler memoizes for you. Remove it, or measure and justify it in the PR.';

   reactHooks.configs.flat.recommended,
   reactYouMightNotNeedAnEffect.configs.strict,
   {
     rules: {
       'react-hooks/exhaustive-deps': 'error',
       'no-restricted-imports': ['error', { paths: [{ name: 'react', importNames: ['useMemo', 'useCallback', 'memo'], message: manualMemo }] }],
       'no-restricted-properties': ['error',
         { object: 'React', property: 'useMemo', message: manualMemo },
         { object: 'React', property: 'useCallback', message: manualMemo },
         { object: 'React', property: 'memo', message: manualMemo },
       ],
     },
   },
   ```
   Run `npx eslint --print-config <a .tsx file>` and confirm `react-hooks/set-state-in-effect` and `react-hooks/use-memo` are `error`.
4. `tsconfig.json`: `"strict": true`.
5. Scripts in `package.json`:
   ```json
   "check": "tsc --noEmit && eslint . --max-warnings 0 && vitest run",
   "check:fast": "eslint --cache --max-warnings 0 . && vitest run --changed"
   ```
   Drop the `vitest` parts if the project has no Vitest yet, and say so.
6. Existing code: remove manual `useMemo`, `useCallback`, and `memo` (a codemod or a scripted pass, per `principles/build-the-lever.md`), and fix the effects the rules flag, file by file. Don't disable the rules.
7. Prove it: add a component that copies a prop into state with `useEffect`; `npm run check` must fail. Revert it.

## What this enforces

- Effects only for synchronizing with something outside React (`principles/derive-dont-sync.md`).
- No manual memoization: the lint rule bans it. A measured exception needs an inline `eslint-disable-next-line` with the reason.
