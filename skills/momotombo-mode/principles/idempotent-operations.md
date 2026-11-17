# Idempotent operations

**When:** designing commands, jobs, webhooks, migrations, lifecycle steps, or anything that may run twice after a crash or retry.

- Converge to the same end state no matter how many times or how partially it ran before.
- Use natural keys, upserts, and "create if missing" instead of blind inserts. Record what was processed.
- Make side effects (emails, charges, external calls) safe to retry with an idempotency key.
- Wrap multi-step writes in a transaction.

**Bad:** a webhook handler that charges the card every time the provider retries. **Good:** it stores the event id and skips one it has already processed.
