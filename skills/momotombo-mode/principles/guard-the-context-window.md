# Guard the context window

**When:** a task needs reading many files, long logs, or wide searches.

- Send bulk reading to `analyst` and keep only its summary in the main conversation.
- Read the part of a file you need, not the whole file. Search before you open.
- Don't paste large outputs back; quote the lines that matter.

**Bad:** reading 25 files into the main session to find where a value is set. **Good:** `analyst` finds it and returns the three `file:line` locations.
