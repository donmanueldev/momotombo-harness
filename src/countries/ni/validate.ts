import {
  TaxIdErrorCode,
  type TaxIdValidationError
} from '../../types/validation-error'

export function validateNiRuc(value: string): TaxIdValidationError[] {
  const errors: TaxIdValidationError[] = []

  if (!value) {
    errors.push({
      code: TaxIdErrorCode.REQUIRED
    })
    return errors
  }

  if (value.length !== 14) {
    errors.push({
      code: TaxIdErrorCode.INVALID_LENGTH
    })
  }

  if (!/^[0-9A-Z]+$/i.test(value)) {
    errors.push({
      code: TaxIdErrorCode.INVALID_CHARACTERS
    })
  }

  return errors
}
