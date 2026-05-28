export function sanitizeTaxId(value: string): string {
  return value.replace(/[\s-]/g, '').toUpperCase()
}
