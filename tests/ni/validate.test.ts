import { describe, expect, it } from 'vitest'
import { validateTaxId } from '../../src'
import { TaxIdErrorCode } from '../../src/types/validation-error'

describe('NI RUC Validation', () => {
  it('should validate a valid RUC', () => {
    const result = validateTaxId({
      country: 'NI',
      value: '001-230480-0004A'
    })

    expect(result.valid).toBe(true)
    expect(result.normalized).toBe('0012304800004A')
    expect(result.formatted).toBe('001-230480-0004A')
    expect(result.country).toBe('NI')
    expect(result.errors).toHaveLength(0)
  })

  it('should reject invalid length', () => {
    const result = validateTaxId({
      country: 'NI',
      value: '123'
    })

    expect(result.valid).toBe(false)
    expect(result.errors.some((error) => error.code === TaxIdErrorCode.INVALID_LENGTH)).toBe(
      true
    )
  })

  it('should return localized error messages', () => {
    const result = validateTaxId(
      {
        country: 'NI',
        value: '123'
      },
      { locale: 'es' }
    )

    expect(result.errors[0]?.message).toBe('Longitud inválida')
  })
})
