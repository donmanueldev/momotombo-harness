export function weightedSum(
  digits: string,
  weights: number[],
  fromRight = true
): number {
  let sum = 0
  let weightIndex = 0

  if (fromRight) {
    for (let index = digits.length - 1; index >= 0; index -= 1) {
      const digit = Number.parseInt(digits[index] ?? '0', 10)
      const weight = weights[weightIndex % weights.length] ?? 1
      sum += digit * weight
      weightIndex += 1
    }
  } else {
    for (let index = 0; index < digits.length; index += 1) {
      const digit = Number.parseInt(digits[index] ?? '0', 10)
      const weight = weights[weightIndex % weights.length] ?? 1
      sum += digit * weight
      weightIndex += 1
    }
  }

  return sum
}

export function weightedSumCheckDigit(
  digits: string,
  weights: number[],
  modulus: number,
  fromRight = true
): number {
  const sum = weightedSum(digits, weights, fromRight)
  return sum % modulus
}
