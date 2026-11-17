# Safe defaults

**When:** adding an endpoint, a feature flag, a config option, a role, or a file upload.

- Deny by default: a new route requires authentication and authorization unless it is deliberately public.
- Fail closed: when a check errors, refuse instead of allowing.
- Uploads: check type and size on the server, store outside the web root or in private storage, never trust the file name.
- Rate-limit actions that can be abused (login, signup, password reset, anything that sends messages or costs money).

**Bad:** a new admin route added without middleware "because it's not linked anywhere". **Good:** it sits behind auth and a policy from the first commit.
