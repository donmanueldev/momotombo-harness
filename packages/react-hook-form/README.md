# @arev/tax-id-validator-react-hook-form

React Hook Form adapter for [`@arev/tax-id-validator`](../..).

## Installation

```bash
pnpm add @arev/tax-id-validator @arev/tax-id-validator-react-hook-form react-hook-form
```

## Usage

```tsx
import { useForm } from 'react-hook-form'
import { validateTaxIdField } from '@arev/tax-id-validator-react-hook-form'

function Form() {
  const { register, handleSubmit } = useForm()

  return (
    <form onSubmit={handleSubmit(console.log)}>
      <input {...register('ruc', { validate: validateTaxIdField('NI') })} />
      <button type="submit">Submit</button>
    </form>
  )
}
```

## API

### `validateTaxIdField(country: 'NI')`

Returns a React Hook Form `validate` function:

- `true` when the tax ID is valid
- First error message string when invalid
