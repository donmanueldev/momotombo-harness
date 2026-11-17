# Secrets and PII are never logged

**When:** writing logs, errors, exceptions, analytics events, traces, AI prompts, or anything sent to a third party.

- Never log or send secrets (tokens, passwords, keys) or personal data (emails, phones, addresses, documents, health or payment data).
- Log identifiers and context instead: the record id, the action, the outcome.
- Secrets live in environment or secret storage, never in code, config committed to git, or the client bundle.
- Error responses to users never include stack traces, queries, or internals.

**Bad:** `log.info('login failed', { email, password })`. **Good:** `log.info('login failed', { userId, reason: 'bad_password' })`.
