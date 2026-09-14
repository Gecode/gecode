# Lookup relations and variable rotations

This investigation used temporary Release-only actors and removed the source,
binaries, and raw samples after collection. The measurements use commit
`5a2787db6d28804aecbe4385a617e4eb41ff74d7` and do not include the unrelated
FlatZinc worktree change.

## Small constant lookup

The public case is the redistributable first 16 bytes of the AES substitution
box. To keep a common nontrivial domain after an explicit lower-bound control,
the measured table slice uses inputs 1 through 15. The public projection is
`(input, index, output)`, with `input = index` and `output = sbox[input]`.
An independent evaluator enumerated all 15 projections and produced checksum
129,532,072 for `input + 257*index + 65537*output`. Forward enumeration and an
inverse observation fixing `output = 0x7b` matched this exact set in all three
formulations.

The controls were (1) unsigned eight-bit Word input/output channelled to
IntVars with constant Int Element and (2) the current Int-indexed WordVar-array
Element with fixed entries. The temporary candidate was one bit-event Word
actor that scanned the constant table, discarded unsupported rows, and
published the supported bit hull to input and output. Its proposed production
surface is `lookup(Home, WordVar input, const WordValueArgs& table, WordVar
output)`: widths must match, table values must fit that width, the table has
1--256 entries, and `input` is restricted to `[0, table.size())`. Input/output
aliasing is accepted and means retaining only fixed points. `WordValueArgs`
would be a small value-array wrapper owned by the API; no variable entries,
reification, or general tuple engine are part of the proposal.

Ten rotated-and-reversed trials gave the following medians. Times are
microseconds. Actor counts cover only the lookup relation and its required
channels, excluding the common domain restriction and brancher.

| formulation | actors | construct | root | search | nodes | failures | propagations |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| Int constant Element | 3 | 96.4 | 32.9 | 43.1 | 29 | 0 | 137 |
| WordVar-array Element | 3 | 86.3 | 18.2 | 42.0 | 29 | 0 | 112 |
| temporary lookup actor | 1 | 67.9 | 16.3 | 29.6 | 29 | 0 | 49 |

Search ranges were 39.7--47.7, 36.3--47.8, and 27.9--32.9 microseconds,
respectively. The actor does not improve the already exact search tree, but it
cuts relation propagations by 56% against Word Element and reduces median
search time by 30%. With 512 independent inverse constraints, median
construction/root times were 433/169 microseconds for Word Element and 233/94
microseconds for the candidate; relation actors were 1,536 and 512, and root
propagations were 4,608 and 2,560. The Int control used 390/249 microseconds,
1,536 actors, and 5,120 root propagations.

**Decision: implement a bounded follow-up.** Add only constant-table lookup at
the surface and limits above, using a shared immutable table and one support
scan actor. Production work must repeat exhaustive width 1--8 forward and
inverse projection checks, cover fixed-point aliasing, and compare table sizes
4, 16, 64, and 256. Do not add variable tables or a general table engine.

## Variable rotation

The relation is exactly
`result = rotate_left(value, amount mod width)`. The independent evaluator used

```text
((value << (amount % width)) |
 (value >> ((width - amount % width) % width))) & mask
```

and enumerated the complete amount domains at widths 1, 8, and 16. This covers
amounts below, equal to, and above the width. With one fixed result, candidate
and decomposition produced 2, 256, and 65,536 projections. The fixed result
was `1` at width one and `0xa6` at widths 8 and 16. The checksum was the sum of
`value + 257*amount + 65537*result` over those projections; it produced
131,333, 2,793,481,472, and 1,265,944,051,712, respectively. Width one reduces
to equality.

The public decomposition extracts the low `log2(width)` amount bits for these
power-of-two widths, zero-extends the residue, computes the modular complement
with Word subtraction, posts variable shift-left and logical-shift-right, and
ORs their results. The temporary native candidate was one O(width) residue
loop around an exhaustive support-hull scan of the value cube; it had one actor
but deliberately no retained support cache.

| width | formulation | actors | construct (us) | root (us) | search (us) | nodes | failures | propagations |
| ---: | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 1 | equality decomposition | 1 | 72.0 | 4.0 | 14.9 | 3 | 0 | 1 |
| 1 | native candidate | 1 | 67.4 | 6.0 | 15.9 | 3 | 0 | 3 |
| 8 | decomposition | 6 | 89.8 | 23.0 | 171.7 | 557 | 23 | 525 |
| 8 | native candidate | 1 | 68.3 | 12.5 | 716.5 | 511 | 0 | 555 |
| 16 | decomposition | 6 | 87.3 | 23.8 | 22,119.8 | 131,321 | 125 | 1,793 |
| 16 | native candidate | 1 | 68.8 | 2,129.8 | 31,303,000 | 131,071 | 0 | 142,860 |

Width-16 search used three capped rotated trials; all other rows used ten.
Search ranges were 160.5--180.1 versus 645.0--727.3 microseconds at width 8,
and 18.6--25.0 milliseconds versus 19.4--31.5 seconds at width 16. Repeated
root controls confirm that this is intrinsic scan cost: 256 width-eight
constraints took 136 microseconds for the decomposition and 1,231 microseconds
for the candidate; 16 width-sixteen constraints took 33 microseconds and 40.3
milliseconds. The stronger hull removed failures but did not reduce nodes
enough to pay for its work.

**Decision: defer.** Do not add either direct
`rotate_left(Home, WordVar, WordVar, WordVar)` or MiniModel
`rotate_left(WordExpr, WordExpr)` overloads from this prototype. A native API
would need an incremental support representation or a realistic model showing
a search-tree reduction that outweighs the scan; neither is present here.
Right rotation is symmetric by replacing each residue with its modular
negative, so it does not justify a second experiment. If variable rotation is
needed meanwhile, keep the explicit modulo-normalized decomposition. Aliased
value/result cases should use that decomposition; the rejected candidate does
not justify new alias-specific production code.
