# Kitchen: Expo (React Native)

Verified against the Expo React Compiler guide and `eslint-config-expo` (November 2026). Check the project's SDK version docs before changing this setup.

## Tools

| Tool | Catches |
|---|---|
| React Compiler | memoizes for you; no manual `useMemo`, `useCallback`, `memo` |
| `eslint-config-expo` (via `npx expo lint`) | Expo and React rules; on SDK 55 and later it includes the React Compiler rules |
| `eslint-plugin-react-you-might-not-need-an-effect` (`strict`) | effects used for derived state, event logic, chained updates |
| TypeScript `strict` | untyped and unsafe code |
| Jest (`jest-expo`) | behavior |

## Setup

1. Enable the compiler in `app.json`: `"expo": { "experiments": { "reactCompiler": true } }`.
2. Run `npx expo lint` once to create the ESLint config if there is none.
3. Ask the user, then install `eslint-plugin-react-you-might-not-need-an-effect` and add its `configs.strict` to `eslint.config.js`, plus the `no-restricted-imports` and `no-restricted-properties` rules that ban manual `useMemo`, `useCallback`, and `memo` from `kitchens/react.md`. Below SDK 55, also install `eslint-plugin-react-compiler` and add `reactCompiler.configs.recommended`.
4. `tsconfig.json`: `"strict": true`.
5. Scripts in `package.json`:
   ```json
   "check": "tsc --noEmit && expo lint --max-warnings 0 && jest",
   "check:fast": "expo lint --max-warnings 0 && jest --onlyChanged"
   ```
6. Verify on a device or simulator, not only in tests: the Expo MCP tools can boot a simulator, tap, and take screenshots.
7. Prove it: add a component that copies a prop into state with `useEffect`; `npm run check` must fail. Revert it.

Check native config (`app.json`, config plugins, permissions) against the Expo docs for the project's SDK before changing it.
