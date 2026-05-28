export const ALPHANUMERIC_PATTERN = /^[0-9A-Z]+$/i

export const ALPHANUMERIC_LENGTH_PATTERN = (length: number) =>
  new RegExp(`^[0-9A-Z]{${length}}$`, 'i')
