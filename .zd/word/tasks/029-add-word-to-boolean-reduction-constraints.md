+++
schema_version = 1
id = "word-029"
key = "word-reductions"
area = "word"
status = "open"
blocked_by = []
+++
# Add Word-to-Boolean reduction constraints

## Outcome

Models can post whole-word conjunction, disjunction, and exclusive-or reductions to BoolVar results without channeling every bit.

## Boundaries

- Provide Gecode-style snake_case reduce_and, reduce_or, and reduce_xor semantics over all significant bits of one WordVar.
- Use direct mixed Word/Bool actors or established rewrites when equally strong; do not expose a Boolean array or add a general reduction framework.
- Document that parity normally cannot prune a cube while two or more bits remain unknown.
- Add MiniModel BoolExpr forms only by lowering through the direct posting API and existing BoolExpr machinery.
- Add normal Gecode-style tests through the shared `test/word` infrastructure established by word-002 and register them in the ordinary `gecode-test` inventories.

## Done when

- [ ] Direct APIs implement reduce_and, reduce_or, and reduce_xor for widths 1 through 64 with BoolVar results and normal constant/fixed rewrites.
- [ ] AND and OR propagate in both directions at their all-bits and last-unknown seams; XOR propagates parity exactly when the cube representation permits it.
- [ ] MiniModel exposes the three reductions as BoolExpr operations without creating a per-bit word representation.
- [ ] Focused test/word coverage checks assigned semantics, representative partial propagation, width 1 and 64, aliases/failure, reified Bool composition, cloning, recomputation, and subsumption using existing patterns.

## Validation

- Build the Word and MiniModel components and existing gecode-test target.
- Run focused Word reduction, Word MiniModel, and Word::TestFramework tests.
- Run git diff --check and inspect build/package dependency closure for no new cycle.
