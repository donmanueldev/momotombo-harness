import { TaxIdErrorCode } from '../types/validation-error'

export const ptMessages = {
  [TaxIdErrorCode.REQUIRED]: 'O documento fiscal é obrigatório',
  [TaxIdErrorCode.INVALID_LENGTH]: 'Comprimento inválido',
  [TaxIdErrorCode.INVALID_CHARACTERS]: 'Caracteres inválidos',
  [TaxIdErrorCode.INVALID_FORMAT]: 'Formato inválido',
  [TaxIdErrorCode.INVALID_CHECKSUM]: 'Checksum inválido',
  [TaxIdErrorCode.INVALID_COUNTRY]: 'País inválido'
}
