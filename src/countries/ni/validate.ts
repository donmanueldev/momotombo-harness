export function validateNiRuc(value: string): string[] {
  const errors: string[] = []

  const clean = value.replace(/[\s-]/g, '').toUpperCase()

  if (!clean) {
    errors.push('RUC is required')
    return errors
  }

  if (clean.length !== 14) {
    errors.push('RUC must contain 14 characters')
  }

  if (!/^[0-9A-Z]+$/.test(clean)) {
    errors.push('RUC contains invalid characters')
  }

  return errors
}