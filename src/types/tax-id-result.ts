import type { TaxIdValidationError } from './validation-error'

export interface TaxIdValidationResult {
  valid: boolean
  normalized: string
  formatted?: string
  country?: string
  errors: TaxIdValidationError[]
}
