# Make it observable

**When:** shipping a job, webhook, integration, payment, AI call, or flow users depend on.

- Decide what "working in production" means, then emit the signal that answers it: one structured log per outcome (action, record id, result, duration) through the stack's logger, and failures that need a human sent to the error tracker. A caught-and-only-logged error is invisible.
- Trigger the path locally and read the entry. Logs follow `secrets-and-pii-never-logged.md`.

**Bad:** a webhook handler that catches every exception and returns 200. **Good:** it logs `{ event: 'webhook.processed', eventId, outcome, ms }` and reports failures.
