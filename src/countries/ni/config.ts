import { normalizeNiRuc } from './normalize'
import { validateNiRuc } from './validate'

export const niConfig = {
  country: 'NI',

  label: 'RUC',

  placeholder: '001-230480-0004A',

  normalize: normalizeNiRuc,

  validate: validateNiRuc
}