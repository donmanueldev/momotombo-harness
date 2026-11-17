# Perf

**Outcome:** a measured improvement against a baseline, explained.

1. Reproduce the slowness and measure a baseline on a realistic input: a timing, a query count, a profile, a bundle size.
2. Find what limits it before changing anything: the profile, the query log (N+1), the render count, the payload size.
3. Change one thing, measure again with the same method, and keep it only if it moves the number.
4. Explain the number: confirm it measured the work you think, and that the result is not noise.

**Stop:** when the target is met or the next win costs more than it gives. Say which.

**Reply:** the baseline, the result, the cause, and the change, as numbers with the method used to get them.
