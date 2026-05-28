import { useMemo } from 'react'
import { formatTaxId, validateTaxId } from '@arev/tax-id-validator'

export function useTaxId(country: 'NI', value: string) {
  return useMemo(() => {
    const validation = validateTaxId({
      country,
      value
    })

    return {
      ...validation,
      formattedValue: formatTaxId({
        country,
        value
      })
    }
  }, [country, value])
}
