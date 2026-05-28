import { describe, expect, it } from 'vitest'
import { validateTaxIdField } from '../src/index'

describe('validateTaxIdField', () => {
  it('returns true for a valid NI RUC', () => {
    expect(validateTaxIdField('NI')('001-230480-0004A')).toBe(true)
  })

  it('returns an error message for invalid values', () => {
    const message = validateTaxIdField('NI')('123')
    expect(typeof message).toBe('string')
    expect(message).not.toBe(true)
  })
})
