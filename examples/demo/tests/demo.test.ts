import {
  detectTaxIdCountry,
  formatTaxId,
  normalizeTaxId,
  validateTaxId
} from '@arev/tax-id-validator'
import { taxIdSchema } from '@arev/tax-id-validator-zod'
import { describe, expect, it } from 'vitest'

describe('example demo integrations', () => {
  it('validates a NI RUC through the core API', () => {
    const result = validateTaxId(
      { country: 'NI', value: '001-230480-0004A' },
      { locale: 'es' }
    )

    expect(result.valid).toBe(true)
    expect(result.normalized).toBe('0012304800004A')
    expect(result.formatted).toBe('001-230480-0004A')
  })

  it('detects NI from a formatted RUC', () => {
    expect(detectTaxIdCountry('001-230480-0004A')).toEqual({
      country: 'NI',
      confidence: 0.9
    })
  })

  it('normalizes and formats through core helpers', () => {
    expect(normalizeTaxId({ country: 'NI', value: '001-230480-0004A' })).toBe(
      '0012304800004A'
    )
    expect(formatTaxId({ country: 'NI', value: '0012304800004A' })).toBe(
      '001-230480-0004A'
    )
  })

  it('validates with the zod adapter', () => {
    expect(taxIdSchema('NI').safeParse('001-230480-0004A').success).toBe(true)
    expect(taxIdSchema('NI').safeParse('123').success).toBe(false)
  })
})
