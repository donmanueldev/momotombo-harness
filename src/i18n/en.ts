import { TaxIdErrorCode } from '../types/validation-error'

export const enMessages = {
  [TaxIdErrorCode.REQUIRED]: 'Tax ID is required',
  [TaxIdErrorCode.INVALID_LENGTH]: 'Invalid length',
  [TaxIdErrorCode.INVALID_CHARACTERS]: 'Invalid characters',
  [TaxIdErrorCode.INVALID_FORMAT]: 'Invalid format',
  [TaxIdErrorCode.INVALID_CHECKSUM]: 'Invalid checksum',
  [TaxIdErrorCode.INVALID_COUNTRY]: 'Invalid country'
}
