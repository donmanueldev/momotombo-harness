import { z } from 'zod'
import { validateTaxId } from '@arev/tax-id-validator'

export function taxIdSchema(country: 'NI') {
  return z.string().superRefine((value, ctx) => {
    const result = validateTaxId({
      country,
      value
    })

    if (!result.valid) {
      for (const error of result.errors) {
        ctx.addIssue({
          code: z.ZodIssueCode.custom,
          message: error.message
        })
      }
    }
  })
}
