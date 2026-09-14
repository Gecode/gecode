# Bounded propagation through packed fields

This investigation tested unsigned concatenation and fixed extraction. It used
a temporary Release prototype and removed the prototype and raw samples after
collection. The task normally calls for an indexed `.zdev` report; the worker
boundary prohibited `.zdev` edits, so this task-owned benchmark note and the
link from `benchmarks/word/README.md` retain the result instead.

## Candidate rules

For unsigned `result = concat(high, low)`, with `s = 2^low_width`, the guarded
non-wrapping hull is
`[high_min*s + low_min, high_max*s + low_max]`. In the reverse direction,
`high` is narrowed by `[floor(result_min/s), floor(result_max/s)]`. The low
field is narrowed by the endpoint residues only when both result endpoints are
in the same `s`-sized block; otherwise the projection is non-convex and the
candidate leaves it to the existing cube actor.

For `extract(source, first, width)`, unsigned high-field extraction
(`first + width == source_width`) is monotone and uses the shifted endpoint
hull. A non-high fixed extraction uses the same rule only when the source
interval stays within one modulo period `2^(first+width)`. It falls back to the
cube actor when the interval crosses a period. Both rules decline compact and
signed domains. This preserves the existing signed and modular interpretation
instead of silently replacing a non-convex image by an unsafe interval.

These guards fit the existing split between cube propagation in
`gecode/word/structure/{concat,fixed}.hpp` and typed bounded dispatch in
`gecode/word/structure.cpp`. They also follow the useful part of the Alt-Ergo
2025 result: bit-vector propagation benefits from communicating bit-pattern
and interval domains, while extraction can naturally produce unions of
intervals. Gecode's single-interval Word representation makes the explicit
period-crossing fallback essential. The local source is CPKB paper
`2025-ait-el-hara-constraint-propagation-for-bit-vectors-in-alt-er`, especially
its interval-domain discussion and concat/extract integer encoding.

## Oracle

An exhaustive independent Python oracle checked 3,174,275 cases. Concatenation
covered high and low widths one through three, every interval for both fields,
and every result interval. Extraction covered source widths one through six,
every legal fixed field, and every source interval. Every retained candidate
endpoint contained every concrete support. The run also exercised width one,
boundary-aligned endpoints, same-block low projections, and 26,572
period-crossing extraction cases. Explicit compact, signed, and wrapping
controls were declined rather than narrowed.

## Packed-field model

The temporary packet-header model used twelve independent 12-bit packed
fields. Each field was a concatenation of an 8-bit high field and a fixed-zero
4-bit low field, and its high field was also obtained by fixed extraction.
Packed values were bounded to `[0x120,0x18f]`, while a separate numeric header
restriction bounded each high field to `[0x17,0x19]`. Cube propagation cannot
remove `0x19`; the guarded high-field endpoint rule narrows it to
`[0x17,0x18]`. The public projection was the twelve packed values.

Three formulations were run in baseline/candidate/control order for 20
interleaved Release trials. The candidate injected exactly the guarded endpoint
deduction. The control channelled all fields to IntVars and posted
`packed = 16*high + low`. For each public projection, the checksum term was
`sum((i+1)*packed[i] for i in 0..11)`; the reported checksum is the unsigned
64-bit wrapping sum of that term across all solutions. An independent evaluator
enumerated the two supported values per field, giving exactly 4,096 projections
and checksum 120,127,488. All formulations matched both independent invariants.

| formulation | solutions | nodes | failures | propagations | median time (ms) | range (ms) |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| cube baseline | 4,096 | 16,381 | 4,095 | 50,827 | 6.779 | 6.672–7.002 |
| guarded candidate | 4,096 | 8,191 | 0 | 36,450 | 3.963 | 3.891–4.131 |
| Int channel control | 4,096 | 8,191 | 0 | 72,924 | 6.512 | 6.404–6.662 |

The guarded rule halved nodes, removed all failures, reduced propagations by
28%, and reduced median runtime by 42% in the packed model. The numeric channel
obtained the same search tree but doubled candidate propagations and lost the
runtime gain.

### Period-crossing fallback control

A second temporary Release control created 4,096 unsigned bounded 8-bit
sources in `[0x0e,0x12]` and extracted their low four bits. This interval
crosses the 16-value modulo period. The candidate dispatch evaluated the guard
and posted only the existing cube actor. Over 20 interleaved trials, cube and
candidate both retained 4,096 propagators and performed 4,096 propagations.
Median construction plus root-propagation time was 0.512 ms for cube and
0.512 ms for candidate (0.000511813 s versus 0.000512271 s, about 0.1%
overhead); observed ranges were 0.494–0.996 ms and 0.489–0.568 ms. Thus the
unsupported regime adds no actor or propagation work, and its guard cost was
below the noise of this capped control.

## Recommendation

Implement one narrow production follow-up: typed unsigned-bounded concat plus
high/fixed extraction endpoint propagation at the existing structural dispatch
seams. Keep the cube actor posted for bit closure, subscribe the bounded actor
to bound events, and use the guards above. Do not add interval unions, signed
rules, or modulo-crossing low projections. Production validation should reuse
the exhaustive small-width support oracle and add this packed-header regression;
compact and signed calls must continue selecting only the cube actor.
