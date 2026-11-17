# Least privilege

**When:** creating tokens, roles, service accounts, database users, API keys, permissions, or integrations.

- Grant only what the task needs, for the time it needs it: read over write, one resource over all, short-lived over permanent.
- Separate credentials per environment and per service.
- Expose only the fields a client needs; never return whole records or internal ids by default.

**Bad:** an admin API key in the mobile app. **Good:** the app calls your backend, which holds a scoped key server-side.
