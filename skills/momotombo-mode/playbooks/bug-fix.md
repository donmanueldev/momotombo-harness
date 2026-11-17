# Bug fix

**Outcome:** the bug no longer happens, a test proves it, and the root cause is gone.

1. Reproduce it on the surface where it happens: the request, command, or screen. If it won't reproduce, tighten the conditions or add temporary logging until it does. Ask the user only for access you can't get.
2. Write the failing test that captures the bug, through the public interface. Run it and see it fail for the right reason.
3. Find the root cause (`principles/fix-root-causes.md`). Form hypotheses and eliminate them with evidence; read the code's history with `git log -p` when the bug is a regression. Every line you ship traces to evidence; a guard that "might help" does not ship.
4. Make the smallest fix at the root (`principles/smallest-change.md`). Remove the temporary logging.
5. Run the failing test (now passing), the tests around the change, and the reproduction on the original surface.

**Stop:** when the repro passes and the affected checks are green. Don't commit unless the user asks.

**Reply:** what was broken, the root cause, the fix, and the evidence: the test failing before and passing after, verbatim.
