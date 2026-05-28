# @arev/tax-id-validator-zod

Zod integration for [`@arev/tax-id-validator`](../..).

## Installation

```bash
pnpm add @arev/tax-id-validator @arev/tax-id-validator-zod zod
```

`@arev/tax-id-validator` is a **peer dependency** and must be installed alongside this package.

## Usage

```ts
import { z } from 'zod'
import { taxIdSchema } from '@arev/tax-id-validator-zod'

const schema = z.object({
  ruc: taxIdSchema('NI')
})

schema.parse({ ruc: '001-230480-0004A' })
```

## API

### `taxIdSchema(country: 'NI')`

Returns a `z.ZodString` with custom validation. All validation errors from the core library are added as Zod issues with localized messages.
