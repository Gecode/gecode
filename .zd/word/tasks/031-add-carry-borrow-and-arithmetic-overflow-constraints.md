+++
schema_version = 1
id = "word-031"
key = "word-arithmetic-flags"
area = "word"
status = "done"
blocked_by = ["word-030"]
+++
# Add carry, borrow, and arithmetic overflow constraints

## Outcome

Word models can observe and constrain addition carry, subtraction borrow, and signed or unsigned arithmetic overflow through compact BoolVar results.

## Boundaries

- Keep carry and borrow outputs distinct from language-level overflow predicates; use one compact Gecode-style operation selector or overload family rather than a combinatorial API matrix.
- Cover current SMT-LIB overflow meanings for negation, unsigned and signed addition, unsigned and signed multiplication, and signed division overflow; preserve WS_SMTLIB semantics.
- Reuse native Add, Sub, Mult, relations, and existing rewrites where they give the required propagation; do not add widening WordVar support or arbitrary precision.
- Use ordinary mixed Word/Bool actors and focused test/word coverage only.
- Add normal Gecode-style tests for each new propagator through the shared `test/word` infrastructure established by word-002 and register them in the ordinary `gecode-test` inventories.

## Done when

- [x] Public direct APIs expose addition with carry and subtraction with borrow while constraining both the modular Word result and BoolVar flag in all directions.
- [x] Public overflow predicates cover modular negation, unsigned/signed addition, unsigned/signed multiplication, and signed division overflow, with assigned semantics matching the current SMT-LIB definitions.
- [x] MiniModel BoolExpr forms are added where they follow the existing Misc/direct-lowering pattern without duplicating arithmetic expression state.
- [x] Focused tests cover small assigned truth tables, representative partial flag-to-word propagation, zero and signed min/-1 cases, width 1 and 64 seams, aliases/failure, cloning, recomputation, and subsumption at spike depth.

## Validation

- Build Word, MiniModel, and the existing gecode-test target.
- Run focused arithmetic flag/overflow, Word arithmetic regression, Word MiniModel, and Word::TestFramework tests.
- Run git diff --check and verify the documented semantics against the official SMT-LIB FixedSizeBitVectors definitions.

## Result

Added modular addition carry and subtraction borrow result-plus-flag APIs backed by native terminal-state automata, plus a compact WordOverflowType predicate family for signed negation, unsigned/signed addition and multiplication, and signed division overflow. Added MiniModel BoolExpr lowering and normal registered Word overflow tests.

Validation:

- Independent verification PASS after correcting the test oracle to use overflow-safe unsigned sign/magnitude logic at widths 1 through 64. Focused ^Word::Overflow, ^Word::Arithmetic, ^Word::MiniModel, and ^Word::TestFramework tests passed. Source review confirmed carry/borrow backward propagation, all predicate semantics including zero/min/-1/width64, honest QF_BV-derived signed-div wording, MiniModel ownership, ordinary inventories, and git diff --check.
