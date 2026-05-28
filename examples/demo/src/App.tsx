import {
  detectTaxIdCountry,
  formatTaxId,
  validateTaxId
} from '@arev/tax-id-validator'
import { useTaxId } from '@arev/tax-id-validator-react'
import { validateTaxIdField } from '@arev/tax-id-validator-react-hook-form'
import { taxIdSchema } from '@arev/tax-id-validator-zod'
import { useState } from 'react'
import { useForm } from 'react-hook-form'
import { z } from 'zod'

const formSchema = z.object({
  ruc: taxIdSchema('NI')
})

type FormValues = z.infer<typeof formSchema>

export function App() {
  const [value, setValue] = useState('001-230480-0004A')
  const taxId = useTaxId('NI', value)
  const detection = detectTaxIdCountry(value)

  const {
    register,
    handleSubmit,
    formState: { errors }
  } = useForm<FormValues>({
    defaultValues: { ruc: '001-230480-0004A' },
    mode: 'onChange'
  })

  const [submitted, setSubmitted] = useState<string | null>(null)

  const onSubmit = (data: FormValues) => {
    setSubmitted(data.ruc)
  }

  const coreResult = validateTaxId({ country: 'NI', value }, { locale: 'es' })

  return (
    <main className="page">
      <header className="hero">
        <p className="eyebrow">Arev Tax ID Validator</p>
        <h1>Demo NI RUC</h1>
        <p className="lede">
          Prueba del core, React hook, React Hook Form y Zod en un solo flujo.
        </p>
      </header>

      <section className="panel">
        <label htmlFor="ruc-input">RUC</label>
        <input
          id="ruc-input"
          value={value}
          onChange={(event) => setValue(event.target.value)}
          placeholder="001-230480-0004A"
          aria-describedby="ruc-status"
        />

        <dl id="ruc-status" className="status-grid">
          <div>
            <dt>Valid</dt>
            <dd data-valid={taxId.valid}>{taxId.valid ? 'Yes' : 'No'}</dd>
          </div>
          <div>
            <dt>Normalized</dt>
            <dd>{taxId.normalized || '—'}</dd>
          </div>
          <div>
            <dt>Formatted</dt>
            <dd>{taxId.formattedValue || '—'}</dd>
          </div>
          <div>
            <dt>Detected country</dt>
            <dd>{detection?.country ?? '—'}</dd>
          </div>
          <div>
            <dt>Core format</dt>
            <dd>{formatTaxId({ country: 'NI', value })}</dd>
          </div>
        </dl>

        {!taxId.valid && taxId.errors.length > 0 && (
          <ul className="errors" role="alert">
            {taxId.errors.map((error) => (
              <li key={error.code}>{error.message}</li>
            ))}
          </ul>
        )}

        <p className="meta">
          Core locale es: {coreResult.errors[0]?.message ?? 'Sin errores'}
        </p>
      </section>

      <section className="panel">
        <h2>React Hook Form + Zod</h2>
        <form onSubmit={handleSubmit(onSubmit)} noValidate>
          <label htmlFor="ruc-form">RUC (form)</label>
          <input
            id="ruc-form"
            {...register('ruc', {
              validate: validateTaxIdField('NI')
            })}
          />
          {errors.ruc?.message && (
            <p className="field-error" role="alert">
              {errors.ruc.message}
            </p>
          )}
          <button type="submit">Validar</button>
        </form>
        {submitted && <p className="success">Enviado: {submitted}</p>}
      </section>
    </main>
  )
}
