# Kitchen: Expo (React Native)

Verified against docs.expo.dev for SDK 58 (React Native 0.88) in November 2026: the React Compiler, ESLint, unit testing, typed routes, environment variables, CNG, and MCP guides. Check the docs for the project's SDK before changing this setup.

The Expo plugin skills own the how-to of routing (`expo-router`), native UI (`expo-native-ui`, `expo-ui`), animation (`expo-animation`), data fetching (`expo-data-fetching`), native modules (`expo-module`), builds and updates (`eas-*`), and upgrades (`expo-upgrade`). Software Mansion's `react-native-best-practices` owns performance and its libraries (Reanimated, Gesture Handler, Screens). Use those skills; this kitchen only sets up what rejects bad code.

## Tools

| Tool | Catches |
|---|---|
| React Compiler | memoizes for you; no manual `useMemo`, `useCallback`, `memo` |
| `eslint-config-expo` (via `npx expo lint`) | Expo and React rules; on SDK 55 and later it includes the React Compiler rules |
| `eslint-plugin-react-you-might-not-need-an-effect` (`strict`) | effects used for derived state, event logic, chained updates |
| TypeScript `strict` + typed routes | untyped code and links to routes that don't exist |
| `npx expo install --check`, `npx expo-doctor` | dependency versions that don't match the SDK, broken project config |
| Jest (`jest-expo`) | behavior |

## Setup

1. Install packages with `npx expo install <package>`, never `npm install`, so versions match the SDK.
2. In `app.json`: `"experiments": { "reactCompiler": true, "typedRoutes": true }`.
3. Run `npx expo lint` once to create the ESLint config if there is none.
4. Ask the user, then install `eslint-plugin-react-you-might-not-need-an-effect` and add its `configs.strict` to `eslint.config.js`, plus the `no-restricted-imports` and `no-restricted-properties` rules that ban manual `useMemo`, `useCallback`, and `memo` from `kitchens/react.md`. Below SDK 55, also install `eslint-plugin-react-compiler` and add `reactCompiler.configs.recommended`.
5. `tsconfig.json`: `"strict": true`, and keep the generated `expo-env.d.ts` and `.expo/types` in `include` (typed routes). Git-ignore `expo-env.d.ts`.
6. Tests: `npx expo install jest-expo jest @types/jest --dev` and `"jest": { "preset": "jest-expo" }` in `package.json`. SDK 58 needs Jest 30; test screens with `@testing-library/react-native` 14 or later, not `react-test-renderer`.
7. Scripts in `package.json`:
   ```json
   "check": "tsc --noEmit && expo lint -- --max-warnings 0 && expo install --check && npx expo-doctor && jest",
   "check:fast": "expo lint -- --max-warnings 0 && jest --changedSince=origin/main"
   ```
8. Native folders: with Continuous Native Generation, keep `/ios` and `/android` in `.gitignore` and regenerate them with `npx expo prebuild --clean`.
9. Environment variables: read them only as `process.env.EXPO_PUBLIC_NAME` (static dot access; destructuring and `process.env['NAME']` are not inlined). Git-ignore `.env.local`. `EXPO_PUBLIC_*` values ship in the bundle: never secrets.
10. Verify on a simulator, not only in tests: `npx expo run:ios` (or `run:android`), and `EXPO_UNSTABLE_MCP_SERVER=1 npx expo start` with `expo-mcp` as a dev dependency so the Expo MCP local tools (`automation_find_view`, `automation_tap`, `automation_take_screenshot`) can drive the app.
11. Prove it: add a component that copies a prop into state with `useEffect`, and a `<Link href="/missing">`; `npm run check` must fail. Revert them.

Check native config (`app.json`, config plugins, permissions) against the Expo docs for the project's SDK before changing it.
