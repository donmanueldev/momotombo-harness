export function detectNiRuc(value: string): boolean {
  const clean = value.replace(/[\s-]/g, '')
  return /^[0-9A-Z]{14}$/i.test(clean)
}
