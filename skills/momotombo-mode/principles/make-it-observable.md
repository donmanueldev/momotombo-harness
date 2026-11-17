# Make it observable

**When:** shipping behavior that runs in production: a job, a webhook, an integration, a payment, an AI call, a flow users depend on.

- Decide first what "working" means in production, then emit the signal that answers it: a structured log for "what happened to this request", a metric for "how often and how fast", a trace for "where did the time go".
- Log events, not prose: one entry per outcome with the action, the record id, the result, and the duration. Use the stack's logger and its context, never `echo`, `dd`, `print`, or `console.log`.
- Failures that need a human reach the error tracker with enough context to act. An error that is caught and only logged is invisible.
- Every log line follows `secrets-and-pii-never-logged.md`.
- Check the signal exists: trigger the path locally and read the log entry or the reported error.

**Bad:** a webhook handler that catches every exception and returns 200. **Good:** it logs `{ event: 'webhook.processed', provider, eventId, outcome, ms }` and reports failures to the error tracker.
