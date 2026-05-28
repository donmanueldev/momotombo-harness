import { TaxIdErrorCode } from '../types/validation-error'

export const esMessages = {
  [TaxIdErrorCode.REQUIRED]: 'El documento fiscal es requerido',
  [TaxIdErrorCode.INVALID_LENGTH]: 'Longitud inválida',
  [TaxIdErrorCode.INVALID_CHARACTERS]: 'Caracteres inválidos',
  [TaxIdErrorCode.INVALID_FORMAT]: 'Formato inválido',
  [TaxIdErrorCode.INVALID_CHECKSUM]: 'Checksum inválido',
  [TaxIdErrorCode.INVALID_COUNTRY]: 'País inválido'
}
