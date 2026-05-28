# @arev/tax-id-validator-nestjs

NestJS pipe for [`@arev/tax-id-validator`](../..).

## Installation

```bash
pnpm add @arev/tax-id-validator @arev/tax-id-validator-nestjs @nestjs/common
```

## Usage

Instantiate the pipe with the target country:

```ts
import { Body, Controller, Post } from '@nestjs/common'
import { TaxIdPipe } from '@arev/tax-id-validator-nestjs'

@Controller('taxpayers')
export class TaxpayersController {
  @Post()
  create(@Body('ruc', new TaxIdPipe('NI')) ruc: string) {
    return { ruc }
  }
}
```

## Behavior

- **Success:** returns the normalized tax ID (e.g. `0012304800004A`)
- **Failure:** throws `BadRequestException` with payload `{ errors: TaxIdValidationError[] }`

## API

### `TaxIdPipe`

```ts
new TaxIdPipe(country: 'NI')
```

Implements NestJS `PipeTransform<string, string>`.
