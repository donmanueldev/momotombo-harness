import type { TaxIdMask } from './tax-id-mask'
import type { TaxIdMetadata } from './tax-id-metadata'
import type { TaxIdValidationError } from './validation-error'

export interface TaxIdConfig {
  country: string
  metadata: TaxIdMetadata
  mask?: TaxIdMask
  normalize(value: string): string
  format(value: string): string
  detect(value: string): boolean
  validate(value: string): TaxIdValidationError[]
}
