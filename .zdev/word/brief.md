# WordVar follow-up

## Objective

Complete and harden native bounded WordVar integration from the reviewed correctness, performance, search, and modeling findings.

## Context

The fixed-width WordVar subsystem and its optional signed or unsigned bounds
are implemented. The retained design history, measurements, and completed task
record remain in `.zd/word/brief.md` and `.zd/word/tasks/`. This area owns the
follow-up work found by the August 2026 review: current API defects, avoidable
domain work, missing Gecode integration, and bounded propagator families with
concrete modeling or benchmark value.

## Settled decisions

- Keep one public `WordVar` type with compact cube, unsigned-bounded, and
  signed-bounded implementations selected at construction.
- Keep compact Word models on the existing bit-event path. Bounds-specific
  state, subscriptions, synchronization, and arithmetic must be selected
  statically through typed views wherever practical.
- Use ordinary Gecode variable implementations, views, propagators, rewriting,
  propagation conditions, costs, cloning, recomputation, tracing, and
  branching. Do not add side registries or a second scheduling mechanism.
- Batch local cube and interval deductions and synchronize each distinct role
  once per local pass. Publish through normal VarImp tells, skipping calls that
  are locally known to be unchanged.
- Adaptive bounded propagation is evidence-gated. Prototype it first for
  binary Add, retain exact cube closure while unclassified, and generalize only
  after repeatable search or runtime gains without a material compact or
  symbolic-model regression.
- Preserve cube-default MiniModel behavior. Any bounded lowering policy must be
  explicit rather than inferred from an operator.
- Improve search and tracing with cheap, saturating bounded-domain measures;
  do not compute expensive exact cube/interval cardinalities in branch merits.
- Add bounded variants only where a realistic model or retained measurement
  shows useful interval propagation. `product_mod`, n-ary Add,
  Boolean-controlled ITE, and Element are the current candidates.
- Evaluate one native bounds-consistent Word `distinct` before considering any
  broader integer-compatible view. Do not promise general Int propagator reuse
  across 64-bit Word values or cube holes.
- Do not split compact and bounded words into separate public variable types,
  redesign `WordDelta` without profile evidence, revive the rejected
  incremental Element scan, or add FlatZinc, arbitrary precision, SAT
  explanations, affine/GF(2) domains, or global contradiction analysis here.

## Delivery boundaries

- Each performance task must name the control model and retain a candidate only
  on measured evidence. A clean rejection with preserved evidence is a valid
  result only for a task explicitly framed as an investigation.
- Keep examples ordinary Gecode `Script` models. Avoid a new benchmark
  framework when extending the existing Word benchmark is sufficient.
- Prefer small direct changes over new shared abstractions. The two existing
  local-domain helper families may remain separate unless a task demonstrates
  harmful behavioral drift.
- Document public exceptions and endpoint semantics as part of the relevant
  API task rather than creating documentation-only machinery.

## Shared mixed model

The durable comparison model is a six-entry DMA descriptor window, exposed as
an ordinary `Script` example with descriptor count 3 through 9 as a scaling
option. Address and end fields are 12-bit values. Lengths repeat
`0x20, 0x30, 0x20, 0x40, 0x30, 0x20`. For a descriptor count `n`, the window
starts at `0x100` and ends at `0x100 + sum(length[0..n)) + 0x20`; bases lie
between the window start and `window_end-0x20`, have their low four bits fixed
to zero, and satisfy `end[i] = base[i] + length[i]`,
`end[i] <= base[i+1]`, and `end[i] <= window_end`. Each four-bit flag Word is
one of read-only `0x1`, read-write `0x3`, or read-execute `0x5`. Exactly
`floor(n/3)` flags are write and the same number are execute; unsigned
`flags[i] <= flags[i+1]` breaks the interchangeable-descriptor symmetry.

An `IntVar` index selects base, end, and flags through Element. The selected
write bit controls a Boolean ITE choosing either `selected_end + 0x10` or
`selected_end` as `selected_limit`, constrained to at most `0x300`. Search
branches on the index, flag bits, bases, and remaining derived values in that
order, using each formulation's native value selector. Equivalence is the
solution count plus the unsigned 64-bit wrapping sum of
`index + 1 + selected_limit + sum((i+1)*base[i] + flags[i])` over solutions.

The selected-write ITE result is constrained to at most `window_end`. The
example supports compact Word, bounded Word, compact Word plus one
channel-equal IntVar for each numeric Word, and equivalent Int/Bool
formulations. `benchmarks/word/benchmark.py`, its public manifest, and README
are extended to run these four modes. Byte evidence always reports current
object sizes and an RSS slope from retained clone populations of 2,000, 8,000,
and 32,000; if the RSS delta is below 16 MiB, multiply all populations by four
until it is measurable or 256,000 clones are retained, and report an explicitly
unmeasured slope if process limits prevent that run.

## MiniModel bounded policy

Cube-default operator syntax remains unchanged. Explicit bounded Boolean
lowering uses `word_rel(const WordExpr&, WordRelType, const WordExpr&,
WordDomainType)` and overloads of `bit`, `reduce_and`, `reduce_or`, and
`reduce_xor` with an explicit trailing `WordDomainType`. The new unary and
binary `overflow` overloads place explicit `WordDomainType` before the optional
`WordSemantics`. Each
Word-backed `BoolExpr::Misc` node stores its policy, so ordinary Boolean
composition preserves it without changing generic `BoolExpr::Misc::post` or
adding a policy to non-Word Boolean nodes.

## Cheap bounded uncertainty measures

For a bounded domain, let `span_bits` be the width-safe ceiling of
`log2(rank_max-rank_min+1)`, with the full width-64 span handled without
overflow. Trace slack is `unknown_size + span_bits`, while the candidate branch
size merit is `min(unknown_size, span_bits)`, clamped to at least one for an
unassigned variable. Compact views retain `unknown_size`. Trace correction is
required; branch-merit adoption remains evidence-gated.

## Focused performance controls

The batching task uses two root-propagation controls, each containing 512
independent constraints and retaining 100 clones per trial. The divmod control
uses 12-bit unsigned `a in [1536,2047]`, `b in [17,31]`,
`q in [48,120]`, and `r in [0,30]`. The variable-SHL control uses 13-bit
unsigned `x in [250,550]`, `amount in [2,3]`, and
`result in [2000,4400]`. Run 20 interleaved Release trials against the exact
parent commit and repeat both controls with compact Word variables.

The numeric-channel control contains 2,048 unsigned and 2,048 signed pairs.
Unsigned Word and Int endpoints are `[1000,2000]`; signed endpoints are
`[-1000,1000]`. Reach fixpoint, fix encoded bit zero to zero on each Word (an
endpoint-neutral bit event), reach fixpoint again, then narrow the Int interval
to `[1100,1900]` or `[-900,900]`. Retain 100 clones and run 20 interleaved
Release trials. Record Word actor executions, synchronization calls,
propagations, and time, followed by the DMA comparison.

The branch-merit microcase is one brancher over 2,048 alternating 16-bit
unsigned bounded variables: `wide in [0,65535]` and
`narrow in [32767,32768]`. Both retain the full cube and therefore have 16
unknown bits. Record the first selected position and repeated root-choice scan
time; use the DMA model for search-tree evidence.

The adaptive-Add arithmetic control has 65 12-bit unsigned bounded states
`x[0..64]`, initially full range, joined by 64 Add constraints whose constants
repeat `1,2,3,4`. Post every
binary Add before restricting the first state to `[100,200]` and the last to
`[260,360]`, then branch on all states by lower ranked split. It has 101
solutions; its checksum is the wrapping sum over solutions of
`sum((i+1)*state[i])`. Controls are the checked-in examples invoked as
`word-symbolic-alu -steps 8 -width 18 -word-domain cube -solutions 0`, the same
with `-word-domain unsigned`, `word-arx-preimage -solutions 0`, and
`word-md5-preimage -steps 2 -unknown 12 -solutions 0`.

## Specialist evidence models

Bounded n-ary Add uses a scatter/gather total-length model. For segment counts
4, 6, and 8, each 12-bit unsigned length lies in `[64,256]`, has its low four
bits fixed to zero, and is ordered nondecreasingly. One native n-ary Add fixes
the total to `160*count`; the maximum possible sum is non-wrapping. Compare
the existing cube actor with the bounded candidate using lower ranked splits,
solution count, and the wrapping sum of `sum((i+1)*length[i])` as checksum.

Bounded `distinct` uses a register-address allocation model with three logical
registers in each of two banks. Each eight-bit unsigned address has its low
four bits zero; bank zero fixes bit six to zero and restricts addresses to
`[0x00,0x3f]`, while bank one fixes bit six to one and uses `[0x40,0x7f]`.
Addresses have no ordering constraint. Compare native Word distinct, pairwise
Word disequality, and compact Word plus channel-equal IntVars with Int
distinct. Scales use two, three, and four logical registers per bank, retaining
the four aligned slots in each bank. Compare solution count and the wrapping
sum of `sum((i+1)*address[i])`.

## Testing

Focused coverage.

- For domain or propagator changes, add focused small-width oracle coverage when
  it tests soundness, aliases, or inverse propagation that scenario tests do
  not characterize. Do not add tests that merely mirror implementation shape.
- Cover applicable width-one, width-64, signed/unsigned, failure, alias,
  cloning, recomputation, and genuine replay paths in the focused Word suite.
  A task need only add the cases relevant to its behavior.
- For branching and public API defects, add one direct regression that fails on
  the reviewed behavior and succeeds after the fix.
- For performance changes, compare Release builds with interleaved trials and
  record solutions, nodes, failures, propagations, and runtime. Include compact
  Word controls whenever shared code or event behavior changes.
- For memory claims, use observed object sizes or scaled allocation/RSS slopes;
  formula-based entity counts are not byte measurements.
- Run the affected Word test group and TestFramework. Run the broader Word
  suite and warning-clean Release compilation when shared variable, view,
  event, branch, or MiniModel infrastructure changes.
