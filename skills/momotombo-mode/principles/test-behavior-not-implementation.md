# Test behavior, not implementation

**When:** writing, changing, or keeping a test.

- Assert what the code's users can observe: the response, the rendered result, the stored record, the event sent.
- A test fails when the behavior breaks and keeps passing when only the internals change.
- One test per user goal through the public interface, plus unit tests for business rules with cases picked by equivalence classes and boundary values. Don't prove the same behavior twice at different levels.
- No tests that can't fail, that only assert a mock was called, or that repeat the implementation. Don't test trivial code.

**Bad:** asserting that `repository.save` was called once. **Good:** asserting the saved order can be fetched with the right total.
