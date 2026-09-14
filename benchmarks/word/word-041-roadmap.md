# Word application and corpus roadmap

This report separates results already observed in the Word campaign from
experiments that have not yet been run. It recommends one next application
experiment and one bounded compatibility corpus. Neither recommendation is a
commitment to a new public importer or solver architecture.

## Evidence and application order

1. **Device and packet configuration — demonstrated fit.** The DMA and register
   families in the [word-037 campaign](word-037-result.md) preserved exact
   public results across native formulations and Z3. The bounded DMA candidate
   had a 1.06x paired median advantage over its baseline in the broad screen.
   More directly, the [word-038 packed-header experiment](word-038-result.md)
   matched an independent 4,096-projection oracle and cut nodes from 16,381 to
   8,191, failures from 4,095 to zero, propagations by 28%, and median time by
   42%. Fixed-width fields, masks, lookup tables, alignment, and bounded
   arithmetic therefore match capabilities that have already paid for
   themselves in realistic small models.
2. **Short-horizon occupancy and timetabling — best next experiment.** This is
   a hypothesis, not a campaign result. Demirovic and Musliu encode timetable
   times and resource occupancy as bitvectors and use Boolean operations for
   clashes and availability; they also warn that unassigned resources require
   many more bitvectors. Their [primary paper](https://repositum.tuwien.at/bitstream/20.500.12708/338/2/Demirovic%20Emir%20-%202016%20-%20Modeling%20high%20school%20timetabling%20with%20bitvectors.pdf)
   supports testing the deliberately narrower preassigned-resource model below.
3. **Bounded verification / QF_BV — compatibility corpus.** Program analysis
   and verification are established bit-vector applications. Wombit combines
   word-level propagation with learning and a bit-blasting portfolio
   ([Wang, Søndergaard, and Stuckey 2019](https://doi.org/10.1007/s10817-018-9493-1));
   Alt-Ergo reports bit-pattern and interval propagation on SMT-LIB and
   industrial verification inputs
   ([Ait-El-Hara et al. 2025](https://ceur-ws.org/Vol-4008/SMT_paper20.pdf)). Those systems
   justify a compatibility check, but not an expectation that Gecode's current
   DFS model will beat a specialist SMT solver.
4. **Circuit-topology synthesis — different model and search.** Lagoon and
   Metodi select add/subtract/shift nodes, their predecessor indices, and a node
   bound, then prove feasibility or infeasibility to obtain an optimal
   multiplication-by-constant circuit
   ([primary paper](https://modref.github.io/papers/ModRef2020_Deriving%20Optimal%20Multiplication-by-Constant%20Circuits%20With%20A%20SAT-based%20Constraint%20Engine.pdf)). The
   [word-040 experiment](word-040-result.md) instead related the high and low
   halves of one general product. Its large actor and clone-memory advantage
   over a Boolean schoolbook network does not supply topology variables,
   symmetry breaking, optimization search, or a scalable inverse propagator.
   Circuit synthesis is therefore fourth and must be scoped separately.

## Next application experiment: one-word occupancy

Use a feasibility-only timetable whose horizon is exactly 32 or 64 discrete
slots and whose resources are preassigned. Each event has a fixed duration and
a small finite set of allowed starts. A start choice selects a precomputed
constant occupancy mask with consecutive one bits. For every resource, OR the
masks of its events; availability requires each selected mask to be a subset
of a constant availability mask, and pairwise or accumulated intersections
must be zero. Keep event choice variables as the public projection and derive
occupancy Words from them. Do not add room assignment, soft costs, population
counts, multiple Words per horizon, or optimization in this experiment.

Retain the model as a continuing benchmark only if all of the following pass:

- An exact independent oracle enumerates the complete projection set for tiny
  cases. Paired SAT and UNSAT instances exercise both availability and resource
  clashes.
- Three calibrated scales vary events, choices per event, resource density,
  and occupied slots without changing semantics. The largest accepted scale
  must finish under the cap in every compared configuration.
- Release runs interleave the Word model, a native Gecode Bool/Int control, and
  Z3 under identical wall, CPU, node, solution, and memory caps. A timeout,
  memory limit, unknown result, or process error is never classified as UNSAT.
- Every row records status, public projection count and checksum, nodes,
  failures, propagations, median time and observed spread, relation and total
  actor counts, and retained-clone RSS after allocator warm-up.
- All formulations have status and projection parity. Word must remain within
  2x the native control's median runtime at every retained scale and either
  reduce nodes or failures by at least 20% on one calibrated, non-root-solved
  pair, or show a measured actor-count or retained-clone-memory advantage.

Failing that gate means the timetable remains a documented negative result;
it does not trigger a production constraint or a larger scheduling suite.

## GF(2): no implementation task now

The current linear control does not justify a global constraint. Of the ten
linear CRC/xorshift fixtures in word-036, only `xorshift-rounds` exposed a
full-rank system while native root propagation left correlated public bits
unknown. Neither word-036 nor the subsequent campaign compared a GF(2)-enhanced
Gecode formulation, so the campaign provides no application-level GF(2)
search or timing evidence. This is one correlation-loss witness, not evidence
for a production domain or a general affine actor.

Reconsider only a **model-level affine constraint** after a realistic repeated
linear network shows consistent rank pruning and a material reduction in
search or runtime against the current decomposition. Any follow-up must report
rank/nullity beside native root fixing and then pass exact projection parity
and capped repeated measurements. A GF(2) kernel variable domain remains out
of scope regardless of that result.

## Deterministic public QF_BV sample

The source is the official [SMT-LIB 2025 non-incremental
release](https://zenodo.org/records/16740866), specifically its `QF_BV`
archive. The release records each benchmark's contributor and licence in file
metadata. Use the [SMT-LIB explorer](https://explore.smt-lib.org/) metadata to
identify source family and status, and retain the release DOI, archive MD5,
relative path, file SHA-256, family, status, licence, byte size, declaration
count, widths, and feature stratum in the generated manifest.

The manifest contains exactly 24 inputs: six in each of these exclusive
strata, with three SAT and three UNSAT per stratum:

| Stratum | Required distinguishing features |
| --- | --- |
| structural/bitwise | Boolean core, equality, literals, concat/extract or bitwise operations; no arithmetic, comparison, shift, or extension operator |
| arithmetic/comparison | add/sub/mul, division/remainder/modulus, or signed/unsigned comparison; no shift or extension operator |
| shift/extension | fixed-width shifts, repeat, zero extension, sign extension, or constant indexed rotation; no operator from the mixed rule below |
| mixed bounded verification | at least one structural/bitwise feature and features from at least two of arithmetic/comparison, shift/extension, and Boolean control (`ite` or nontrivial Boolean nesting) |

Eligibility is unconditional before sampling: `set-logic QF_BV`, known SAT or
UNSAT status, exactly one `check-sat`, no command after it except `exit`, all
widths in 1..64, a metadata licence permitting redistribution, decompressed
size at most 262,144 bytes, and at most 512 declarations plus zero-argument
definitions. The adapter described below must accept the whole file. Reject a
file on any unsupported command, sort, term, indexed operator, malformed or
missing metadata field, or cap violation before constructing a Gecode Space.

Assign eligible files to `mixed`, then `shift/extension`, then
`arithmetic/comparison`, then `structural/bitwise` in that precedence. Within
each stratum and status, group by explorer source family; sort family names by
their UTF-8 byte sequence and each family's candidates by `(SHA-256,
relative-path)`. Select in lexical round-robin passes over families, taking at
most two files from any family in a stratum, until three files are selected.
The selection is valid only if each stratum contains at least two source
families. Record the exact explorer export/version used. Any shortage is a
manifest-generation failure, never permission to relax a rule silently.

This sample is a compatibility and regression corpus. Compare Release Gecode
and Z3, and record Bitwuzla when available. Z3 documents fixed-size bit-vector
operations and signed/unsigned distinctions in its
[bit-vector guide](https://microsoft.github.io/z3guide/docs/theories/Bitvectors/);
[Bitwuzla](https://github.com/bitwuzla/bitwuzla) is an official specialist
solver, and its relevance is independently visible in the
[SMT-COMP 2025 QF_BV single-query results](https://smt-comp.github.io/2025/results/qf_bv-single-query/).
These references select controls; they do not predict a Word performance win.

## Benchmark-only S-expression adapter

The adapter is test infrastructure, not a public SMT-LIB importer. It parses a
complete S-expression stream, validates and type-checks it into an immutable
intermediate form, and constructs a Space only after successful validation.
The accepted script surface is `set-logic QF_BV`, `set-info` metadata,
`declare-const`, zero-argument `declare-fun` and `define-fun`, `assert`,
`check-sat`, and `exit`. Terms may use `let`, Core `true`, `false`, `not`,
`and`, `or`, `xor`, and `=>`, equality, `distinct`, `ite`,
binary/hexadecimal and width-checked indexed bit-vector literals, and:

- `bvnot`, `bvneg`, `bvand`, `bvor`, `bvxor`;
- `bvadd`, `bvsub`, `bvmul`, `bvudiv`, `bvurem`, `bvsdiv`, `bvsrem`, `bvsmod`;
- `bvult`, `bvule`, `bvugt`, `bvuge`, `bvslt`, `bvsle`, `bvsgt`, `bvsge`;
- `concat`, indexed `extract`, `repeat`, `zero_extend`, and `sign_extend`;
- `bvshl`, `bvlshr`, `bvashr`, `bvcomp`, and indexed constant rotates only
  when the indexed amount can be represented directly by the fixed-width Word
  expression. Variable rotates are rejected.

Follow the official [QF_BV logic](https://smt-lib.org/logics-all.shtml),
[language releases](https://smt-lib.org/language.shtml), and
[FixedSizeBitVectors semantics](https://smt-lib.org/theories-FixedSizeBitVectors.shtml).
All operations are width typed. Arithmetic wraps modulo `2^w`; signed
operations interpret the same `w` bits in two's-complement order; division,
remainder, modulus, overshift, extension, extraction, and zero-divisor cases
must match the cited theory exactly. `concat` produces the sum of operand
widths and is accepted only when that result is at most 64. Literals must fit
their declared width exactly. No value, width, shift amount, or intermediate
result is silently truncated.

Reject arrays and uninterpreted functions, quantifiers, floating point,
optimization, incremental push/pop or multiple checks, proofs and unsat cores,
widths above 64, variable rotates, and every Int/BV conversion (`ubv_to_int`,
`sbv_to_int`, `(_ int_to_bv m)`, and legacy spellings). The official conversion
semantics map a bit-vector to an unbounded nonnegative or signed integer and map
an integer to its residue modulo `2^m`; Gecode Word has no unbounded Int carrier
and its bounded `IntVar` cannot preserve that contract generally. Rejecting the
whole input before construction is therefore the only accepted behavior.

## Strategic decisions

- **Learning and backjumping:** deferred solver-wide. Wombit's explanations,
  learning, conflict handling, and portfolio choice are architectural features,
  not a Word propagator follow-up.
- **Arrays:** separate and excluded. They require an array theory and memory
  model beyond QF_BV Word expressions.
- **Multiword representation:** separate and excluded. Neither the timetable
  nor corpus may chain Words to admit widths above 64.
- **Tooling:** only the deterministic manifest generator and benchmark adapter
  are contemplated. There is no public importer, supported SMT-LIB frontend,
  or API commitment.
- **FlatZinc:** no delta. This roadmap adds no FlatZinc syntax, translation,
  examples, tests, or runtime behavior.
