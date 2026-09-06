# High/low multiplication investigation

## Recommendation

Defer a production high/low multiplication constraint. The measured gap is
real: a single actor with joint access to both halves can avoid a large Boolean
network and can propagate the complete small relation much more effectively.
The temporary candidate, however, obtains that result by exhaustive support
enumeration through width 8 and only evaluates fixed operands at larger widths.
That is useful experimental evidence, not a scalable propagator design.

Reject the Boolean schoolbook network as the intended user-facing
decomposition. It is a faithful expression using existing Boolean primitives,
but its construction, propagation count, and clone footprint grow too quickly.
Also reject expanding this task into fused multiply-add, weighted sums, signed
variants, wider variables, or instruction-set modeling. A later implementation
should be separately scoped around scalable unsigned forward and inverse
reasoning for the exact four-variable relation.

## Relation and experimental controls

For unsigned words of width `w`, the experiment used

```
x*y = high*2^w + low
low  = (x*y) mod 2^w
high = floor((x*y)/2^w)
```

The candidate was one temporary four-view actor exposing same-width `high` and
`low` variables. The control channelled the four words to Boolean variables and
formed the full `2w`-bit product from `w` shifted partial-product rows and ripple
carry adders. Both models used the same public branching order over
`[x,y,high,low]`. No `2w` WordVar or arbitrary-precision Gecode variable was
introduced.

The prototype computed the 64-by-64 product with four guarded 32-by-32 limb
products, including explicit middle and high carries. Thus it did not rely on
a compiler-specific 128-bit integer. Expected values came independently from
Python integers.

The source context was Lagoon and Metodi, *Deriving Optimal
Multiplication-by-Constant Circuits With A SAT-based Constraint Engine* (2020),
using the maintained CPKB source page and extracted full text. Their model
selects a topology of add/subtract/left-shift nodes for a known constant and can
prove a node bound infeasible. A constraint that recovers factors or exposes
the halves of a general product solves a different problem; it does not by
itself synthesize the selectable circuit topology used in SCM or MCM.

## Semantic checks

Exact projection sets from the candidate and Boolean control were compared
with an independent Python oracle. Exhaustive widths 1 through 8 covered
87,380 operand pairs. A further 35 searches fixed `high`, fixed `low`, fixed
both halves, or used actual `x=y` and `high=low` aliases. All three projections
were identical. The checks included zero, one, maximum values, powers of two,
and both satisfiable and unsatisfiable observations.

The width-64 oracle matrix retained the expected status explicitly:

| Case | Expected `(high, low)` | Status |
| --- | --- | --- |
| `0 * max` | `(0, 0)` | SAT |
| `1 * max` | `(0, max)` | SAT |
| `max * max` | `(max-1, 1)` | SAT |
| `2^63 * 2` | `(1, 0)` | SAT |
| `2^63 * 2^63` | `(2^62, 0)` | SAT |
| `(2^32-1)^2` | `(0, 18446744065119617025)` | SAT |
| `2^32 * (2^32+1)` | `(1, 2^32)` | SAT |
| `1 * 1` | `(0, 1)` | SAT |
| fixed `max * max`, observed `(max,1)` | impossible | UNSAT |

This covers satisfiable `high=0`, `high=max-1`, `low=0`, `low=1`, and
`low=max`; `high=max` is impossible for two unsigned width-64 operands and was
checked as UNSAT rather than omitted. A constrained prefix case used four
values for `x` and eight for `y`, immediately above `2^32`, and enumerated the
same 32 projections in both formulations.

## Release measurements

The prototype was compiled with Apple Clang 21 using `-O3 -DNDEBUG` against the
Release build at revision `1e9822acc06dd98849ea3a2718d71ab5bd0174e5` on
arm64. Seven trials rotated case and formulation order. Each subprocess had a
12 s wall cap, 10 s CPU cap, two million DFS-node cap, one million solution
cap, and a 2 GiB resident cap checked after each retained clone and each 1,024
solutions. No timeout, error, or cap event was interpreted as UNSAT; the
accepted trials had none. One preliminary width-8 full-forward Boolean timing
run exceeded the wall cap. It was retained only as a timeout observation; the
capped repeated forward cell therefore uses width 7. The exact width-8
semantic enumeration completed separately and is not presented as timing
evidence.

Times below are medians in microseconds. The ranges in parentheses are the
observed search-time minima and maxima. Stable search counters were identical
across repetitions.

| Case | Form | Root | Search | Nodes / failures / propagations |
| --- | --- | ---: | ---: | ---: |
| width-7 forward, 16,384 solutions | candidate | 81 | 18,207 (16,849–18,616) | 32,767 / 0 / 35,779 |
|  | Boolean | 4 | 1,273,484 (1,152,685–1,400,668) | 217,571 / 92,402 / 15,955,051 |
| width-8 fixed high, 187 solutions | candidate | 213 | 777 (708–781) | 373 / 0 / 413 |
|  | Boolean | 19 | 181,955 (124,792–215,414) | 28,103 / 13,865 / 2,835,310 |
| width-8 fixed low, 128 solutions | candidate | 263 | 559 (503–566) | 255 / 0 / 276 |
|  | Boolean | 26 | 1,820 (1,629–2,204) | 255 / 0 / 37,776 |
| width-8 joint observation, 4 solutions | candidate | 244 | 117 (106–118) | 7 / 0 / 6 |
|  | Boolean | 22 | 1,481 (1,339–1,833) | 235 / 114 / 30,455 |
| width-8 `x=y`, 256 solutions | candidate | 311 | 13,941 (13,331–14,171) | 1,009 / 249 / 1,105 |
|  | Boolean | 4 | 5,009 (4,531–5,700) | 1,009 / 249 / 96,409 |
| width-64 prefix, 32 solutions | candidate | 3 | 29 (26–33) | 63 / 0 / 67 |
|  | Boolean | 39 | 953 (859–967) | 63 / 0 / 23,694 |

Construction medians for fixed width-64 cases were 51 us for the candidate
and 841–886 us for the Boolean control. Both became solved at the root, so
their actor counts after propagation were zero. On the unconstrained width-64
root, the candidate retained one actor and the Boolean control retained 28,799;
construction medians were 51 us and 1,780 us respectively.

Clone memory was measured from the process resident set while clones remained
alive, after allocator warm-up in the constructed root. For eight stable clones
of the unconstrained width-64 root, the median retained RSS deltas were 49,152
bytes for the candidate and 25,853,952 bytes for the Boolean control. At small
widths, 200 retained clones used 524,288 bytes for the candidate and between
8,372,224 and 13,303,808 bytes for the control. Actor count is reported only as
a secondary explanation of these measured footprints.

The candidate's one unfavorable search result matters: on the width-8 square
alias, the Boolean network was about 2.8 times faster despite performing many
more propagations. Together with the candidate's deliberately non-scalable
enumeration, this prevents a production recommendation based solely on the
large wins in the other rows.

## Retained scope

The prototypes, binaries, oracle scripts, raw samples, and temporary build
commands were used only for this investigation and are not part of the retained
source tree. This report and its README link are the complete task delta.
