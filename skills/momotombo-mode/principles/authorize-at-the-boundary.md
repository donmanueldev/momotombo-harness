# Authorize at the boundary

**When:** any input from outside, or any action on protected data.

- Validate every input on the server, at the entry point, against an explicit schema or rule set. Client-side validation is for UX only.
- Authorize every action on the server, per resource and per user: "may this user do this to this record?". Hiding a button is not authorization.
- Scope every query to what the user may see; never trust an id from the client to mean access.
- Use the framework's parameterized queries, escaping, and CSRF protection. Never build queries, commands, or HTML by string concatenation.

**Bad:** `Order::find($request->id)` for any logged-in user. **Good:** the order is loaded through the user's scope and a policy checks the action.
