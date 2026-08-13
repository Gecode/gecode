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

Every new or replacement Word propagator must add or update normal
Gecode-style tests in the shared `test/word` infrastructure established by
word-002, with the test source registered in the ordinary monolithic
`gecode-test` inventories. Focused benchmarks, source inspection, and ad-hoc
development drivers supplement these tests; they never replace them.

Follow established Gecode actor, cloning, recomputation, and subsumption
patterns. Do not add a new test executable or testing framework. Performance
validation uses Release builds, resumable per-run artifacts, medians, and
best-effort peak RSS. It must keep raw output and avoid timing instrumentation
that traverses every actor in the measured process. Broad platform matrices,
large industrial QF_BV corpora, SAT learning comparisons, and battle-hardening
edge campaigns remain outside this follow-up.

## Accepted propagator follow-up

The post-spike gap analysis identified a small second follow-up. It adds useful
Gecode modeling constraints that are not part of the original Wang/Wombit
surface, and replaces several remaining decompositions only where focused
Release measurements justify the native actor.

The accepted modeling additions are:

- a Word-array `element` constraint with an `IntVar` index;
- Word-to-Boolean reductions for conjunction, disjunction, and parity;
- carry, borrow, and signed/unsigned arithmetic overflow predicates, including
  the corresponding current SMT-LIB overflow operations;
- population count and leading/trailing-zero counts with bounded `IntVar`
  results; the zero-input semantics for the zero-count operations must be
  explicit; and
- MiniModel forms only where they follow the established direct-API lowering
  pattern without creating a parallel representation.

The accepted native-actor investigations are subtraction/negation, unsigned
division/remainder, signed division/remainder/modulus, Boolean-controlled word
ITE, n-ary word logic, and n-ary modular addition. These are not new semantics:
the existing public APIs remain valid and the current decompositions are the
baseline. Each native actor records its propagation contract and is retained
only when a temporary Release comparison shows a useful actor, propagation,
runtime, or memory result without a material semantic or propagation
regression. Benchmark scripts and raw results remain outside the repository.

General Word-to-Int conversion remains outside the accepted work because a
Gecode `IntVar` cannot represent the entire unsigned or signed 64-bit Word
range. Generic bit permutation, word `distinct`, table globals, arbitrary
precision, and new frontend support remain evidence-gated rather than implied
by these tasks.

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

## Measured hot-path follow-up

The expanded Release test campaign ran all 124 registered `Word::` tests under
150 varied iterations per test, including sparse and aggressive fixing orders
and a four-thread run, followed by 500 iterations of every Word test family.
All runs passed. Temporary sustained workloads and sampling profiles then
identified four bounded behavior-preserving optimization candidates:

- n-ary AND, OR, and XOR repeatedly publish every input during their local
  fixpoint; XOR sampling spent about one fifth of top-of-stack samples in
  `WordView::narrow`;
- native multiplication repeatedly publishes intermediate prefix and inverse
  results during its local fixpoint; actor and `WordView::narrow` frames
  accounted for about half of the sampled time;
- native n-ary addition rebuilds its width-by-arity carry state and recreates a
  `Region` plus input buffers on every local fixpoint pass; and
- generic `Logic::Table` now spends most of its time in its small generic tuple
  enumeration after the earlier publish-once optimization removed most kernel
  narrowing overhead.

Optimize these paths independently. Preserve the existing public API,
propagation contract, aliases, lifecycle, and actor count. Prefer local mask
closure and changed-only publication over advisors or a finer event design:
the standard Word `ModEventDelta` exposes the event class, not the individual
newly fixed bit masks. Table specialization is retained only if measurement
justifies the extra code; a measured no-change outcome is valid.

Testing remains proportionate. Reuse the ordinary registered `test/word`
Logic, Conditional, Arithmetic, MiniModel, and TestFramework coverage. Add or
adjust a normal Gecode-style test only when an optimized path is not already
exercised; do not add a new harness or exhaustive matrix. Benchmark and profile
temporary Release drivers outside the repository, recording semantic parity,
actors, propagation calls, nodes, failures, solutions, wall time, and
best-effort peak RSS. Retain a production change only for a useful repeatable
gain without changing the declared propagation result.

## Mixed modular product follow-up

Add a mixed-domain modular product relation with two same-width Word operands,
a positive `IntVar` modulus, and a same-width Word result. Its mathematical
meaning is `result = (x * y) mod modulus`; implementation must compute assigned
products without host overflow. This is a deliberate mixed constraint, not a
general Word-to-Int conversion: the modulus remains within Gecode's ordinary
integer limits, while operands and result retain Word widths up to 64.

The direct API is
`product_mod(Home, WordVar, WordVar, IntVar, WordVar)` followed by a standard
`Reify` overload supporting `RM_EQV`, `RM_IMP`, and `RM_PMI`. Reject or prune
nonpositive modulus values through the normal posting/propagation contract.
Do not add constant overload matrices, alternate zero-modulus semantics,
MiniModel syntax, or unrelated integer arithmetic changes in the first slice.

Both propagators use the ordinary `test/word` infrastructure. The direct task
covers assigned semantics, overflow-safe products, representative partial
propagation, width and modulus contracts, aliases, failure, cloning,
recomputation, and subsumption. The reified task covers the complete truth rows
for all three modes, control rewrites, aliases, cloning/recomputation, and
subsumption. Keep this proportionate: use small assigned domains and focused
partial cases, not an exhaustive 64-bit campaign or a new harness.

## System workload performance follow-up

Temporary Release examples exposed the next system-level opportunities after
the propagator-local optimizations. An 8-bit ARX preimage model took about 235
ms per root and 158,975 nodes when branching on input bits from the least
significant bit, but about 0.81 ms and 511 nodes when branching on output/state
information first. Reversing the input order was worse still at about 539 ms
and 370,815 nodes. This is a modeling and search-order result, not evidence for
another ARX-specific propagator.

Three other temporary models exercised distinct public surfaces: a lookup and
register-file model using `element` and n-ary arithmetic (about 3.12 ms, 2,595
nodes, and 13,915 propagations), a bitboard model using shifts, logic, and
population count (about 0.126 ms, 463 nodes, and 1,766 propagations), and a
combined quotient/remainder model (about 1.96 ms, 7,141 nodes, and 14,638
propagations). A variable-shift decoder was small at about 0.0048 ms, 17 nodes,
and 35 propagations. Using full copying instead of deeper recomputation reduced
runtime by roughly 13% for ARX, 45% for lookup, 48% for quotient/remainder, and
25% for the bitboard model with identical solutions and nodes; the tiny shift
model was neutral. Examples should therefore demonstrate information-flow
aware branching and mention search-option sensitivity without baking one
global policy into WordVar.

Sampling located the remaining implementation work. Binary Add accounted for
roughly 46--56% of ARX top-of-stack samples, with Word narrowing another
13--19%. The lookup model spent about 19% in NaryAdd, 18.5% in Element
propagation, 8.2% in Element copying, and 8.6% in integer-domain operations.
The quotient/remainder model spent about 27% in Word narrowing plus about 9%
in Div and 4% in Mod, while the variable-shift model spent about 14% in Word
narrowing and 10.5% in its actor. Popcount and the already-specialized generic
logic table were not primary system bottlenecks in these workloads.

Turn these findings into five narrow slices: normal ARX/lookup/bitboard examples
and search guidance; binary Add local-loop optimization; variable-shift local
closure/publication optimization; Element candidate/copy investigation; and a
combined unsigned divmod relation that can share quotient/remainder reasoning.
Examples are ordinary Gecode Script examples, not benchmark fixtures. All
benchmark drivers, profiles, and raw results remain temporary and outside the
repository. Each retained propagator change preserves its public semantics and
declared propagation contract, uses normal `test/word` coverage, and is kept
only when a focused exact-baseline Release comparison shows a useful tradeoff.

## Realistic profiling corpus research

The next profiling pass must move beyond microbenchmarks while remaining small
enough to understand. Research candidate WordVar models grounded in published
bit-vector applications and standard QF_BV benchmark families, then calibrate
public, reproducible instances to roughly 10--60 seconds on the current Release
solver. Prefer workloads whose runtime comes from sustained propagation and
search rather than construction, output, or a deliberately poor variable
order.

The initial source set is the Wang/Søndergaard/Stuckey word-level propagation
and Wombit papers, which evaluate standard SMT-LIB QF_BV families including
bounded-model-checking, Brummayer/Biere arithmetic, SAGE symbolic execution,
and SPEAR software-analysis instances; CP work on automatically generated
differential-cryptanalysis models for AES, CRAFT, Midori, and Skinny; and
reduced-step MD5/SHA-1 inversion work. These sources motivate representative
models but do not require importing private or third-party benchmark payloads.

For each retained case, identify the public provenance, model shape, adjustable
difficulty parameter, intended search order, and dominant Word operation
families. Where reasonable, build an independent Boolean-array or ordinary
integer decomposition through existing Gecode APIs and compare exact solution
or checksum semantics. Do not force a decomposition where Gecode IntVar cannot
represent the Word range or where the alternative would test a materially
different relation. Record actor counts, propagation calls, nodes, failures,
solutions/checksum, wall time, best-effort memory, and a bounded sampling
profile. Temporary drivers and raw results stay outside the repository; the
durable output is the ranked research conclusion and recommended later
profiling corpus, not a benchmark framework or production implementation.

### Recommended corpus and calibration

The research retained three deterministic one-root instances, but they cover
two application families rather than three. This limitation is important when
interpreting the profiles.

1. **CRC-16/CCITT-FALSE preimage** is the best control and decomposition case.
   A 28-bit message starts from `0x1d0f`, uses polynomial `0x1021`, and
   constrains the low four output bits to zero. The Word model enumerates
   16,777,216 messages in 44.51 seconds, with 164 root actors, 1,102,413,424
   propagation calls, 33,554,431 nodes, no failures, and checksum
   2,251,799,805,296,640. An independent Boolean recurrence has the same
   solutions, checksum, nodes, and failures; it takes 42.60 seconds with 455
   actors and 1,021,866,623 propagation calls. The Boolean model is therefore
   about 4% faster despite 2.8 times as many actors.
2. **Constructed symbolic register/ALU path** is the best arithmetic-heavy
   profile. One 22-bit input passes through 12 rotate/add, rotate/XOR, and
   rotate/AND-plus-constant transitions before two output bits are constrained.
   It enumerates 2,367,275 solutions in 59.14 seconds, with 40 root actors,
   546,624,487 propagation calls, 6,081,663 nodes, 673,557 failures, and
   checksum 4,897,215,055,457. The model is motivated by the SAGE/SPEAR and
   symbolic-execution workload families in the Wang and Wombit evaluations,
   but it is a public deterministic construction rather than a copied paper
   instance. A Boolean ripple-add decomposition remains useful follow-up work.
3. **Reflected CRC-16/X-25-style preimage** gives a distinct right-shift path.
   It uses initial state `0xffff`, reflected polynomial `0x8408`, and the same
   28-bit message/output restriction. The Word model takes 47.10 seconds with
   164 actors, 1,144,169,916 propagation calls, 33,554,431 nodes, no failures,
   16,777,216 solutions, and checksum 2,251,799,805,296,640. Its independent
   Boolean recurrence takes 44.38 seconds with 455 actors and 1,050,569,834
   calls; solutions, checksum, nodes, and failures match exactly. Keep this
   instance as a reflected-shift comparison, not as evidence of a third
   application family.

Five-second macOS samples make the algorithmic hot paths concrete. In the
CCITT Word model, `WordView::narrow` accounts for 910 top-of-stack samples,
`Logic::Table::propagate` for 464, followed by search status/cloning, bit
channeling, and fixed shifts. In the ALU model, `Arithmetic::Add::propagate`
dominates with 2,573 samples, followed by `WordView::narrow` at 461 and
`Logic::Table::propagate` at 205. These profiles point first to narrowing and
the interaction among shifts, truth-table logic, channels, and conditional
feedback in CRC; and to the native Add transition/support loop in the ALU
model. They do not by themselves justify an implementation change.
The reflected case has the same profile shape: `WordView::narrow` leads with
944 top-of-stack samples and `Logic::Table::propagate` follows with 469; its
successful sample is `/private/tmp/word049-crcr-word.sample.txt` with SHA-256
`aa470ad4506f94c270a6ebc88b53af48113ff2f06330a353426d6f7f552d3de2`.

The reduced-hash and differential-cryptanalysis candidates were not retained.
The Zaikin paper supplies the exact MD5/SHA-1 step structures and a strong
future target, but temporary reduced ARX models exceeded the 60-second bound
even after reducing rounds or constraining more digest bits. The 18-round
variants exceeded 130 seconds and an 8-round variant hit the 60-second hard
limit. This exposes a calibration and propagation gap rather than a useful
current benchmark. Tagada's AES/CRAFT/Midori/Skinny truncated-differential
models use byte/nibble activity variables and extensional S-box/MixColumns
relations; Word cube domains represent bit values, so translating that corpus
would profile a different model. It was rejected before implementation. ALU
variants with 24 and 48 transitions also exceeded the target window; the
12-transition instance is the calibrated scale.

The temporary driver is `/private/tmp/word049-profile.cpp` with SHA-256
`57ebd2861752cfc8ba6afc01763f317d05277638b0d5b23c36d25155c98d3ac7`;
the profiled Release binary has SHA-256
`fa8d1750177f63d44eb81096359a81e9cbbdfa7aee72d9c37e13d272dfd0b0d1`
and was built from commit `f04ac62b9bb2c72af83e2b635057ff6648413832`.
Use a 60-second hard timeout for future calibration. Peak RSS is unavailable
on this host because `/usr/bin/time -l` cannot read `kern.clockrate`; do not
substitute process-wide historical RSS. The next research question is how to
calibrate a faithful reduced MD5 or SHA-1 instance without using a deliberately
bad branch order, followed by an exact Boolean ripple decomposition of the ALU
case.

## Reduced-hash calibration follow-up

Calibrate a faithful reduced-step MD5 preimage by scaling the inverse problem,
not by leaving most of the digest unconstrained. Follow Zaikin's intermediate
problem idea at the modeling level: retain 32-bit MD5 state words and authentic
round functions, constants, message schedule, modular additions, and rotations;
fix the initial state, most message words, and a complete computed target state;
then expose a bounded part of one message word or weaken a controlled boundary
between adjacent steps. Start with MD5 because its four-word state and schedule
are smaller than SHA-1's five-word state and expanded message schedule. Attempt
SHA-1 only after an MD5 calibration succeeds and through the same bounded
method.

The retained instance should be one deterministic DFS root and should complete
in roughly 10--60 seconds because of its authentic inverse search, not because
the same root is repeated, output is printed, or branching is deliberately poor.
Use normal information-flow-aware branching and a hard timeout for every
calibration. Record the exact construction, adjustable difficulty parameter,
solutions/checksum, actors, propagation calls, nodes, failures, wall time,
best-effort memory, and a bounded sample. Where it stays within the budget,
build an independent Boolean ripple/bitwise decomposition and require exact
semantic parity. A measured failure to calibrate SHA-1 is acceptable after the
bounded ladder; do not turn this research into SAT learning, a new propagator,
or permanent benchmark infrastructure.

Testing is limited to temporary Release drivers and exact concrete hash oracles.
There are no production changes and therefore no new `test/word` tests. Reuse
ordinary Word and Boolean APIs, standard DFS ownership/cloning patterns, and
temporary artifacts outside the repository. The durable result is only the
calibration conclusion in this brief.

### Calibration result

The reduced-hash calibration retains two faithful one-block inverse workloads.
Both use the standard padded `abc` block, standard initialization constants,
authentic 32-bit step equations, and a complete feed-forward target produced by
independent concrete code. Only a prefix of the actual 512-bit message block is
left unknown; branching follows message schedule order with the ordinary MSB
word decision. Each measurement is one DFS root and enumerates distinct
preimages.

The primary workload is **MD5-16 with 148 unknown message bits** (`M[0]` through
`M[3]` plus 20 low bits of `M[4]`). It enumerates 1,048,576 preimages with the
order-independent checksum 4,454,945,375,245,327,872. The Release run took
14.33 seconds wall and 14.31 seconds user, with 170 root actors, 34,293,028
propagation calls, 2,180,289 nodes, and 41,569 failures. The bounded ladder at
132, 136, 140, 144, and 148 unknown bits produced 16, 256, 4,096, 65,536, and
1,048,576 solutions in approximately 0.00, 0.01, 0.07, 0.97, and 14.59 seconds,
respectively. This is the recommended first cryptographic profiling case.

An independent MD5 Boolean model uses per-bit round logic, wired rotations, and
ripple modular addition without Word constraints. At the adjacent 132-bit
boundary it exactly matches the Word model's 16 solutions and checksum
4,820,228,306,632,347,304. The Word model uses 170 actors, 1,496 propagation
calls, 59 nodes, and 14 failures; the Boolean model uses 11,119 actors,
2,909,918 propagation calls, 10,333 nodes, and 5,151 failures and takes 0.60
seconds. Its search expansion makes a 148-bit decomposition run disproportionate
under the 65-second cap, so parity is established at the neighboring boundary
rather than claimed at the retained scale.

The secondary workload is **SHA-1-16 with 180 unknown message bits**. The model
uses the standard five-word state, Ch round function, round constant, rotations,
five-operand modular step sum, and complete message-expansion implementation;
the retained prefix consumes `W[0]` through `W[15]`. It enumerates 1,048,576
preimages with checksum 1,829,032,905,292,644,352 in 18.51 seconds wall and
18.47 seconds user, with 168 root actors, 46,750,389 propagation calls,
2,097,411 nodes, and 130 failures. The ladder at 164, 168, 172, 176, and 180
unknown bits produced 16, 256, 4,096, 65,536, and 1,048,576 solutions in
approximately 0.00, 0.00, 0.07, 1.16, and 18.51 seconds. A SHA-1 Boolean model
was not attempted after the MD5 decomposition demonstrated the likely search
expansion; this is an explicit budget limitation.

Three-second macOS samples contain 2,256 samples each. MD5 spends 1,390
top-of-stack samples (61.6 percent) in `NaryAdd`, 479 (21.2 percent) in binary
`Add`, and 75 (3.3 percent) in `WordView::narrow`. SHA-1 spends 1,753 (77.7
percent) in `NaryAdd`, 109 (4.8 percent) in binary `Add`, and 91 (4.0 percent)
in `WordView::narrow`; cloning and model copying account for roughly four
percent in each sample. N-ary modular addition is therefore the concrete hot
path in these models, but this research does not infer a defect or authorize a
fix. Sample peak physical footprints are 1,424 KiB for MD5 and 1,440 KiB for
SHA-1; independent `ps` snapshots report 2,528 KiB RSS for each. The normal
`/usr/bin/time -l` RSS path remains unavailable because macOS denies the
`kern.clockrate` query.

The construction is derived from Zaikin's MD5/SHA-1 step definitions and his
adjacent-step message-addend weakening method, but these 16-step CP workloads
are not the paper's 28--29-step MD5 or 23--24-step SHA-1 SAT instances and do
not support solver-competitiveness claims. Temporary source hashes are
`4e6cbbe8bf3db61f9370b77c93f9bb57efc26b14aeed98636ca8b29e3ca01e34`
(MD5), `5638dfdc8c2032cdc79664bce06c5c56fc8c4fb283facb4ddeba882c6d835508`
(Boolean MD5), and
`80319346087c892b68340daa90796680f02c81c914aa70ad6a813343420f12a0`
(SHA-1). The sample hashes are
`88c3a12556ffbdb2ae28c68261baf026cc9038a91e3fa32655bf923bfadf7472`
and `5dfe9251b0163184e0e5732072c487aa981df8fdf1432d961ba8b88162a7cf24`.
All temporary sources, binaries, raw output, and samples remain outside the
repository.

## Reduced-hash examples and profiling follow-up

Promote the calibrated MD5-16 and SHA-1-16 models into ordinary configurable
Gecode `Script` examples. They are natural demonstrations of 32-bit WordVar
round logic, rotations, n-ary modular addition, fixed targets, cloning,
recomputation, and information-flow-aware search. The checked-in examples are
models, not a benchmark harness: use standard driver options, modest defaults,
and normal CMake/Make example inventories. The tens-of-seconds configurations
remain explicit profiling invocations rather than slow default runs.

Use the examples as the end-to-end feedback loop for detailed Release
profiling. Separate model/search effects from implementation costs by recording
wall time, actors, propagation calls, nodes, failures, solutions/checksum,
clone/recomputation options, and best-effort memory. Use bounded sampling plus
temporary targeted instrumentation to count n-ary-add invocations, local
fixpoint passes, support checks, changed publications, and clone/copy work.
Compare the current n-ary relation with an equivalent established Word
decomposition where that isolates algorithm cost; do not change semantics or
mistake a different search tree for a faster propagator.

Rank three to five falsifiable performance hypotheses before optimization.
Turn only confirmed mechanisms into narrow tasks. A retained change must keep
the implementation native and Gecode-style: ordinary views and actors, honest
propagation conditions and costs, normal cloning/disposal, no solver-specific
switches, no benchmark-driven special cases, and no hidden bit blasting. Both
algorithmic and code-level improvements are in scope, including stronger or
cheaper propagation, reduced redundant work/publication, and smaller clone
state, but not SAT learning, a new profiler, or a hash-specific propagator.

Testing is proportionate. The examples are built and run through the existing
example machinery and checked against their concrete hash oracle at a small
configuration. Each retained propagator change adds or updates normal
Gecode-style `test/word` coverage only when its behavior or lifecycle changes;
otherwise reuse the existing arithmetic, clone/recomputation, MiniModel, and
TestFramework suites. Temporary profiling supplements these checks and never
becomes a new test executable or durable benchmark subsystem.

The natural example corpus also includes the three calibrated models from the
preceding realistic-workload research: CRC-16/CCITT-FALSE, the constructed
symbolic register/ALU path, and reflected CRC-16/X-25-style recurrence. Promote
them as ordinary configurable `Script` examples with quick defaults and
explicit profiling scales. The two CRC examples should share conventional
model structure where that improves readability, while remaining separate
executables with their distinct left/right-shift recurrences. The ALU example
must clearly identify itself as a constructed symbolic-execution-style path,
not a copied paper instance.

Profile all five examples before selecting production optimizations. This
cross-workload check prevents a hash-only improvement from being mistaken for
a general Word implementation improvement. Preserve exact native/decomposition
parity for the CRC cases and compare the ALU path with a Boolean ripple
decomposition only if the bounded run remains practical. Rank optimization
tasks by repeatable end-to-end benefit across the relevant examples, while
allowing an operation-specific improvement when its public propagator has a
clear independent modeling use.

### Hash-example profiling findings

The checked-in MD5 and SHA-1 examples reproduce the calibrated workloads as
ordinary `Script` runs. MD5-16/148 enumerates 1,048,576 solutions with
34,293,028 propagations, 2,180,289 nodes, and 41,569 failures in 13.944
seconds. SHA1-16/180 enumerates 1,048,576 solutions with 46,750,389
propagations, 2,097,411 nodes, and 130 failures in 17.929 seconds. Their quick
defaults use 132 and 164 unknown bits and finish in less than three
milliseconds. These runs establish the end-to-end feedback loop; word-052 must
cross-check any proposed optimization against CRC and the symbolic ALU before
an implementation task is accepted.

The hash-only investigation tested five ranked, falsifiable hypotheses:

1. **Repeated local n-ary closure is expensive.** If this is the main cause,
   counted local passes should substantially exceed actor calls and a
   deliberately one-pass experimental build should reduce work without
   changing this workload's solutions or search. Instrumented MD5 recorded
   7,977,666 `NaryAdd` calls but 15,188,528 local passes, covering 486,032,896
   bit-passes and 949,501,184 input-support checks, with 7,217,686 changed
   publications. A temporary one-pass run reported identical solutions,
   checksum, actors, propagations, nodes, failures, and publications while
   reducing user time from 14.36 to 10.03 seconds. This is **provisional
   evidence only**: the exact one-pass patch and command were not preserved,
   and matching one model does not establish the actor's general fixpoint
   contract. Word-052 may use the result to motivate a fresh reproducible
   experiment, but no production task may simply remove the loop.
2. **The bounded-carry support algorithm dominates each call.** If so, sampling
   should remain in `NaryAdd` after publication reductions, and measured work
   should scale with width times arity and local passes. A five-second MD5
   sample placed 1,155 of 1,852 main-thread samples in `NaryAdd`, versus 384 in
   binary `Add`; the targeted counters above confirm nearly one billion
   input-support checks. This hypothesis is **confirmed for the hash models**.
   The next investigation must distinguish necessary support projection from
   avoidable recomputation while preserving soundness, aliases, and the
   declared propagation contract.
3. **The separate constant-add model shape is costly.** If folding the round
   constant into the public n-ary addition is useful, it should reduce actors
   and end-to-end work with exact solution/checksum parity. Folding removes 16
   root actors. MD5 changes from 14.96 to 13.07 seconds and 34,521,825 to
   31,231,517 propagations; its nodes change from 2,180,289 to 2,114,231 and
   failures from 41,569 to 8,540. SHA-1 changes from 18.68 to 17.04 seconds and
   46,204,439 to 45,372,000 propagations; nodes change from 2,097,411 to
   2,097,169 and failures from 130 to 9. Solutions and checksums remain exact.
   This is a **confirmed model/search tradeoff**, not evidence that the
   propagator itself became faster: exposing the constant to the global changes
   propagation and the search tree. Update the examples only after word-052
   confirms that the clearer one-global model remains preferable across the
   relevant corpus; no new API is needed because n-ary posting already folds
   assigned operands.
4. **Clone/recomputation policy contributes materially.** If true, changing
   clone distance with the same model should preserve the tree and checksum but
   change propagation work and time. On MD5, `c_d=1` uses 33,003,682
   propagations and 13.95 seconds; `c_d=32` uses 35,854,340 and 15.60 seconds,
   with identical solutions, checksum, nodes, and failures. This is
   **confirmed but moderate** and belongs in example/search guidance, not in a
   Word actor optimization or global policy.
5. **Dynamic arity loops are a primary code-level cost.** If true, a small
   compiler-visible four-way unroll should improve the same instrumented work
   materially. It left every counter unchanged and measured 14.18 seconds
   versus the 14.36-second instrumented baseline, within the noise of these
   single runs. This hypothesis is **rejected**; do not add arity-specific
   NaryAdd code on this evidence.

The provisional dependency-ordered follow-up boundary is exact. First,
word-052 adds CRC-16/CCITT, reflected CRC-16, and symbolic ALU examples and
reruns the five-model comparison. Only after that cross-check may it create:

- one **NaryAdd fixpoint-work investigation** that reconstructs an auditable
  candidate, proves when an additional local pass can or cannot change masks,
  preserves ordinary aliases and lifecycle, adds normal `test/word` coverage
  only for a changed contract, and retains a native change only for exact
  cross-workload parity plus repeatable gain; and
- one **example addition-shape cleanup** that folds assigned constants into
  existing n-ary addition calls where this is clearer and measured useful,
  without adding an overload, propagator, rewrite policy, or benchmark switch.

Do not create a loop-unrolling task. Treat clone-distance sensitivity as
documented model guidance unless the five-model study exposes a distinct
general search-engine issue. These boundaries remain provisional until
word-052 records the cross-corpus evidence.

Profiling used exact source commit
`beaefa0e5a0a51a33cd30f0e4f47bd39b05617f4`, Release libraries, the two
checked-in example sources, and temporary variants under
`/private/tmp/gecode-word051-instrument`. The MD5 example source hash is
`fb88e49d3ccf56cd15ec49f143dcae9413be9f8219bd63394345f2405bba2ed3` and
the SHA-1 hash is
`982e4857a1721b61acdfdc5f594236ea8bc1754f33c51992d5bdc342d61a5412`.
Key raw-result hashes are
`40c4c1df6915320c1ec5c34ba08d9b82bca72fe08b4d227779e5a3130f585625`
(MD5 baseline),
`afc95feee17b910b029fb5671df5e0c6850c9e91b6e6bc2bd7f4655b4df171c4`
(folded MD5),
`9f4bc9ea4345c46f8381c18ed74a8fb0918cbbc3a87e68b3f959615dda3bb00b`
(instrumented MD5),
`b6e799dc9f7838ea359a9ca4eb2b76b20d4639b0dcf7d0145f896f3dc3097eb0`
(one-pass output),
`b106da57b88275305892de340919c443133601925b4fec434e37ca8207637e86`
(unrolled output),
`67b9871f409d1940bdfa5e46297e45bcdb6c520bf7137a6ed93ca8261d24cbe7`
(SHA-1 baseline), and
`895e1ddddb64d48f7de576ca2352400a279b54023dcf9648360c810a2fbd1d88`
(folded SHA-1). The sample `/private/tmp/word051-md5.sample.txt` hashes to
`1c78ed86797c900d15b55844d62eed239db0dc7f0cbb7ebfb1e29dd894c9e257`.
Best-effort live RSS was 2,352 KiB; `/usr/bin/time -l` again could not report
RSS because macOS denied `kern.clockrate`. No temporary instrumentation or raw
result is tracked.

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
