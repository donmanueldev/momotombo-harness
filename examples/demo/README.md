# Tax ID Validator — Demo

Interactive example that exercises the core library and all integration packages in a single React app.

## What it demonstrates

| Layer | Usage in this app |
|-------|-------------------|
| Core | Live validation, normalization, formatting, country detection |
| `@arev/tax-id-validator-react` | `useTaxId` hook bound to the main input |
| `@arev/tax-id-validator-react-hook-form` | `validateTaxIdField` on a form field |
| `@arev/tax-id-validator-zod` | Schema definition (wired in `App.tsx`) |

## Run locally

From the repository root:

```bash
pnpm install
pnpm example:dev
```

Or from this directory:

```bash
pnpm dev
```

The `predev` script builds the core library and integration packages before starting Vite.

Open **http://localhost:5173**.

## Build for production

```bash
pnpm example:build
pnpm preview
```

## Tests

```bash
pnpm test
```

Includes:

- Integration smoke tests (`tests/demo.test.ts`)
- UI behavior tests (`tests/app.test.tsx`)

## Try these values

| Input | Expected result |
|-------|-----------------|
| `001-230480-0004A` | Valid — normalized `0012304800004A`, country `NI` |
| `123` | Invalid — `INVALID_LENGTH`, message in Spanish |
| `` (empty) | Invalid — `REQUIRED` |

## Project files

```
examples/demo/
├── src/
│   ├── App.tsx       # Main demo UI
│   ├── main.tsx      # React entry
│   └── styles.css
├── tests/
│   ├── demo.test.ts  # Core + Zod integration
│   └── app.test.tsx  # Component tests
├── index.html
└── vite.config.ts
```
