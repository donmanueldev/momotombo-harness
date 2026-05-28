import { validateTaxId } from '@arev/tax-id-validator'

export function validateTaxIdField(country: 'NI') {
  return (value: string) => {
    const result = validateTaxId({
      country,
      value
    })

    if (!result.valid) {
      return result.errors[0]?.message
    }

    return true
  }
}
