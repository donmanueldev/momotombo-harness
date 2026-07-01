---
name: diagnosing-bugs
description: Diagnose hard bugs and regressions with a tight feedback loop.
---

# Diagnosing Bugs

Use this skill when the user says something is broken, flaky, slow, or hard to reproduce.

## 1. Build a feedback loop

Create a command that goes red for this exact bug:
- a failing test
- a curl or HTTP script
- a browser script
- a CLI repro
- a throwaway harness

If you cannot make a loop, say so and list what is missing.

## 2. Reproduce and minimize

Run the loop until the failure is clear.
Then cut the repro down one variable at a time until only the load-bearing pieces remain.
Keep it deterministic and fast.

## 3. Rank hypotheses

Write 3 to 5 falsifiable hypotheses before you probe.
Each one should predict what will happen if it is true.

## 4. Probe one variable at a time

Use the debugger or REPL first when available.
Otherwise add targeted logs or measurements.
Avoid blanket logging.

## 5. Fix and verify

Turn the minimized repro into a regression test when the seam is correct.
Apply the fix, rerun the original loop, and remove all temporary instrumentation.
