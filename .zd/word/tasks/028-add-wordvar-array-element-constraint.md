+++
schema_version = 1
id = "word-028"
key = "word-element"
area = "word"
status = "done"
blocked_by = []
+++
# Add WordVar array element constraint

## Outcome

A standard Gecode element posting selects a same-width WordVar from a WordVarArgs array using a zero-based IntVar index and propagates between the index, candidates, and result without an ITE chain.

## Boundaries

- Follow the existing Int element actor and posting patterns; do not add symbolic memory, table infrastructure, or a new array-expression system.
- All candidate words and the result have one width; reject mixed widths through the established Word exceptions.
- Keep propagation honest for cube domains: pruning unsupported indices and publishing the supported result hull is sufficient; do not claim arbitrary-value domain consistency.
- Use the existing test/word framework with proportionate focused cases; do not add a bespoke executable or exhaustive large-array campaign.
- Add normal Gecode-style tests through the shared `test/word` infrastructure established by word-002 and register them in the ordinary `gecode-test` inventories.

## Done when

- [x] The public direct-posting API supports a WordVarArgs array, zero-based IntVar index, and WordVar result with conventional empty-array, index-range, alias, and width handling.
- [x] One ordinary mixed Int/Word actor prunes impossible index values, narrows the result to the hull of supported candidates, and rewrites or subsumes through the normal lifecycle when the index or selected word becomes fixed.
- [x] Focused shared-framework tests cover assigned semantics, representative partial cubes and duplicate/aliased candidates, invalid index and width cases, failure, cloning, recomputation, and subsumption at implementation-spike depth.
- [x] CMake and Make source inventories and public documentation are updated through the normal Word component paths.

## Validation

- Build GecodeWord and the existing gecode-test target in a coherent Word-enabled configuration.
- Run only the focused Word element selection and Word::TestFramework tests plus a representative existing Int element smoke test.
- Run git diff --check and inspect the actor lifecycle and module inventories.

## Result

Added the standard zero-based WordVar array element constraint as one mixed Int/Word actor. It prunes unsupported index values, narrows the result to the supported candidate cube hull, rewrites a sole candidate to ordinary Word equality, and follows normal Gecode clone/schedule/dispose/subsumption patterns. Added the public API, exception, documentation, build inventories, and a registered Gecode-style test/word element test.

Validation:

- Independent verification PASS. A coherent current CMake GecodeWord/gecode-test artifact passed ^Word::Element, ^Word::TestFramework, and a representative Int element smoke test. Source review confirmed sound cube compatibility and hull propagation, duplicate/result aliases, index pruning, subscriptions, cloning, recomputation, failure, rewrite, and subsumption. git diff --check passed.
