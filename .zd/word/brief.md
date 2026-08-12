# Word vectors

## Objective

Add fixed-width word-vector variables and their core propagators to Gecode, following Wang et al. where suitable, so models that would otherwise rely on SMT bit-blasting can use Gecode propagation, search, cloning, and tooling.

## Why this area exists

Bit-vector models are commonly handed to SMT solvers, which eventually reduce many of them to Boolean clauses. That route gains SAT learning, but it also expands every word operation into bit-level machinery. Wang, Søndergaard, and Stuckey show that a different representation is useful for a substantial class of instances: represent the known zero and one bits of a fixed-width value by two machine words, and propagate many bits at once with ordinary word operations.

This area brings that representation into Gecode as a proper variable type. The aim is not to replace specialist bit-vector SMT solvers on their hardest benchmarks. It is to make word-vector constraints available in the same engineered environment as Gecode's existing integer, Boolean, set, and float variables: actors, cloning and recomputation, standard search, tracing, tests, documentation, both supported build systems, and installable component libraries.

## Research basis

The starting points in the local constraint-programming knowledge base are:

- `2016-wang-a-bit-vector-solver-with-word-level-propagation`, *A bit-vector solver with word level propagation*. This paper gives the lo/hi trit-vector representation, constant-time logical propagation rules, structural propagators, addition decompositions, composed equality and comparison propagators, and the first evaluation against bit-blasting.
- `2019-wang-wombit-a-portfolio-bit-vector-solver-using-word`, *Wombit: A Portfolio Bit-Vector Solver Using Word-Level Propagation*, DOI `10.1007/s10817-018-9493-1`. This extends the operator coverage to non-linear arithmetic and describes the final portfolio solver.
- Michel and Van Hentenryck, *Constraint Satisfaction over Bit-Vectors*, CP 2012, DOI `10.1007/978-3-642-33558-7_39`. Wang et al. build on this original word-level propagation scheme and correct or replace some of its rules.

The papers combine word-level propagation with SAT explanations and learning. Gecode does not currently have that learning layer. The propagation rules and decompositions transfer directly; the explanation, clause-learning, backjumping, and portfolio mechanisms do not.

## Success

The area succeeds when a Gecode model can:

- declare word variables with a fixed width and partially known bits;
- inspect and restrict their known-zero and known-one masks without leaving an invalid domain behind;
- post the logical, arithmetic, comparison, and structural constraints needed to express the supported fixed-width word theory;
- connect individual word bits to `BoolVar` values;
- branch on word variables with clone-safe, archive-safe choices and no-good literals;
- clone, recompute, trace, print, and dispose spaces containing word variables and actors through the ordinary Gecode interfaces;
- build, test, install, and consume the word module through both CMake and Autoconf/Make; and
- model word expressions through MiniModel as well as direct posting functions; and
- demonstrate the API and intended performance range with documented examples, a large private development corpus, and comparisons against a Boolean decomposition.

Correctness takes priority over matching an SMT solver's scale. Performance work should preserve the central reason for the representation: propagation over a word should normally use a bounded number of native word operations rather than scan or allocate per bit.

## Terms

- **Word vector** is the public Gecode term for the new variable type. It avoids implying arbitrary-length bit vectors and matches the machine-word implementation boundary.
- **Width** is the fixed number of significant bits in a word variable. Bit zero is the least significant bit.
- **Lower mask** (`lo`) contains the bits known to be one.
- **Upper mask** (`hi`) contains the bits that may still be one. A zero in `hi` is therefore a bit known to be zero.
- **Unknown mask** is `hi & ~lo`, restricted to the variable width.
- **Word domain** is the cube of concrete values described by `(lo, hi)`. It is not an arbitrary set or interval of unsigned integers.
- **Assigned** means `lo == hi` after masking to the width.
- **Word-level propagator** updates several bit positions using native word operations. It need not be domain complete over the set of all concrete word values.
- **Decomposition** introduces intermediate word or Boolean variables and posts simpler word-level constraints. It does not mean bit-blasting unless every word is deliberately channelled to separate Boolean bits.
- **Semantics policy** selects edge-case arithmetic behavior without changing the word domain or its ordinary logical and structural operations. SMT-LIB is the default policy.

## Settled decisions

### Representation and invariants

- The first implementation supports widths from 1 through 64 and uses an unsigned 64-bit storage type. Width is immutable and part of the variable's identity.
- Every stored and returned mask is restricted to the declared width. The width-64 mask is handled without a shift by 64.
- A domain is valid exactly when `lo & ~hi == 0` within the width. Updates compute candidate masks, validate them together, and publish them atomically so a failed update cannot expose a half-updated variable.
- Domain narrowing only adds known-one bits to `lo` and removes may-be-one bits from `hi`.
- The variable delta records newly fixed zero and one bits. The initial event model distinguishes assignment from any newly fixed bit; finer events are added only if measurements show that they avoid meaningful propagation work.
- The checked-in generated kernel variable headers remain canonical build inputs. The word `.vis` specification participates in the existing opt-in regeneration and freshness checks.

### Module and API boundary

- Word vectors form an optional module alongside Int, Set, and Float, with a `GECODE_HAS_WORD_VARS` feature guard and a separately installable/linkable `GecodeWord` component.
- The public surface follows established Gecode conventions: `WordVar`, variable arguments and arrays, value/mask accessors, exceptions and limits, views, propagator posting functions, branch options, tracing, and stream output.
- Constants carry an explicit width. Posting rejects width mismatches rather than silently truncating or extending operands.
- Bit channeling to `BoolVar` is part of the core API. It is the explicit bridge for mixed Boolean/word models and for tests that compare word propagation with bit-level models.
- Public names follow established Gecode posting conventions: compact overloaded posting functions and operation or relation enums where the existing modules use them, with descriptive functions for word-specific operations that do not fit an established overload.
- Multiword function and variable names use `snake_case`. Type names use `CamelCase`. This applies to public APIs, MiniModel nodes, internal actors, tests, and examples; established one-word Gecode names remain unchanged.
- MiniModel word expressions are part of the first releasable module. They are implemented over the direct posting API after its contracts stabilize, so expression syntax does not determine the variable representation or propagator internals.

### Semantics

- Operator names and concrete-value semantics default to SMT-LIB's quantifier-free bit-vector theory, including its total semantics for division, remainder, modulus, zero divisors, and signed overflow.
- Edge-case arithmetic semantics are selected through an explicit `WordSemantics` policy in the relevant direct posting and MiniModel APIs. The initial implementation provides the SMT-LIB policy and keeps the policy boundary open for later alternatives; it does not add a second unmotivated policy merely to exercise the abstraction.
- Semantics selection does not alter `WordVar` domains, logical operations, structural operations, relations whose meaning is policy-independent, or branch choices. It is carried only by operations whose concrete result can differ between policies.
- Arithmetic wraps modulo `2^width`.
- Signed comparisons and arithmetic interpret the most significant bit as a two's-complement sign bit; the stored domain remains the same unsigned mask pair.
- Logical operators include complement, conjunction, disjunction, exclusive-or, nand, nor, and equivalence/xnor, with binary and useful n-ary posting forms.
- Relations include equality and disequality plus signed and unsigned ordering, with direct and reified forms.
- Structural operations include constant and variable shifts, constant rotations, concatenation, extraction, repetition, zero extension, and sign extension.
- Arithmetic operations include if-then-else, addition, unary negation, subtraction, multiplication, unsigned and signed division, remainder, and signed modulus.
- Direct word-level propagators are preferred for the basic logical, relational, comparison, and structural operations described by Wang et al. Arithmetic may use word-level decompositions with intermediate variables when that makes the propagator smaller and preserves useful propagation.
- Posting code may rewrite a derived operator to a smaller primitive set when the rewrite preserves fixed-width semantics and does not collapse into unconditional bit-blasting.

### Search and engineering scope

- Standard word branching chooses an unassigned variable, selects an unknown bit, and creates complementary zero/one alternatives. Initial bit selectors cover least-significant, most-significant, and random unknown bits; variable selectors follow the existing size, degree, AFC, action, and CHB patterns where their contracts apply.
- Choices contain only the variable position, bit position, and alternative data needed by `commit`. They support archives, recomputation, printing, and no-good literals.
- The module includes trace deltas and variable printing. Gist inspection support is included if it can use the normal variable-inspector extension point without making the word library depend on Qt.
- FlatZinc integration is outside this area because FlatZinc has no native word-vector type. A later frontend or model-IR extension can target the public word API.
- SAT explanations, clause learning, conflict-directed backjumping, bit-blasting fallback, machine-learned solver selection, and the Wombit portfolio are outside this area.
- Arbitrary-precision word vectors are outside this area. A future design can compose machine words, but the representation, events, and propagators must be reconsidered rather than hidden behind the 64-bit API.

## Constraint inventory

The implementation is complete only when each row has public posting overloads, propagation tests, assigned-value semantics tests, failure tests, clone/recomputation coverage, and documentation.

| Family | Operations | Initial implementation shape |
| --- | --- | --- |
| Domain and channel | mask restriction, assignment, bit-to-`BoolVar` channel | Native variable operations and binary channel propagators |
| Relations | `=`, `!=`, signed/unsigned `<`, `<=`, `>`, `>=`, reified variants | Direct equality/disequality and composed or direct comparison propagators |
| Logic | not, and, or, xor, nand, nor, xnor/equivalence | Constant-time native word rules; n-ary forms may fold through intermediates |
| Conditional | Boolean and word-mask if-then-else | Decomposition through sign extension where useful, following Wang et al. |
| Linear arithmetic | add, unary negation, subtract | Carry-vector decomposition over word primitives, with a fused propagator considered only after measurement |
| Non-linear arithmetic | multiply, unsigned/signed divide, remainder, signed modulus | Word-level decompositions based on multiplication and conditional shifted terms; SMT-LIB edge cases selected through `WordSemantics` |
| Fixed structural | constant shifts and rotations, extract, concatenate, repeat, zero/sign extend | Masked equality-style propagators |
| Variable structural | shifts by a word variable | Staged or decomposed propagators that retain word-level state rather than eagerly exposing every bit |

Nand, nor, xnor, greater-than, and related dual operations do not require unique actor classes when a rewrite to a primitive word propagator is equally strong. “All propagators” means semantic and propagation coverage of the inventory, not one class per surface operator.

## Implementation shape

### 1. Establish the variable kernel

- Add the word feature guard, library component, generated variable specification, module headers, source inventory, export/install rules, and disabled-module build path.
- Implement width and mask value types, `WordVarImp`, modification events, deltas, subscriptions, cloning, recovery, public variables, arrays, constant/basic views, and printing.
- Add exhaustive domain-operation tests before posting non-trivial constraints. These tests cover every partial domain for small widths, failed atomic updates, width 1 and width 64, copying, disposal, and generated-header freshness.

### 2. Add the Boolean bridge and search

- Implement bit extraction/channeling to `BoolVar`, including constants and already-fixed bits.
- Add variable and value selection, branch/assign posting, archive reconstruction, no-good literals, print callbacks, AFC/action/CHB integration, and trace deltas.
- Verify search on models containing only words and on mixed word/Boolean models under cloning and recomputation.

### 3. Add relations and logical operations

- Implement equality, disequality, reification, Boolean-controlled if-then-else, and the logical operator family.
- Translate Wang et al.'s lo/hi rules into views and ordinary Gecode actor lifecycles. Validate every rule independently rather than copying formulas without checking their fixed-width masks and update order.
- Compare each propagator exhaustively with a concrete-value oracle on small widths. The oracle checks soundness, failure, subsumption, and a declared bit-consistency expectation.

### 4. Add structural operations and ordering

- Implement extraction, concatenation, repeat, zero/sign extension, constant shifts and rotations, followed by variable shifts.
- Implement direct unsigned comparisons and derive or implement their signed and reified forms.
- Test edge cases at zero, width minus one, width, and larger shift amounts, plus mixed operand widths for the structural operations that intentionally change width.

### 5. Add modular arithmetic

- Implement addition and its carry representation first, then unary negation and subtraction.
- Add multiplication using conditional shifted terms or a stronger measured alternative.
- Introduce `WordSemantics` at the first operation that needs it. Implement SMT-LIB's total division, remainder, signed modulus, zero-divisor, and signed-overflow behavior as the default and test every edge case directly.
- Keep policy selection visible in direct posting and MiniModel expression records so a future semantics policy can be added without changing the domain representation or silently changing existing models.
- Keep intermediate variables available to ordinary Gecode search when that is useful; do not expose implementation-only variables through the public result API.

### 6. Add MiniModel word expressions

- Add `WordExpr` and the smallest supporting expression-node set needed to cover the direct logical, structural, comparison, conditional, and arithmetic APIs.
- Follow Gecode's established operator style where C++ operators have the intended fixed-width meaning. Use `snake_case` functions for multiword operations such as `sign_extend`, logical right shift, signed division, and signed modulus when operator syntax would hide an important distinction.
- Preserve widths and `WordSemantics` in expression nodes, reject invalid width combinations before lowering, and lower expressions through the same posting functions tested by the direct API.
- Add mixed word/Boolean expressions and reification without creating a parallel Boolean representation of every word.
- Provide model-level tests that compare direct posting and MiniModel solution sets and propagation outcomes.

### 7. Complete system integration

- Add the normal CMake and Autoconf/Make matrices: enabled and disabled word module, shared and static libraries, install/export consumption, generated sources, compiler visibility, and representative supported platforms.
- Add fault-injection coverage for variable/actor cloning and any heap-backed state, plus sanitizer runs for the module and its tests.
- Add Doxygen groups, MiniModel and direct-posting examples, changelog entries, and several representative models that are clearer with words than with manually bit-blasted Boolean arrays.
- Keep redistributable examples small, reviewable, licensed for inclusion, and built through the normal example machinery.
- Add a repeatable benchmark harness comparing native word propagation with an equivalent Gecode Boolean decomposition. Checked-in manifests describe expected corpus layout and benchmark metadata without copying private instances into the repository.
- Maintain as large a private development benchmark corpus as practical outside tracked repository content. The harness accepts an explicit external corpus root, reports missing private datasets cleanly, and makes accidental redistribution difficult through ignore rules and packaging checks.
- Include small verification and cryptographic kernels, applicable Wang-style instances, and broader QF_BV material in the private corpus when licensing and local storage permit. Public CI uses only redistributable fixtures.

## Performance follow-up

The initial Release benchmarks establish two independent optimization targets.
On 64-bit multiplication, the word model has 25.8 times fewer root
propagators than the Boolean decomposition but is only 1.54 times faster. A
sampling profile attributes 67.7 percent of native top-of-stack samples to
`Word::Logic::Table::propagate` and another 21.8 percent to
`WordView::narrow`. The existing addition and multiplication postings are
decompositions, so actor scheduling and communication consume much of the
representation advantage.

The follow-up therefore has two separate implementation tracks:

- implement compact native addition and multiplication propagators, declaring
  and measuring each actor's propagation contract rather than assuming that a
  smaller actor is stronger; and
- optimize the generic `Word::Logic::Table` actor independently, preserving its
  existing semantics and bit-consistency contract.

Addition is implemented before multiplication. The native addition actor must
achieve bit consistency for the fixed-width carry relation without model-level
carry variables. The native multiplication actor must be sound, exact on
assigned words, perform useful bidirectional cube narrowing, and document any
gap from bit consistency. It must remain one ordinary Gecode actor rather than
hiding the existing decomposition inside an actor.

The final development benchmark reconstructs four builds from the durable task commits:
the original decomposed baseline, native arithmetic only, table optimization
only, and both changes. Experimental switches do not become public API. The
production posting keeps a native actor only when focused propagation and
search benchmarks show a useful performance result without a material
regression in the declared propagation contract.

This benchmark is temporary development work, not Gecode product
infrastructure. Run it from temporary worktrees and scripts, retain raw results
outside the repository, and record only the conclusions needed to justify the
production implementation. Do not add a benchmark CLI, plotting stack,
manifests, fixtures, or benchmark-only example target to Gecode for this
follow-up.

The benchmark separates forward assigned evaluation from partial-domain and
inverse search cases. It reports wall time, peak RSS, actors, propagation calls,
nodes, failures, and solution parity. Results distinguish representation and
propagation effects from process startup and do not claim SMT-solver parity.

## Testing

Use focused coverage through the existing `test/word` framework and temporary
development benchmarks. Addition receives exhaustive assigned semantics and
small-width partial-domain checks sufficient to establish its declared bit
consistency. Multiplication receives exhaustive assigned semantics at small
width, representative partial/inverse cases, and differential solution checks;
it does not require an exhaustive campaign over every partial multiplication
cube. Table optimization reuses the existing logic and conditional tests and
adds only focused regression coverage for a concrete optimized path when the
current tests do not exercise it.

Follow established Gecode actor, cloning, recomputation, and subsumption
patterns. Do not add a new test executable or testing framework. Performance
validation uses Release builds, resumable per-run artifacts, medians, and
best-effort peak RSS. It must keep raw output and avoid timing instrumentation
that traverses every actor in the measured process. Broad platform matrices,
large industrial QF_BV corpora, SAT learning comparisons, and battle-hardening
edge campaigns remain outside this follow-up.

## Validation

Validation is layered so that a fast rule error does not hide behind end-to-end search:

1. **Representation invariants:** exhaustive mask-pair construction and update tests for small widths, plus boundary tests at width 64.
2. **Concrete semantics:** exhaustive evaluation of each operation on small assigned words against an independent unsigned-integer oracle.
3. **Propagation soundness:** enumerate partial word domains at small widths and verify that no supported concrete tuple is removed. Where bit consistency is claimed, verify that every remaining unknown bit has support for both values.
4. **Actor lifecycle:** shared variables, posting aliases, subsumption, disposal, cloning, recomputation, archived choices, no-good pruning, tracing, and failure during clone allocation.
5. **Differential models:** channel words to Boolean arrays and compare solution sets with an independently posted Boolean decomposition.
6. **MiniModel parity:** direct-posting and expression-based models have identical widths, semantics policies, solution sets, and documented propagation behavior.
7. **Build and packaging:** CMake and Autoconf/Make, module on/off, checked-in generated sources, shared/static, install and downstream consumption.
8. **Corpus boundary:** packaging and repository checks contain all public fixtures and no private benchmark payloads; manifests and benchmark code remain usable when the external corpus is absent.
9. **Performance guardrails:** propagation-call counts, clone footprint, allocations, runtime, and peak memory on representative models. Results are compared with the Boolean decomposition and reported without claiming SMT-solver parity.

Every operator implementation records whether it is direct, rewritten, or decomposed and what propagation property its tests enforce. This keeps later performance work from silently weakening a previously established contract.

## Boundaries

- This area is a full implementation spike, not a battle-hardening exercise. Each task must follow established Gecode patterns, reuse normal framework and test machinery, and avoid novel infrastructure, special-case test paths, exhaustive edge-case campaigns, or verification beyond what is proportionate to getting the implementation working.
- Prefer the ordinary Gecode implementation for the analogous variable, propagator, brancher, MiniModel, test, build, or documentation concern. Introduce a new pattern only when the Word domain genuinely requires it.
- This area owns the native word variable, its constraints, search support, tracing, tests, build integration, and user documentation.
- It does not add a SAT engine or explanation protocol to the Gecode kernel.
- It does not promise competitive results on large industrial QF_BV benchmarks.
- It does not add a general arbitrary-precision bit-vector library.
- It does not change existing integer or Boolean semantics to imitate fixed-width arithmetic.
- It does not require FlatZinc, MiniZinc, or the experimental model interface to grow a word-vector type in the same change.
- Private benchmark instances are development inputs, not project artifacts. They are not committed, packaged, copied into `.zd`, or treated as required inputs for ordinary builds and public tests.
