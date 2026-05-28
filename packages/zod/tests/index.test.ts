import { describe, expect, it } from 'vitest'
import { taxIdSchema } from '../src/index'

describe('taxIdSchema', () => {
  it('accepts a valid NI RUC', () => {
    const result = taxIdSchema('NI').safeParse('001-230480-0004A')
    expect(result.success).toBe(true)
  })

  it('rejects an invalid NI RUC', () => {
    const result = taxIdSchema('NI').safeParse('123')
    expect(result.success).toBe(false)
  })

  it('returns localized error messages', () => {
    const schema = taxIdSchema('NI')
    const result = schema.safeParse('123')

    if (result.success) {
      throw new Error('Expected validation to fail')
    }

    expect(result.error.issues[0]?.message).toBeDefined()
  })
})
