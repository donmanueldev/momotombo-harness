# Refactor

**Outcome:** a simpler structure with exactly the same behavior.

1. Pin the current behavior: make sure tests cover what you'll move. If they don't, add characterization tests through the public interface first.
2. Name the target shape and why it's simpler (`principles/minimize-reader-load.md`, `principles/model-the-domain.md`).
3. For mechanical changes across many files, write a codemod or script and run it (`principles/build-the-lever.md`).
4. Migrate every caller and delete the old path in the same change. No compatibility layer, no "deprecated" leftovers (`principles/migrate-then-delete-legacy.md`).
5. Run the affected tests after each step; they must pass unchanged.

**Stop:** when the old structure is gone and the tests pass without edits to their assertions.

**Reply:** before and after in one or two lines each, the lines removed versus added, and the evidence.
