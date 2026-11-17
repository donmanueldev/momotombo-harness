# Derive, don't sync

**When:** a value can be computed from other state, or you reach for an effect, watcher, observer, listener, or trigger to keep two values equal.

- Compute derived values where they are read. Don't store a copy and keep it in sync.
- Run logic caused by a user action in the handler of that action, not in a reaction to state changing afterwards.
- Reach for a sync mechanism only to talk to something outside the system (a subscription, the DOM, a device API, an external service).
- Don't add manual memoization; measure first, and prefer what the compiler or framework already does.

**Bad:** storing `fullName` and updating it whenever `firstName` changes. **Good:** compute `fullName` from `firstName` and `lastName` when it's read.
