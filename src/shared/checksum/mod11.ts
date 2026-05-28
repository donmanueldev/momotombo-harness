export function mod11CheckDigit(
  digits: string,
  weights: number[] = [2, 3, 4, 5, 6, 7, 8, 9]
): number {
  let sum = 0
  let weightIndex = 0

  for (let index = digits.length - 1; index >= 0; index -= 1) {
    const digit = Number.parseInt(digits[index] ?? '0', 10)
    const weight = weights[weightIndex % weights.length] ?? 2
    sum += digit * weight
    weightIndex += 1
  }

  const remainder = sum % 11
  const checkDigit = 11 - remainder

  if (checkDigit === 10) {
    return 0
  }

  if (checkDigit === 11) {
    return 1
  }

  return checkDigit
}

export function validateMod11(
  value: string,
  checkDigitIndex = value.length - 1,
  weights?: number[]
): boolean {
  const digits = value.replace(/\D/g, '')
  if (digits.length <= 1) {
    return false
  }

  const body = digits.slice(0, checkDigitIndex)
  const checkDigit = Number.parseInt(digits[checkDigitIndex] ?? '', 10)
  return mod11CheckDigit(body, weights) === checkDigit
}
