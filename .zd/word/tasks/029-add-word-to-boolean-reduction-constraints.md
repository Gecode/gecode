+++
schema_version = 1
id = "word-029"
key = "word-reductions"
area = "word"
status = "done"
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

- [x] Direct APIs implement reduce_and, reduce_or, and reduce_xor for widths 1 through 64 with BoolVar results and normal constant/fixed rewrites.
- [x] AND and OR propagate in both directions at their all-bits and last-unknown seams; XOR propagates parity exactly when the cube representation permits it.
- [x] MiniModel exposes the three reductions as BoolExpr operations without creating a per-bit word representation.
- [x] Focused test/word coverage checks assigned semantics, representative partial propagation, width 1 and 64, aliases/failure, reified Bool composition, cloning, recomputation, and subsumption using existing patterns.

## Validation

- Build the Word and MiniModel components and existing gecode-test target.
- Run focused Word reduction, Word MiniModel, and Word::TestFramework tests.
- Run git diff --check and inspect build/package dependency closure for no new cycle.

## Result

Added direct Word-to-Bool reduce_and, reduce_or, and reduce_xor postings with one standard mixed reduction actor, width-one rewrite through bit channeling, exact cube propagation at the all-bits/last-unknown/parity seams, MiniModel BoolExpr lowering, and registered Gecode-style test/word coverage.

Validation:

- Independent verification PASS. Current focused artifact passed ^Word::Reduction, ^Word::MiniModel, and ^Word::TestFramework. Source review confirmed sound AND/OR decisive and backward propagation, XOR parity behavior at zero/one unknown with no unsound multi-unknown pruning, standard MixBinary lifecycle and honest fixpoint/subsumption, MiniModel ownership/negation composition, ordinary inventories, and no dependency cycle. git diff --check passed.
