+++
schema_version = 1
id = "value-params-002"
key = "library"
area = "value-params"
status = "open"
complexity = "advanced"
afk = true
blocked_by = ["value-params-001"]
+++
# Fix value-parameter copies by repair group

## Outcome

Every repair group in the inventory is fixed. Owned and reference-counted parameters are passed by const&, or taken by value and moved when the callee keeps them, and virtual overrides change together with their base declarations.

## Context

Work from background/value-param-inventory.md. Before editing, split this task into one child per repair group (or a few groups per child) through the derived-work contract. This task stays open until they are done. The brief's Testing section names the main risk: a parameter changed to const& that the callee keeps after the call. Each child lists those cases in its result so the verifier can check them by name. Public header signature changes are acceptable for Gecode 7 (see the brief's Boundaries). For callbacks and other values that are kept, fix the storage path too, including SharedData/SDO construction where needed, so that a by-value parameter reaches storage through an actual move. A std::move that binds to a copying `const&` constructor does not count. Verify this by source inspection; no new permanent test is needed.

## Done when

- [ ] Running the inventory command reports no diagnostics outside the shallow-handle AllowedTypes.
- [ ] The results list every changed parameter that the callee keeps after the call, and how the copy is made.
- [ ] Every by-value-and-move parameter reaches its storage through a move constructor, including through SharedData/SDO, as shown by the source references in the results.

## Validation

- Build build/full and the default Release configuration and run ctest in both.
- Rerun the inventory command.
