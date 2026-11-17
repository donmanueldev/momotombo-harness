# Smallest change

**When:** sizing any diff, or tempted to add an abstraction, option, layer, or parameter.

- Make the smallest change that solves the problem. Fewer lines beat elegant boilerplate.
- Build for the requirement in front of you, not a future one. No options, hooks, or layers "for later".
- Put a decision in one place and pass the result, instead of repeating the same check in several places.
- No drive-by refactors, renames, or upgrades inside an unrelated change.

**Bad:** a `NotificationStrategyFactory` with one strategy. **Good:** the one function that sends the notification.
