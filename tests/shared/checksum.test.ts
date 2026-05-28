import { describe, expect, it } from 'vitest'
import { mod10CheckDigit, validateMod10 } from '../../src/shared/checksum/mod10'
import { mod11CheckDigit, validateMod11 } from '../../src/shared/checksum/mod11'
import {
  weightedSum,
  weightedSumCheckDigit
} from '../../src/shared/checksum/weighted-sum'

describe('mod10', () => {
  it('computes check digit for a known value', () => {
    expect(mod10CheckDigit('7992739871')).toBe(3)
  })

  it('validates a value with correct check digit', () => {
    expect(validateMod10('79927398713')).toBe(true)
  })
})

describe('mod11', () => {
  it('computes check digit with default weights', () => {
    expect(mod11CheckDigit('123456789')).toBe(7)
  })

  it('validates a value with correct check digit', () => {
    expect(validateMod11('1234567897')).toBe(true)
  })
})

describe('weightedSum', () => {
  it('sums digits with weights from right to left', () => {
    expect(weightedSum('1234', [2, 3], true)).toBe(24)
  })

  it('computes check digit with modulus', () => {
    expect(weightedSumCheckDigit('1234', [2, 3], 11, true)).toBe(2)
  })
})
