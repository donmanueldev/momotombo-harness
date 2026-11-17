# Subtract before you add

**When:** sequencing an addition, a refactor, or a rewrite.

- First delete what is dead or duplicated: unused code, redundant validators, stale flags, stub references.
- Then build on the simpler base. Adding next to dead weight makes both harder to read.
- Delete what your own change made dead: code, imports, files, config, flags.

**Bad:** adding a new formatter beside two old ones that nothing calls. **Good:** delete the two, then add the one you need.
