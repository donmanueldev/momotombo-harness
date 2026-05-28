import { detectNiRuc } from './detect'
import { formatNiRuc } from './format'
import { niMask } from './mask'
import { niMetadata } from './metadata'
import { normalizeNiRuc } from './normalize'
import { validateNiRuc } from './validate'

export const niConfig = {
  country: 'NI',
  metadata: niMetadata,
  mask: niMask,
  normalize: normalizeNiRuc,
  format: formatNiRuc,
  detect: detectNiRuc,
  validate: validateNiRuc
}
