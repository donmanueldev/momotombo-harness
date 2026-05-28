import { registry } from './registry'

export function normalizeTaxId(params: {
  country: keyof typeof registry
  value: string
}) {
  const config = registry[params.country]
  return config.normalize(params.value)
}
