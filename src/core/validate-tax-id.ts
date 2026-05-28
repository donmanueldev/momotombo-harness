import { resolveErrorMessage } from './resolve-error-message'
import { registry } from './registry'
import type { ValidateTaxIdOptions } from '../types/validation-options'

export function validateTaxId(
  params: {
    country: keyof typeof registry
    value: string
  },
  options?: ValidateTaxIdOptions
) {
  const locale = options?.locale ?? 'es'
  const config = registry[params.country]
  const normalized = config.normalize(params.value)
  const errors = config.validate(normalized).map((error) => ({
    ...error,
    message: resolveErrorMessage(error.code, locale)
  }))

  return {
    valid: errors.length === 0,
    normalized,
    formatted: config.format(normalized),
    country: params.country,
    errors
  }
}
