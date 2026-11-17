# Autonomous run

For "run until done", "I'm going to bed", or a long task with a goal a command can check.

**Outcome:** the goal met, proven by a command, without the user in the loop.

1. Write the goal as a check that fails today and passes only when the work is done: the project's check command, a test file, a script. Run it once and see it fail. A goal no command can check doesn't qualify; do it with the matching playbook instead.
2. Pick a round limit that fits the work (each round is one attempt to stop), at most 50.
3. Start the loop from the repository: `harness loop start --max <N> --done "<check command>" "<goal in one sentence>"`.
4. Work with the matching playbook. Each time you stop, the loop hook runs the check: when it fails, you get the goal and its output back and keep going. Read the output; don't repeat a failed approach (`principles/attack-the-premise.md`).
5. The approval boundary still holds: no push, merge, deploy, or shared data without approval. If the goal needs one, stop with `harness loop stop` and report.

**Stop:** when the check passes (the hook ends the loop) or the round limit is reached.

**Reply:** what was done, the passing check output, the rounds used, and anything left open. If the limit was reached, where it stands and why.
