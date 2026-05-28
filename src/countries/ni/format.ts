export function formatNiRuc(value: string): string {
  const clean = value.replace(/[\s-]/g, '')

  if (clean.length !== 14) {
    return clean
  }

  return `${clean.slice(0, 3)}-${clean.slice(3, 9)}-${clean.slice(9)}`
}