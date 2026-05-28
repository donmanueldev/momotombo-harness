# @arev/tax-id-validator-react

React hook for [`@arev/tax-id-validator`](../..).

## Installation

```bash
pnpm add @arev/tax-id-validator @arev/tax-id-validator-react react
```

`@arev/tax-id-validator` is a **peer dependency** and must be installed alongside this package.

## Usage

```tsx
import { useTaxId } from '@arev/tax-id-validator-react'

function RucInput({ value }: { value: string }) {
  const result = useTaxId('NI', value)

  return (
    <div>
      <span>{result.formattedValue}</span>
      {!result.valid && <span>{result.errors[0]?.message}</span>}
    </div>
  )
}
```

## API

### `useTaxId(country: 'NI', value: string)`

Returns a memoized object with:

- All fields from `TaxIdValidationResult` (`valid`, `normalized`, `formatted`, `country`, `errors`)
- `formattedValue` — alias from `formatTaxId`

Recomputes when `country` or `value` changes.
