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

### Five-example cross-corpus result

The realistic example corpus now includes CRC-16/CCITT-FALSE, reflected
CRC-16/X-25-style recurrence, and a constructed symbolic register/ALU path in
addition to reduced MD5 and SHA-1. Their defaults finish in less than one
millisecond; the explicit profiling configurations reproduce the word-049
one-root scales. CCITT with 28 message bits takes 42.271 seconds for 16,777,216
solutions, 1,102,413,424 propagations, 33,554,431 nodes, and no failures. The
reflected case takes 44.461 seconds with the same solutions/tree and
1,144,169,916 propagations. The 12-step, width-22 ALU path takes 59.047 seconds
for 2,367,275 solutions, 546,624,487 propagations, 6,081,663 nodes, and 673,557
failures.

Independent Boolean CRC recurrences have exact parity at the bounded 26-bit
scale. Both Word and Boolean variants enumerate 4,194,304 solutions with
checksum 140,737,486,258,176, 8,388,607 nodes, and no failures. CCITT Word uses
152 root actors and 404,887,158 propagations in 16.07 seconds; Boolean uses 421
actors and 404,242,923 propagations in 15.82 seconds. Reflected Word uses 152
actors and 420,878,621 propagations in 16.62 seconds; Boolean uses 421 actors
and 421,083,475 propagations in 16.48 seconds. A complete ALU Boolean ripple
model was omitted because the retained Word case already reaches the task's
59-second bound; no parity or relative-performance claim is made for it.

The five-model comparison sharpens the earlier hypotheses:

- Repeated `NaryAdd` closure and bounded-carry support projection are confirmed
  operation-specific hash costs. CRC and the ALU contain no `NaryAdd`. A
  reconstructed experimental one-pass build reproduces exact MD5 and SHA-1
  solutions, checksums, actors, propagations, nodes, and failures, with MD5
  14.36 to 10.03 seconds and SHA-1 18.68 to 11.83 seconds. This still does not
  prove the general fixpoint contract; the implementation task must derive and
  test the condition under which a second pass is redundant rather than simply
  deleting the loop.
- Folding assigned round constants into the existing n-ary addition is useful
  only in the hash model shapes. CRC has no addition, and the ALU has no
  adjacent multi-input sum into which its constants can naturally fold. This
  remains a small example cleanup after the actor investigation, not a general
  API or propagator task.
- Clone-distance sensitivity is general and larger for the CRC models. At 26
  bits, CCITT changes from 16.07 seconds and 404,887,158 propagations at
  `c_d=8` to 8.25 seconds and 193,462,323 at `c_d=1`; reflected changes from
  16.62/420,878,621 to 8.72/197,984,295. ALU changes from roughly 59 seconds
  and 533,099,261 propagations to 52.18 seconds and 486,316,626. Solutions,
  checksums, nodes, and failures remain unchanged. This is example/search
  guidance; no global clone policy follows from it.
- Dynamic n-ary loop unrolling remains rejected because it changes no work and
  gave only noise-scale timing movement.
- Hot paths are workload-specific beyond search restoration. MD5/SHA-1 are
  dominated by `NaryAdd`; the ALU sample has 2,534 binary `Add` frames, 490
  `WordView::narrow` frames, and 214 `Logic::Table` frames. CRC profiles remain
  led by Word narrowing followed by `Logic::Table`, channels, and fixed shifts.
  These CRC/ALU samples identify where time is spent but do not yet identify a
  new removable inefficiency, so they do not justify new Add, Table, channel,
  or shift tasks.

Sampled peak footprints are 1,184 KiB for ALU, 1,424/1,440 KiB for MD5/SHA-1,
and 1,472/1,488 KiB for the two CRCs. This process-scale evidence shows no
clone-state memory crisis and does not replace a supported peak-RSS facility.

The resulting task split is deliberately small and dependency ordered:

1. Investigate and, only if proved sound and measured useful, optimize
   `NaryAdd` local closure. The task must preserve the public API, native actor,
   aliases, bounded-carry propagation contract, cost/events, cloning, and
   subsumption. It must add or update normal arithmetic tests for any changed
   fixpoint logic and compare exact baseline/candidate results on MD5, SHA-1,
   existing partial/inverse arithmetic tests, and a small general n-ary search.
2. After that task, simplify the two hash examples by folding their assigned
   round constants into the existing n-ary add posting. This changes model
   propagation/search, so verify concrete targets/default output and report
   retained-scale solution/checksum and search counters; add no API or tests.

Do not create tasks for unrolling, clone policy, binary Add, CRC Table/channel,
or fixed shifts from these profiles. A future task requires a concrete measured
candidate rather than another sample naming a hot function.

The cross-corpus profiling baseline is commit
`8d38b69330` with Release libraries. Example source hashes are
`f456e64938af5afa5e457bf9e692fd0d1a21b62808cb0a253631c6733a6ae775`
(CCITT), `ca981becacb3ab877c88603ca8eb0e83bab4bc466e11d6308965ac5ec02b8b37`
(reflected CRC), and
`784828275334cbb13712d3cea14d2f9ef146ab65b3d34327a5805cc8022290a0`
(ALU). Raw CRC/ALU/copy-distance results and the ALU sample remain under
`/private/tmp/word052-*`; the ALU sample hashes to
`f086f4d400eaf9bbf7914aaaec476b36c76924eddd65d6012fb0d9a4984b5c38`.
No raw result, decomposition driver, or profiling probe is tracked.

### Native-citizen performance audit

Run a second profiling round over all five checked-in realistic examples after
the retained arithmetic and example-shape changes.  The purpose is not another
list of hot symbols: it is to distinguish unavoidable propagation work from
rescanning, repeated mask reads, no-op publication, premature computation,
misclassified scheduling cost, and clone/recomputation overhead.  Every
candidate must follow an established Gecode implementation pattern from the
Int, Bool, Set, or Float modules; the Word area must not introduce a private
scheduler, cache protocol, event system, or benchmark-only switch.

The audit starts from these ranked, falsifiable questions:

1. **CRC logic scheduling and publication.**  Determine how often Table,
   fixed-shift, channel, and conditional actors run for each event, how often
   they narrow a view or subsume, and whether they rescan data unchanged by the
   triggering delta.  Compare with established staged `ModEventDelta` and
   advisor implementations.  A delta-aware or advisor candidate is warranted
   only if the measured avoided full scans outweigh its subscription, clone,
   disposal, and state cost.
2. **Binary Add work in the symbolic ALU.**  Count calls, per-call bit and
   transition scans, useful publications, assigned/constant seams, and repeated
   executions after self-publication.  Test whether support can be computed in
   one fused pass or staged using ordinary Gecode patterns without weakening
   the exact carry-chain contract.  Do not revive NaryAdd work already closed
   by word-053.
3. **Actor scheduling cost and batching.**  Check whether current
   `PropCost::{LO,HI}` declarations reflect measured asymptotic work and whether
   changing an existing cost class alters queue order usefully across the five
   models.  Retain such a change only with exact parity and cross-workload
   benefit; cost tuning for one example alone is insufficient.
4. **Word variable notification cost.**  Measure `WordVarImp::narrow` calls,
   no-op calls, changed-zero/changed-one masks, and notification fan-out.  The
   existing atomic lo/hi update and `WordDelta` are the baseline Gecode-style
   design.  Any shared-kernel or variable-implementation change requires clear
   evidence that callers cannot eliminate the work locally and must preserve
   normal variable-copy and event semantics.
5. **Search restoration and clone footprint.**  Separate actor propagation
   time from cloning/recomputation using the examples' ordinary Script search
   options and a bounded `c_d`/`a_d` comparison.  Record actor mix and copied
   state.  This is model guidance unless a concrete Word actor copies avoidable
   state; do not change global search defaults.

Instrumentation stays in a detached temporary tree and uses counters or
sampling only.  It must not affect production actor layout in the measured
baseline, and it must be removed before retaining a candidate.  Benchmark the
current exact commit with Release builds, stable solution/checksum parity, and
actors, propagations, nodes, failures, wall/user time, best-effort memory, and
bounded samples.  Use bounded adjacent configurations for rapid hypotheses and
confirm only useful candidates at retained scales; do not multiply long runs
without a decision they can change.

Testing remains proportional and ordinary.  An investigation adds no tests.
Any later retained propagation-contract change updates the normal registered
`test/word` suite at the existing semantic, partial-domain, alias, clone,
recomputation, and subsumption seams; a pure implementation refactor needs no
new test when those paths already cover it.  Focused example builds/runs and
the relevant Word test filters are required, followed by `zd check word` and
`git diff --check`.  No durable benchmark runner, raw results, profiling hooks,
or generated corpus enters the repository.

### Native-citizen audit result

The exact Release baseline is commit `440d88eed1`.  The retained configurations
now measure MD5-16/148 at 8.127 seconds, 29,548,308 propagations, 2,114,231
nodes, and 8,540 failures; SHA-1-16/180 at 10.340 seconds, 46,656,770
propagations, 2,097,169 nodes, and 9 failures; CCITT-28 at 42.764 seconds and
1,102,413,424 propagations; reflected X-25-28 at 45.084 seconds and
1,144,169,916 propagations; and ALU-12/22 at 59.215 seconds, 546,624,487
propagations, 6,081,663 nodes, and 673,557 failures.  The hashes enumerate
1,048,576 solutions each, both CRCs enumerate 16,777,216 with identical full
binary trees, and the ALU enumerates 2,367,275.  Sampled peak RSS is 2,816 to
3,104 KiB; this is live polling rather than an operating-system high-water
mark.

Temporary counters at adjacent bounded scales distinguish scheduling from
useful propagation.  In CCITT-22, Table runs 4,321,399 times and narrows on
4,311,488 calls; fixed shift runs 3,646,154 times and changes a view on
3,646,133.  X-25 has the same pattern.  Their scans are therefore not mostly
stale work.  Table nevertheless performs 69,063,320 tuple visits and 8,632,915
local passes for CCITT, and 71,708,480 visits and 8,963,560 passes for X-25.
The existing post-time closure and changed-only publication are effective;
generic advisor staging is not justified for Table or Fixed.

Named XOR is a narrower algorithmic opportunity.  A temporary distinct-view
XOR recognizer replaced each uniform three-view truth-table scan with one
bit-parallel affine support calculation, retaining generic Table for aliases
and nonuniform tables.  It preserved solutions, propagations, nodes, and
failures exactly.  Repeated bounded runs improved CCITT and X-25 by roughly
3--8 percent and ALU by 6--13 percent while reducing the CRC tuple-scan count
from about 69--72 million to 4.3--4.5 million.  This evidence supports direct
standard actors for the named public AND, OR, and XOR operations, benchmarked
and retained independently; it does not support another Table-wide arity
specialization or a public truth-table API.

Bit channeling exposes genuine irrelevant-event scheduling.  Each bounded CRC
runs about 6.3 million `Channel::Bit` propagations, but only 2,242,613 decide
and subsume; roughly four million executions are caused by changes to other
bits of the same Word view.  Exact changed-bit masks are available only to an
Advisor, not through ordinary `ModEventDelta`.  A follow-up may compare the
current actor with the established `Council`/`ViewAdvisor` pattern used by
`Int::Linear::LinBoolInt` and `Extensional::Compact`, but must include advisor
fan-out, copy, disposal, and space footprint.  A Word-specific event or
scheduler is not warranted.

Binary Add remains the ALU's dominant sampled frame: at width 18 it runs
3,199,581 times, changes a view on 2,378,315 calls, and performs 172,777,644
recorded bit-phase scans.  This confirms necessary support work, not a new
removable mechanism: word-045 already introduced the local transition lookup
and changed-only publication.  Raising Add from `LO` to `HI` cost was slower on
ALU and slightly increased MD5 propagation; raising Table to `HI` was neutral
or worse and changed X-25 scheduling work.  Both cost changes are rejected.

Across the bounded corpus, 30--36 percent of `WordVarImp::narrow` calls are
no-ops.  Caller-local changed-only publication for fixed shifts removed
3.65 million no-op calls in CCITT and 3.70 million in X-25 with exact search
parity, but alternating timings were noisy and not consistently better.  Do
not change `WordVarImp`, whose atomic update, exact delta, and notification
semantics are already conventional; do not create a Fixed task without a
repeatable retained-scale benefit.

Search restoration remains model guidance.  At bounded scales, changing from
`c_d=1` to `c_d=32` preserves every solution, node, and failure count but moves
MD5 from 1,827,472 to 2,171,200 propagations, SHA-1 from 2,609,577 to
2,913,479, ALU from 16,853,322 to 17,262,312, and each CRC from about 12
million to about 88--89 million.  Samples still place propagation above copy
frames: MD5 has 1,269 `NaryAdd` and 414 Add top frames, CCITT 263 Table, 66
Bit, and 46 Fixed frames, and ALU 1,522 Add and 131 Table frames.  No global
clone-distance or actor-state change follows.

The dependency-ordered follow-up is deliberately limited to two tasks:

1. Add direct native named AND, OR, and XOR actors using bit-parallel exact
   support formulas, with the existing Table fallback for any alias case not
   handled exactly.  Retain each operation independently only with exact normal
   Logic/Conditional/MiniModel parity and repeatable CRC/ALU plus general logic
   gains; do not change generic Table.
2. After the direct logic actors establish the remaining channel share,
   prototype delta-local `Channel::Bit` advice using an ordinary Council and
   ViewAdvisor lifecycle.  Retain it only if avoided scheduling pays for
   advice fan-out, copying, disposal, and memory on both CRCs without harming
   the other examples.

No task follows for cost classes, `WordVarImp`, Fixed publication, Binary Add,
generic Table staging, or global search defaults.  Raw commands, counters,
samples, and temporary patches remain under `/private/tmp/word055-*`; no probe
or benchmark artifact is tracked.

### Structural performance measurements

Continue profiling after the direct OR/XOR and delta-local bit-channel changes
by measuring the shape of propagation and search, not only individual hot
functions.  The objective is to expose multiplicative costs that a focused
microbenchmark can hide: actor graph size, copied state per node, propagation
waves per branch, notification fan-out, information gained per execution,
recomputation amplification, and scaling with word width or model depth.

Use the five checked-in examples as the primary corpus and the established
independent Boolean CRC/MD5 comparisons where they answer a structural
question.  Record at least:

- root actor/advisor counts and estimated space-owned bytes by actor family;
- actor copies, advisor copies, and estimated copied bytes per search node;
- propagation calls per node, per committed input bit, and per newly fixed Word
  bit, separated by actor family;
- notification fan-out and useful-domain-change yield, including propagations
  that fail, subsume, narrow, or do no visible work;
- propagation-wave depth after a branch commit, rather than only aggregate
  `StatusStatistics` totals;
- scaling curves over at least three adjacent widths, unknown-bit counts, or
  round depths, using normalized slopes rather than repeated-root timing; and
- `c_d`/`a_d` sensitivity expressed as copied bytes versus replayed propagation,
  without changing the examples' defaults or inferring a global policy.

Compare these measurements with normal Gecode Int/Bool actor shapes and search
restoration behavior.  Distinguish implementation overhead from limitations of
the Word cube domain and from model topology.  A larger search tree with less
runtime can still be a good trade; a lower actor count is not automatically a
memory win when actors or advisors are larger.  Avoid proposing changes to the
kernel, global search defaults, or propagation conditions unless caller-local
and actor-local explanations have been ruled out with evidence.

Instrumentation remains temporary and detached.  Prefer counters at existing
actor/view/search seams, bounded samples, and model-side structural metrics;
do not add a durable profiler, benchmark runner, production switch, or private
result file.  Retained-scale runs are required only when a bounded scaling or
structural hypothesis would otherwise remain ambiguous.  The investigation
adds no normal tests; any later implementation task follows the established
focused `test/word` level for the contract it changes.

### Structural performance result

The exact baseline is `f1016bd3cd`.  At representative bounded scales, the
instrumented important families account for 112 of 154 root actors in MD5, 106
of 152 in SHA-1, 105 of 128 plus 40 advisors in each CRC, and 17 of 26 in the
ALU.  Estimated total space-owned lower bounds are approximately 14.5 KiB,
14.0 KiB, 8.3 KiB, and 1.4 KiB respectively.  The relevant concrete sizes are
184 bytes for Table plus its view array, 64 for native logic and Add, 80 for
NaryAdd plus its view array, 72 for Fixed, Ite, and Bit, and 24 for a
ViewAdvisor.  These are actor-state estimates rather than whole-Space or peak
RSS measurements.

Cumulative copy traffic is substantial even when live state is small.  The
measured important families average 8.37 copies and roughly 978 copied bytes
per node for MD5, 7.53/~948 for SHA-1, 12.23/~967 for CCITT, about 12.5/~990
for X-25, and 8.50/~716 for ALU.  These are cumulative allocator/copy traffic,
not simultaneously live bytes.  They explain why actor layout must be judged
together with recomputation, but no individual retained actor contains an
obviously avoidable large payload: the largest is the already-specialized
Table, and direct named logic has removed it from the dominant CRC path.

Useful-work yield is generally high.  MD5 Table is useful on 80.6 percent of
calls, Add on 83.5 percent plus 2.2 percent failure, NaryAdd on 82.7 percent
plus 3.6 percent failure, and native logic/Fixed on essentially every call;
Add and NaryAdd account for all 1,703 failures.  SHA-1 Table is useful on 67.2
percent, NaryAdd on 85.4 percent and accounts for all four failures, while
native logic/Fixed are again nearly always useful.  CRC native logic, Fixed,
Bit, and decided Ite are effectively fully productive; X-25 Ite alone has
16.1 percent no-visible-work, only about seven percent of total propagation.
ALU Table is useful on 69.0 percent, native logic on 73.3 percent, Add on 70.7
percent plus 3.7 percent failure, and Fixed on essentially every call.  Add's
26,675 failures and native logic's 39 account for all 26,714 ALU failures.

Word narrowing no-op rates remain 21 percent for MD5, 31 percent for SHA-1,
30 percent for each CRC, and 36 percent for ALU, but earlier caller-local
experiments did not turn this count into repeatable runtime improvement.  Per
newly fixed Word bit, total propagation is approximately 0.26 for MD5, 0.36
for SHA-1, and 0.42 for each CRC and ALU.  Observable important-family calls
per changed Word cube are approximately 0.83, 0.69, 1.02, and 0.78
respectively.  CRC additionally receives about 0.75 advisor notifications per
change, of which 69 percent concern unrelated bits and are now suppressed by
the retained delta-aware Bit actor.

Propagation-wave shape distinguishes the ALU from the other models.  Mean and
maximum actor executions in one status cascade are MD5 12.4/565, SHA-1
20.5/335, CCITT 21.0/82, X-25 22.0/70, and ALU 42.0/156.  Fifty-eight percent
of ALU waves exceed 32 executions, versus three percent for CCITT and six
percent for X-25.  This is a long dependency-chain effect, not evidence of
globally stale scheduling: the actors within those waves have high useful
yield.

Three-point scaling separates topology from implementation overhead.  CRC
message sizes 18/20/22 multiply both nodes and propagation almost exactly four
times per two additional inputs, keeping 22.8--24.0 propagations per node.
SHA-1 unknown-bit counts 168/172/176 similarly keep about 22.2 propagations per
node while the tree grows sixteenfold per four bits.  MD5 is less regular due
to failure pruning.  ALU widths 14/16/18 grow from 43.1 to 45.4 to 48.5
propagations per node in addition to roughly 4.4-times node growth.  This is
consistent with width-sensitive Add support work and the Word cube domain's
loss of cross-bit correlations, rather than broad redundant wakeups.

Independent CRC Word and Boolean recurrences have identical solutions,
checksums, and nodes.  At message sizes 18/20/22, Word roots contain
104/116/128 actors versus Boolean 285/319/353 and Word uses 40--44 percent
fewer propagations.  At 22 bits, CCITT uses 18.41 million versus 31.16 million
and reflected CRC 19.71 million versus 32.74 million.  Native Word topology is
therefore a compact structural win, not hidden actor inflation.

Search restoration remains workload-specific.  For CCITT-20, `c_d=1/8/32`
gives 17.85/22.84/23.02 propagations per node and 734/967/975 measured copied
actor bytes per node; `c_d=8,a_d=64` rises to 38.39 and 1,695 bytes.  Exact
production timings are repeatably about 0.43, 0.55--0.56, 0.56, and 0.82
seconds.  MD5 remains around 976--987 copied bytes per node and ALU around 716
across settings while replay propagation rises about 12 percent for MD5 and
eight percent for ALU.  This supports
CRC-specific invocation guidance, not a global search default or an actor
layout change.

The structural conclusion is deliberately negative.  Delta-aware Bit already
earns its advisor cost; important remaining actors have high useful yield;
native Word CRC is structurally smaller than its Boolean decomposition; and
ALU/hash scaling is dominated by search topology and cube-domain correlation
limits.  No new production task is justified.  Long ALU waves alone do not
establish a sound stronger propagation contract, and cumulative copy traffic
does not identify an avoidable actor payload.  A future model-documentation
change may record `c_d=1` as CRC-specific invocation guidance only after both
retained CRC scales confirm it; do not alter global or example defaults from
this investigation.

Commands are preserved in `/private/tmp/word058-commands.txt`, raw results in
`/private/tmp/word058-raw`, and the detached instrumented source/build in
`/private/tmp/gecode-word058-instrument-{src,build}`.  The independent Boolean
driver is `/private/tmp/word058-profile`.  No probe, raw result, or benchmark
artifact is tracked.

## Larger natural profiling instances

Task 064 surveyed natural public families from Wang et al.'s Wombit SMT-LIB
inventory and operation tables: BMC/software verification (`bmc-bv`,
`bmc-bv-svcomp14`, `uclid`), software/security verification cases (`spear`),
solver samples (`stp`), other solver/application samples (`sage`),
LFSR/core (`bruttomesso/core`, `bruttomesso/lfsr`), arithmetic synthesis
(`mcm`), DSP (`fft`), and packed-state puzzles (`rubik`).  The primary local
source is
`sources/derived/papers/2019-wang-wombit-a-portfolio-bit-vector-solver-using-word/fulltext.md`
in the CPKB, with its Wang 2016 and Wombit 2019 source records.  Six families
were considered: BMC/software verification, LFSR/PRNG recovery, ARX key
recovery, multiple-constant multiplication, FFT/fixed-point DSP, and
packed-state puzzles.  Three distinct natural families were retained as
single-root, hard-capped Release searches.

The per-family selection map is as follows, ranked by profiling value in this
task.

1. **ARX key recovery (retained).**  Public provenance is the independently
   selected, published Speck32/64 recurrence; it is not derived from Wombit's
   categories.  Wombit's `spear` entries are software/security verification
   cases such as CVS, OpenLDAP, and Samba, while `stp` contains solver samples.
   Speck exercises modular Add, rotate, XOR, equality,
   and a many-round key/data-flow graph: a fixed chain of Add, Fixed,
   native/fallback logic, and relation actors.  Unknown key bits and round
   count independently tune search and topology.  It gives a natural combined
   arithmetic/logic/structural profile with a concrete oracle.
2. **LFSR/PRNG state recovery (retained).**  Public provenance is Marsaglia's
   xorshift32 recurrence, corresponding to Wombit's `bruttomesso/lfsr`
   family.  It exercises fixed shifts, XOR, Word/Bool bit channeling, and
   observed relations in a repeated state-transition chain, including one
   initially advised Bit actor per observation.  Unknown seed bits and
   observation count tune search and topology.  It profiles linear bit
   dependencies, advisors, and clone-heavy search.
3. **Arithmetic inversion/MCM-adjacent multiplication (retained).**  Public
   provenance is exact bounded semiprime factor recovery, selected from
   Wombit's arithmetic and `mcm` operation mix rather than claimed as an MCM
   synthesis instance.  One Mult actor relates two Word factors to an exact
   product; unknown low bits tune difficulty without repeated roots.  Its
   minimal topology isolates multiplication inverse strength and cube-domain
   limitations, and permits a direct bounded IntVar comparison.
4. **BMC/software verification (not retained).**  Wombit's public `bmc-bv`,
   `bmc-bv-svcomp14`, and `uclid` families combine arithmetic, structural,
   comparison, reification, arrays, and mixed control/data actors in wide
   transition graphs; unwind depth and symbolic inputs tune difficulty.  This
   is highly valuable for the missing mixed Word/Int and reified paths, but a
   faithful instance requires a provenance-preserving SMT-LIB translator or a
   real program trace.  A hand-written substitute would repeat the already
   constructed ALU workload, so none was retained.
5. **FFT/fixed-point DSP (not retained).**  Wombit's public `fft` family
   suggests regular Add/Mult/shift/extract networks with layer/size and
   coefficient precision as natural controls.  It would profile broad regular
   arithmetic topology, but a faithful transform, rounding semantics, and a
   non-enumeration oracle exceeded the bounded task.
6. **Packed-state puzzle/Rubik (not retained).**  Wombit's public `rubik`
   family implies extract/concat/mask/equality actors over a large permutation
   graph, with scramble depth as the control.  It would profile structural
   actors and symmetry-heavy search, but faithful state encoding, move
   legality, symmetry handling, and branching would dominate the Word actor
   study.  It was rejected rather than replaced by a toy permutation.

Raw SMT-LIB import was rejected because its translator and payload provenance
were outside scope.  Existing MD5/SHA-1, CRC, and constructed ALU examples
were not retained as new families because tasks 051/052 already profile them.
The retained ranking is Speck first for operation breadth, xorshift second for
structural/advisor topology, and factor third for the cleanest isolated
diagnosis; the non-retained ranking is BMC, FFT, then Rubik by prospective
value versus faithful-construction cost.

The retained ARX case is a faithful reduced Speck32/64 known-plaintext key
recovery: 12 rounds, standard 16-bit recurrence and four-word key schedule,
key `1918:1110:0908:0100`, plaintext `6574:694c`, and independently computed
ciphertext `b97f:d091`.  With 20, 21, and 22 distributed unknown master-key
bits, exhaustive wall/user/sys seconds are 22.16/21.89/0.10,
43.96/43.62/0.16, and 87.47/86.89/0.35.  Propagations are 254,387,505,
507,274,228, and 1,015,565,796; nodes 2,097,151, 4,194,303, and 8,388,607;
failures 1,048,575, 2,097,151, and 4,194,303.  Each complete search proves
exactly one solution and recovers the concrete key.  A 24-bit calibration
exceeded the 120-second cap.
The retained root has 138 actors, no advisors, and one brancher.  Its measured
physical footprint is 1,696 KiB (1,712 KiB peak snapshot).  A bounded
five-second sample at 1 ms intervals records Add propagation as the dominant
top frame (2,206 samples),
then Word narrowing (637), Space status (139), native XOR narrowing (109),
Space cloning (106), XOR copy/propagation (55/50), Fixed narrowing/copy
(49/34), generic Table propagation (39), and Add copy (29).  This is primarily
native Add work; copy cost is visible but secondary.

The retained PRNG case is Marsaglia xorshift32 state recovery using the exact
`x ^= x << 13; x ^= x >> 17; x ^= x << 5` recurrence, seed `6d2b79f5`, 32
deterministic observations at bit `(7*i+3) mod 32`, and exhaustive recovery of
the unique seed (final state `5a7e189c`).  At 23, 24, and 25 unknown low bits,
wall/user/sys seconds are 8.99/8.81/0.04, 17.43/17.20/0.08, and
34.82/34.45/0.18; propagations 146,476,033, 292,951,909, and 585,903,169;
nodes 3,145,727, 6,291,455, and 12,582,911; failures 1,572,863, 3,145,727,
and 6,291,455.  The retained root has 224 actors and one brancher.  Its 32 Bit
channel actors each construct one advisor, although live post-propagation
advisor count is not exposed by Script statistics.  Physical footprint is
1,872 KiB (1,888 KiB peak snapshot).  Top sample frames are Word narrowing
997, XOR copy 663, Space clone 603, Fixed copy 276, XOR narrowing 256, Space
status/copy 236/134, Fixed narrow/propagate 124/95, and XOR propagate 69.
Exact doubling exposes lost cross-bit linear correlation in the cube domain;
the topology is also copy-heavy.

The retained arithmetic case recovers the bounded 28-bit primes
`0x0fffffc7` (268435399) and `0x0bffffdd` (201326557) from the exact 56-bit
product `0x00bffffb240007cb` (54043174657591243) with one native Mult actor.
At 24, 25, and 26 unknown low bits per operand, wall/user/sys seconds are
5.30/5.20/0.04, 10.56/10.45/0.06, and 21.38/21.14/0.09; propagations
18,285,339, 36,570,659, and 73,141,343; nodes 16,777,217, 33,554,433, and
67,108,865; failures 8,388,608, 16,777,216, and 33,554,432.  Each reports one
solution.  The root is one actor, no advisors, and one brancher; physical
footprint is 1,312 KiB (1,328 KiB peak snapshot).  Sample top frames are Mult
propagation 2,254, search next 208, Word narrowing 155, allocator work 198,
status 90, value selection 81, Mult copy 75, Space clone/copy 71/53, and commit
42.  The nearly complete powers-of-two tree is a propagation/domain limit, not
actor topology or generic scheduling overhead.

Two independent formulations check semantics.  A direct Bool-array xorshift
encoding expands every recurrence bit into Boolean XOR equations.  At 14, 16,
and 18 unknown bits, both formulations use exactly 32 observations and recover
the same single seed.  Word uses 224 actors,
286,471/1,144,815/4,577,768 propagations, and 6,143/24,575/98,303 nodes;
Bool uses 3,049/3,053/3,057 actors,
1,354,359/5,548,201/21,493,621 propagations, and
12,287/49,151/131,071 nodes.  Search counters differ because branching
topology differs; bit blasting is not a free remedy.  An independent IntVar
factorization at a host-safe adjacent scale (`p=32749`, `q=24593`) gives the
same 1/2/2 solution counts at 12/13/14 unknown bits.  Word takes
4,463/8,931/17,855 propagations and 4,097/8,193/16,385 nodes, whereas Int takes
1/3/3 propagations and 1/3/3 nodes.  Interval multiplication recovers bounded
factors immediately; the Word cube and current prefix inverse do not.

The ranked diagnosis is therefore: first, a cube-domain/fixed-product inverse
limit in Mult; second, algorithmic Add work in Speck; third, search topology
and clone/copy cost in xorshift, under a more fundamental loss of linear
correlation.  Generic scheduling and actor footprint are not supported as
primary causes: measured footprints remain 1.3--1.9 MiB and samples point to
actor algorithms or copying caused by large search trees.  Collective natural
coverage includes Add, Mult, XOR, rotate/shift, fixed/equality relations,
Word/Bool channeling, native/fallback logic, and DFS cloning/recomputation.
Honest gaps are variable shifts, div/mod, element/count, mixed Word/Int actors,
and reified Word relations.  No artificial constraint was added merely to
claim those paths; a faithful public BMC or arithmetic-program import is the
appropriate future corpus work.

The only production-oriented follow-up justified by these measurements is a
bounded investigation of fixed-product Mult inverse support when operands have
known ranges or high prefixes.  Compare the smallest sound refinement with the
current actor on this factor case, ordinary Mult tests, and representative
multiplication workloads, requiring exact semantics/lifecycle/alias safety and
material node or wall-time improvement.  If useful support requires relational
state outside a Word cube, record rejection.  These samples do not independently
justify another Add, XOR, Fixed, advisor, or generic scheduling task.

Exact baseline was `b713321d61`.  Commands are preserved in
`/private/tmp/word064-raw/commands.txt`, hashes in `SHA256SUMS`, raw curves and
parity results in `/private/tmp/word064-raw`, samples in
`{speck,xorshift,factor}.sample.txt`, footprint evidence in `footprints.txt`,
and temporary sources as `/private/tmp/word064-*.cpp`.  No driver, profile, or
third-party input is tracked.

## Binary Add algorithmic acceleration

Task 065 starts from exact Release baseline `5546a91361` and isolates the
current binary/AddCarry automaton.  Each call first scans the width to build an
alias-filtered allowed-tuple mask, scans forward reachability, scans backward
reachability, then scans again to project supported zero/one values into all
three Word cubes before changed-only publication.  Temporary ALU-8/18 counters
record 3,199,586 calls; 57,592,548 allowed-tuple bit scans; 56,459,731 forward,
55,395,288 backward, and 55,395,288 support-projection bit scans; 635,295,334
tuple-transition visits; 2,589,589 publications; 2,378,315 calls that narrow a
view; and 122,070 internal failures.  Thus 74.3 percent of calls narrow and
3.8 percent fail.  This rejects staged/advisor execution: most calls perform
useful exact work, while a Council would add advice, clone, and disposal state.

Exact bit consistency has an Ω(width) floor even if carry reachability is
composed broadly: the actor must inspect each bit's potentially distinct,
alias-sensitive tuple mask and emit supported zero/one masks for three views.
A portable broadword/block transfer therefore cannot eliminate projection and
would add boundary-summary and alias complexity.  Two exact conventional
candidates were nevertheless prototyped.  The first fuses tuple construction
with forward reachability and backward reachability with support projection,
leaving two directional linear scans.  The second also uses an immutable
process-global table keyed by the 8-bit allowed-tuple mask and two-bit carry
state masks to precompute forward, backward, and six-value support summaries.
It adds no actor member, subscription, event, copy, or disposal state.  The
table is about 6 KiB; actor count and clone footprint are unchanged.

Both candidates preserve exact focused Arithmetic Add/Lifecycle/TestFramework
results.  On exhaustive symbolic ALU-8/18, five alternating fresh-process
trials preserve 26 actors, 65,810 solutions, 18,230,041 propagations, 375,811
nodes, and 122,096 failures.  Median user time falls from 2.25 seconds to 1.56
with fusion and 1.20 with fusion plus tables.  On exhaustive Speck32/64 at 12
rounds and 18 unknown key bits, three trials preserve 138 actors, one exact
key, 64,089,495 propagations, 524,287 nodes, and 262,143 failures.  Median user
time falls from 6.96 seconds to 5.09 and 3.91 respectively.  MD5-16/148 is an
independent secondary guard: three exhaustive trials preserve 154 actors,
1,048,576 solutions, 29,548,308 propagations, 2,114,231 nodes, and 8,540
failures; table-candidate user time is 7.25--7.39 seconds versus baseline
8.49--8.57.

The retained-scale ALU-12/22 result confirms the end-to-end benefit.  Baseline
and table candidate both use 40 actors and enumerate 2,367,275 solutions with
546,624,487 propagations, 6,081,663 nodes, and 673,557 failures.  User time is
78.51 versus 39.83 seconds, a 49 percent reduction.  The fused-only candidate
is sound and useful but rejected because the table candidate consistently
dominates it.  No weaker propagation was measured or retained.  AddCarry uses
the same exact terminal-carry table path, and existing normal tests cover its
terminal carry, assigned/partial/inverse support, aliases, width 64, failure,
clone, recomputation, and subsumption seams; no new test is warranted for this
implementation-only change.

Best-effort high-water RSS from `/usr/bin/time -l` was unavailable because its
macOS `sysctl kern.clockrate` query is blocked in the execution sandbox.  This
is a measurement limitation, not a silent zero.  The actor and clone state are
unchanged, and Mach-O segment sizes remain equal (`__TEXT` 557,056 bytes,
`__DATA` 16,384 bytes) between baseline and retained libraries.

Temporary sources/builds are `/private/tmp/gecode-word065-{base,fused,lookup}`
and corresponding `-build` directories.  Exact commands are in
`/private/tmp/word065-raw/commands.txt`, identities in `SHA256SUMS`, raw
instrumentation/timing/statistics in `/private/tmp/word065-raw`, and the Speck
source is `/private/tmp/word064-speck.cpp`.  No counter, driver, profile, or
benchmark artifact is tracked.

The same lookup/fusion mechanism was audited across the remaining Word
actors.  Neg and Sub/SubBorrow are the only direct structural matches.  Neg is
a two-state carry automaton over four `(x,z)` tuples with separate forward,
backward, and support scans.  An exact immutable summary needs 16 allowed-tuple
masks by four carry masks for forward and backward, plus 16 by four by four for
four value-support bits: 384 bytes total.  Sub and SubBorrow use two borrow
states and eight `(x,y,z)` tuples, so they can use Add's dimensions: 256 by
four forward/backward and 256 by four by four six-bit support, 6,144 bytes.
For both, an alias-filtered allowed mask can be built while scanning forward,
and backward reachability can be fused with support projection.  This removes
one width scan and repeated `2×4` or `2×8` transition loops while preserving
exact support, aliases, and terminal borrow.

This structural match is not yet performance evidence for retention.  None of
the five realistic checked-in examples is materially Neg- or Sub-heavy; they
appear only in ordinary arithmetic coverage.  Add's measured benefit proves
that the pattern can matter when such an actor dominates, but it cannot be
transferred as a timing claim.  One bounded follow-up investigation is
justified: prototype Neg and Sub/SubBorrow summaries independently and compare
them on a natural subtract/negate-heavy workload plus ordinary forward,
inverse, alias, lifecycle, and terminal-borrow cases.  If no representative
workload makes them hot, close the candidate without production changes.

The other actors do not share Add's small-state per-bit loop.  Named binary
logic, n-ary logic, reductions, mask/Bool ITE, fixed shifts, Bit channeling,
and equality already use broadword formulas, direct masks, or delta-local
advice.  Generic Logic::Table iterates up to 16 tuples but processes all word
bits simultaneously in WordValue masks and already runs local closure; named
operations bypass it, and earlier bounded-arity specialization was rejected.
Ordering uses early-exit MSB numeric scans whose second pass depends on the
first operand's pruning, not a finite tuple automaton.  Variable shift has up
to width whole-word shift classes and repeated cube-hull closure, making table
state grow with width.  Mult, division/modulo, combined/product-mod, and their
signed forms use whole-word prefix, interval, inverse, or assigned arithmetic.
NaryAdd carry state grows with dynamic arity and its closure is already
optimized.  Concat/extract and remaining structural actors use direct masks.
Lookup tables for these paths would either duplicate formulas, explode with
width/arity, or worsen locality.  No follow-up for them is justified by this
audit.

The detailed classification and table-size estimate is preserved in
`/private/tmp/word065-cross-audit.md`; source-audit commands are appended to
`/private/tmp/word065-raw/commands.txt`.  No cross-propagator source change was
made in task 065.

## Fixed-product multiplication inverse propagation

Task 066 starts from exact Release baseline `801107b187`.  The retained Mult
refinement applies only when the result is fixed and the operands' numeric
cube extrema prove `xmax*ymax <= 2^width-1`; the proof uses division before
multiplication, so it is safe at width 64.  In that non-wrapping case, ordinary
quotient bounds project each operand to the sound cube hull of its feasible
numeric interval.  When either resulting interval spans at most 64 values,
bounded exact factor-pair enumeration additionally projects the supported
operand cubes.  Modular/wrapping cases keep the existing prefix propagation
unchanged.  The implementation adds no actor, subscription, advisor, or
persistent state, and contains no primality- or instance-specific rule.

An exhaustive temporary oracle checked all 102,195 combinations of cube
domains and alias mappings at widths 1--3.  It covered distinct operands,
`x==y`, `x==z`, `y==z`, and all-equal views; 26,195 cases had concrete
solutions and 14,277 included wrapping solutions.  No supported tuple was
removed and no supported case failed.  Normal MultLifecycle coverage adds a
fixed-product narrowing and failure seam, an operand alias, width 64, actor
subsumption at solutions, cloning through the existing lifecycle, and genuine
`c_d=1` replay.

The retained semiprime curve recovers the same unique ordered factors
`p=0x0fffffc7`, `q=0x0bffffdd` and product `0x00bffffb240007cb` in baseline and
candidate.  At 24/25/26 unknown low bits, baseline has
16,777,217/33,554,433/67,108,865 nodes,
8,388,608/16,777,216/33,554,432 failures, and
18,285,339/36,570,659/73,141,343 propagations, with driver runtimes
5.222/10.455/21.442 seconds.  The candidate has 29 nodes, 14 failures, and 30
propagations at every scale; five fresh-process trials per scale report
0.083--0.133 milliseconds of driver runtime.  Both retain one Mult actor and
one brancher at the root and the same single solution.  The improvement
therefore clears the adjacent-scale node and repeatable end-to-end retention
gate by a wide margin.

A wrap-heavy fixed-product control preserves exactly 8 solutions, 257 nodes,
121 failures, and 279 propagations.  A free-result general multiplication
control preserves 65,536 solutions, 131,071 nodes, zero failures, and 142,849
propagations.  Five trials show only process-startup noise and no material
regression.  The checked-in symbolic ALU and MD5 examples do not post Mult, so
they are not claimed as multiplication-path guards.  A bounded baseline u26
sample has 2,374 Mult propagation samples out of 4,029 main samples, consistent
with the prior diagnosis.  The candidate finishes too quickly for a meaningful
sample.  Best-effort high-water RSS remains unavailable because macOS
`/usr/bin/time -l` cannot read `kern.clockrate` in the sandbox; actor state is
unchanged.

Temporary sources and builds are `/private/tmp/word066-{soundness.cpp,
mult-control.cpp}`, `/private/tmp/gecode-word066-{base,candidate}` and their
`-build` directories.  Exact commands are in
`/private/tmp/word066-raw/commands.txt`; raw statistics, solution output,
timings, the full exhaustive soundness summary, and the baseline sample are in
`/private/tmp/word066-raw`.  The exact shared-source hash and temporary source,
binary, and output hashes are recorded in `SHA256SUMS`.  Baseline and candidate
solution output both normalize to `semantic-solution.txt`, whose hash records
the exact solution count and factor/product tuple independently of runtime
statistics.  No benchmark driver or instrumentation is tracked.

## Hybrid cube and integer-range domain investigation

Task 067 uses exact baseline `1ec7ada451` and reaches a **no-go** decision for
changing every WordVar to a hybrid numeric domain.  The useful candidate is
not an extrema cache: cube `lo` and `hi` already are its unsigned extrema.  It
is the cube intersected with numeric bounds.  Supporting unsigned and signed
clients simultaneously requires two independent intervals, stored as four
`WordValue` endpoints.  Signed order is represented without host signed
conversion by rank `encoded ^ sign_bit`.  A signed interval is contiguous in
rank space; across zero its encoded set splits into negative high-bit and
nonnegative low-bit segments.  Width one and width 64 use the same transform.
Values outside `Int::Limits` remain valid Word values but cannot enter an Int
actor.

Canonicalization computes the first and last cube member admitted by both
intervals, fails if none exists, and feeds newly fixed bits back into the cube.
A production algorithm can use constrained-bit successor/predecessor over the
at-most-two encoded signed segments.  The throwaway exhaustive oracle passed
1,899 cases: exhaustive widths one and two, every width-three cube against 36
unsigned/signed boundary pairs, targeted widths four/eight, and exact width-64
sign/rank/assignment arithmetic.  It also passed 2,708 monotone tells and 1,318
copy/replay-equivalence checks, including empty intersections, assignment,
zero/sign boundaries, and signed/unsigned extrema.  An initially broader
cross-product hit the 60-second hard cap and is not used as evidence.  These
1,318 checks are plain copy equivalence, not Gecode search replay; genuine
`c_d=1` replay is exercised by the parity and mixed-workload DFS models below.

Signedness belongs in views, not immutable variable state: the same Word must
remain usable by modular actors plus unsigned and signed relations.  But views
alone cannot create shared persistent precision.  An interpretation-neutral
VarImp would carry unsigned and signed-rank intervals, while signed/unsigned
views select their order and tells.  Current `WordVarImp` is 48 bytes; four
endpoints make it an estimated 80 bytes, adding 32 bytes or 67 percent to every
variable and clone.  WordView and IntView are each eight bytes.  This excludes
event-list growth and canonicalization on every tell.

The Int protocol audit rejects a blanket claim that a view is merely
`min/max`.  A temporary Word-backed adapter directly instantiated established
`Int::Rel::Lq` and `Int::Linear::EqBin` templates and propagated two actors.
Bounds order and bounds linear arithmetic can therefore reuse templates after
a complete adapter supplies bound tells, aliases, update, subscription,
`View::me`, and honest BND events.  Value-consistent distinct additionally
needs exact `nq`; bounds-consistent distinct needs BND scheduling and array
lifecycle.  Domain-consistent distinct requires range/value iterators and
iterator tells; a cube intersected with two ranges cannot provide the Int DOM
protocol without enumerating/materializing holes, so direct reuse is rejected.
`Arithmetic::MultBnd` is hard-coded to `IntView`; ordinary posting cannot
accept the adapter and needs a new actor/overload or channeling.  The same
warning applies to actors assuming Int deltas, regret/size, or DOM iteration.
Gecode's Int range is only `[-2147483646,2147483646]`, excluding many width-32
values and nearly all width 64.

Honest temporary bit-channel models compare established Int actors with
ordinary IntVar models.  At width three, value- and bounds-consistent distinct
both preserve 336 solutions and checksum 30,530,808; order preserves
84/2,629,914; linear addition 36/1,706,736; and bounds multiplication
31/1,454,300.  All searches use `c_d=1`.  Channeling costs are visible:
distinct rises from 64/79 to 1,563/1,578 propagations and from 671 to 687
nodes; order from 62 to 487 propagations; linear from 71 to 385; multiplication
from 62 to 318.  This establishes exact semantic/replay parity, but not a
zero-cost direct view.  It confirms that Val distinct needs exact interior
exclusion supplied by Int rather than an unsound cube `nq`.

Mathematical integer arithmetic through a view remains distinct from modular
Word arithmetic.  Int-view `x+y=z` can fail on overflow and is restricted to
Int limits; Word `add` wraps modulo the width.  Integer multiplication likewise
is not native modular Mult.

A passive four-endpoint VarImp prototype preserves exact counters on
fixed-product u26 (one solution, 30 propagations, 29 nodes, 14 failures), ALU
default (65,810 solutions, 18,230,041 propagations, 375,811 nodes, 122,096
failures), CCITT default (65,536/2,993,199/131,071/0), X-25 default
(65,536/3,145,494/131,071/0), MD5 default (16/1,364/59/14), and SHA-1 default
(16/1,226/41/5).  Three fresh-process trials are noisy for ALU and tiny hashes;
CRC timing suggests at most small overhead but is mostly noise: CCITT ranges
overlap (0.139--0.145 versus 0.142--0.143 seconds), while X-25 is consistently
0.145--0.146 versus 0.149 seconds in only three trials.  This
prototype only copies and maintains derived endpoints; real canonicalization
and new notifications can cost more.  Fixed-product Mult already obtains the
useful non-wrapping range refinement locally.  Existing unsigned/signed order
and division actors likewise derive range hulls where their contracts use
them; variable shift mixes whole-word classes for which one interval does not
encode the lost relation.  A separate `c_d=1` mixed DFS confirms passive-state
exact parity: unsigned div plus comparison has 56 solutions, checksum
961,804,019,540, 127 nodes, 8 failures, and 127 propagations; signed order has
903/29,620,923,279,647/1,847/21/2,898; variable shift has
4,095/93,736,674,455,328/8,191/1/3,000.  Hash/CRC controls have many Word
variables but no integer-range consumers, making the universal clone tax pure
overhead.

Events are another kernel cost.  `ME_WORD_BITS/VAL` cannot distinguish an
unchanged cube with tightened unsigned or signed bounds.  Correct support needs
bound-visible scheduling and endpoint deltas, or conservative scheduling of
all Word actors.  The former changes VarImpConf, views, branchers, trace,
clone/replay, and subscribing actors; the latter discards hoped-for efficiency.
Bit branching also needs canonicalization before selection and after commits.
The oracle performed 2,708 attempted range tells.  Exactly 136 successful tells
changed the represented set, and four of those changed numeric bounds without
changing cube masks.  Those four prove that a bound-visible event is required:
current `ME_WORD_BITS` cannot honestly encode the change.  This synthetic
oracle does not estimate how often such events occur in real workloads;
failed candidate tells are excluded from the event counts.  Best-effort
`/usr/bin/time -l` memory was unavailable because
sandboxed macOS cannot read `kern.clockrate`; exact object sizes and the
+32-byte clone payload are retained instead.  Adjacent width-one/two oracle
totals are 27/927 domain cases and 60/2,708 tells.

The result is no-go for an expand/migrate/contract program, so no implementation
task split is proposed.  Range reasoning should remain local to measured
actors.  If a future natural model needs several mathematical Int constraints
over the same representable Word values, the smaller separately approved
investigation is an explicit opt-in channel/adapter variable, not universal
WordVar kernel migration.

Temporary sources, builds, commands, raw statistics, and hashes are under
`/private/tmp/word067-*` and `/private/tmp/word067-raw`.  No production, API,
test, example, or build file is changed.

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
