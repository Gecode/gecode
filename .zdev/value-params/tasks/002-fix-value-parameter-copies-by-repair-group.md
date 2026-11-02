+++
schema_version = 1
id = "value-params-002"
key = "library"
area = "value-params"
status = "done"
complexity = "advanced"
afk = true
blocked_by = ["value-params-001", "value-params-004", "value-params-005", "value-params-006", "value-params-007", "value-params-008"]
+++
# Fix value-parameter copies by repair group

## Outcome

Every repair group in the inventory is fixed. Owned and reference-counted parameters are passed by const&, or taken by value and moved when the callee keeps them, and virtual overrides change together with their base declarations.

## Context

Work from background/value-param-inventory.md. Before editing, split this task into one child per repair group (or a few groups per child) through the derived-work contract. This task stays open until they are done. The brief's Testing section names the main risk: a parameter changed to const& that the callee keeps after the call. Each child lists those cases in its result so the verifier can check them by name. Public header signature changes are acceptable for Gecode 7 (see the brief's Boundaries). For callbacks and other values that are kept, fix the storage path too, including SharedData/SDO construction where needed, so that a by-value parameter reaches storage through an actual move. A std::move that binds to a copying `const&` constructor does not count. Verify this by source inspection; no new permanent test is needed.

## Done when

- [x] Running the inventory command reports no diagnostics outside the shallow-handle AllowedTypes.
- [x] The results list every changed parameter that the callee keeps after the call, and how the copy is made.
- [x] Every by-value-and-move parameter reaches its storage through a move constructor, including through SharedData/SDO, as shown by the source references in the results.

## Validation

- Build build/full and the default Release configuration and run ctest in both.
- Rerun the inventory command.

## Result

Repair groups G1-G16 landed in children 004-008; the results record lists every kept const& parameter with its copy and every by-value-plus-move path

Validation:

- Inventory rerun at cae26c0d61: 0 diagnostics outside the shallow-handle AllowedTypes
- build/full and Release built; ctest 8/8 and 7/7; uv run misc/check.py all --build-dir build/full exit 0
- Results record in background/value-param-results.md: A1 105, A2 37, A3 109, other 95, move 362 over 708 baseline rows, plus A4 5; selfcheck 0 mismatches
- Independent Codex verifier: pass, no findings (round 4)
