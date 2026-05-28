import { registry } from './registry'

export function formatTaxId(params: {
  country: keyof typeof registry
  value: string
}) {
  const config = registry[params.country]
  return config.format(params.value)
}
