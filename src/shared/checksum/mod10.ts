export function mod10CheckDigit(digits: string): number {
  let sum = 0
  let shouldDouble = true

  for (let index = digits.length - 1; index >= 0; index -= 1) {
    let digit = Number.parseInt(digits[index] ?? '0', 10)

    if (shouldDouble) {
      digit *= 2
      if (digit > 9) {
        digit -= 9
      }
    }

    sum += digit
    shouldDouble = !shouldDouble
  }

  return (10 - (sum % 10)) % 10
}

export function validateMod10(value: string, checkDigitIndex = value.length - 1): boolean {
  const digits = value.replace(/\D/g, '')
  if (digits.length <= 1) {
    return false
  }

  const body = digits.slice(0, checkDigitIndex)
  const checkDigit = Number.parseInt(digits[checkDigitIndex] ?? '', 10)
  return mod10CheckDigit(body) === checkDigit
}
