import { registry } from './registry'

export function validateTaxId(params: {
  country: keyof typeof registry
  value: string
}) {
  const config = registry[params.country]

  const normalized = config.normalize(params.value)

  const errors = config.validate(normalized)

  return {
    valid: errors.length === 0,
    normalized,
    errors
  }
}