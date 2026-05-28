import { render, screen } from '@testing-library/react'
import userEvent from '@testing-library/user-event'
import { describe, expect, it } from 'vitest'
import { App } from '../src/App'

describe('demo app', () => {
  it('renders validation state for the default RUC', () => {
    render(<App />)

    expect(screen.getByText('Demo NI RUC')).toBeInTheDocument()
    expect(screen.getByLabelText(/^RUC$/)).toHaveValue('001-230480-0004A')
    expect(screen.getByText('Yes')).toBeInTheDocument()
    expect(screen.getByText('0012304800004A')).toBeInTheDocument()
  })

  it('shows errors when the RUC is invalid', async () => {
    const user = userEvent.setup()
    render(<App />)

    await user.clear(screen.getByLabelText(/^RUC$/))
    await user.type(screen.getByLabelText(/^RUC$/), '123')

    expect(screen.getByText('No')).toBeInTheDocument()
    expect(screen.getByRole('alert')).toBeInTheDocument()
  })
})
