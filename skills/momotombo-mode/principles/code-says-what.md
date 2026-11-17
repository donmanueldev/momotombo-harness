# Code says what

**When:** writing any code.

- Names carry the what. If code needs a comment to explain what it does, rename, extract, or restructure it until it doesn't.
- A comment is allowed only for a why the code cannot show: a platform or vendor bug, a protocol rule, a business rule from outside the code, a license header, a public API contract, or a link to the issue behind a constraint.
- No narration, section banners, restated JSDoc, commented-out code, or "TODO later". No comments inside tests: the test name says what it checks.
- A lint suppression needs a reason the rule is wrong here; a correctness or safety rule is never suppressed.

**Bad:** `// loop over users and send email`. **Good:** `notifyUnverifiedUsers(users)`.
