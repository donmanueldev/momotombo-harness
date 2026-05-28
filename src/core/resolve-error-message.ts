import { enMessages } from '../i18n/en'
import { esMessages } from '../i18n/es'
import { ptMessages } from '../i18n/pt'
import type { TaxIdLocale } from '../types/locale'
import type { TaxIdErrorCode } from '../types/validation-error'

const locales = {
  es: esMessages,
  en: enMessages,
  pt: ptMessages
}

export function resolveErrorMessage(
  code: TaxIdErrorCode,
  locale: TaxIdLocale = 'es'
): string {
  return locales[locale][code]
}
