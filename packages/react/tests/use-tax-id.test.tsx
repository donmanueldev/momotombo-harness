import { renderHook } from '@testing-library/react'
import { describe, expect, it } from 'vitest'
import { useTaxId } from '../src/use-tax-id'

describe('useTaxId', () => {
  it('validates and formats a valid NI RUC', () => {
    const { result } = renderHook(() => useTaxId('NI', '001-230480-0004A'))

    expect(result.current.valid).toBe(true)
    expect(result.current.normalized).toBe('0012304800004A')
    expect(result.current.formattedValue).toBe('001-230480-0004A')
  })

  it('returns errors for invalid values', () => {
    const { result } = renderHook(() => useTaxId('NI', '123'))

    expect(result.current.valid).toBe(false)
    expect(result.current.errors.length).toBeGreaterThan(0)
  })
})
