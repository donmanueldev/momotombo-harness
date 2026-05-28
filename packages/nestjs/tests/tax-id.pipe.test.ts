import { BadRequestException } from '@nestjs/common'
import { describe, expect, it } from 'vitest'
import { TaxIdPipe } from '../src/tax-id.pipe'

describe('TaxIdPipe', () => {
  it('returns normalized value for a valid NI RUC', () => {
    const pipe = new TaxIdPipe('NI')
    expect(pipe.transform('001-230480-0004A', { type: 'body' })).toBe(
      '0012304800004A'
    )
  })

  it('throws BadRequestException for invalid values', () => {
    const pipe = new TaxIdPipe('NI')

    expect(() => pipe.transform('123', { type: 'body' })).toThrow(
      BadRequestException
    )
  })
})
