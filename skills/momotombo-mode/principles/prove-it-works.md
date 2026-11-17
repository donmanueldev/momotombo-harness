# Prove it works

**When:** before saying anything is done or fixed.

- Verify against the real artifact: run the feature, hit the endpoint, read the actual value, open the screen. "It compiles" and "tests pass" are not proof the user's problem is solved.
- Check on the surface the behavior lives on. A unit test doesn't prove a screen works.
- Report exactly what ran and what it showed. Anything you didn't run is "not verified".

**Bad:** "Fixed, the tests pass." **Good:** "Fixed: the failing test now passes (output below), and saving the form with an empty title shows the inline error."
