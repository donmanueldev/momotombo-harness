export function normalizeNiRuc(value: string): string {
  return value
    .replace(/[\s-]/g, '')
    .toUpperCase()
}