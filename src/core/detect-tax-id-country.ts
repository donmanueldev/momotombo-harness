import { registry } from './registry'

export function detectTaxIdCountry(value: string) {
  for (const [country, config] of Object.entries(registry)) {
    if (config.detect(value)) {
      return {
        country,
        confidence: 0.9
      }
    }
  }

  return null
}
