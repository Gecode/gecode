# Worst-case error of an FP8 multiply-accumulate

Machine-learning accelerators run kernels in low-precision formats such as BF16
and FP8 to save memory, bandwidth and energy. Deciding whether a kernel may run
in such a format needs its worst-case error, not the average error that sampling
gives: a rare input with a large error can break a computation that is accurate
almost everywhere.

[`word-float-error`](../examples/word-float-error.cpp) answers this question for
the multiply-accumulate `a*b+c`, the step that dot products and matrix
multiplications repeat. It computes `a*b+c` in FP8 E4M3FN and in FP32, finds the
inputs on which the two deviate most, and proves with branch-and-bound search
that no input deviates more.

## Build and run

Word variables are on by default; enable the examples in a CMake build:

```sh
cmake -S . -B build -DGECODE_ENABLE_EXAMPLES=ON
cmake --build build --target word-float-error
build/bin/word-float-error
build/bin/word-float-error -computation fused
```

The Autoconf build lists `examples/word-float-error` among the Word examples.

`-computation separate` (the default) rounds the product to E4M3FN and then
rounds the sum, as hardware without a fused multiply-add does.
`-computation fused` rounds `a*b+c` once with `fp_fma`. The example prints the
best solution found, and the driver summary follows it:

```
WordFloatError
	inputs    = a 0.28125 (0x29), b 1.875 (0x3f), c -0.5 (0xb0)
	product   = 0.5 (E4M3FN 0x30)
	E4M3FN    = 0 (0x00)
	FP32      = 0.02734375 (0x3ce00000)
	error     = 14 E4M3FN ULPs
	check     = plain double arithmetic agrees
```

The fused computation prints no product line. The last line comes from an
independent check: plain double arithmetic recomputes `a*b+c` for the reported
inputs, which is exact here, rounds it to E4M3FN by scaling with the local
spacing, and must obtain the same results and error. Otherwise the line starts
with `ERROR:`.

## The model

The inputs are E4M3FN values with magnitudes in `[1/4, 15/4]`, encodings `0x28`
to `0x47`. This activation-like range spans four binades with eight values each.
The inputs `a` and `b` are positive, while `c` takes either sign, so `a` and `b`
take 32 values and `c` takes 64. Each input is a `WordVar` of domain type
`WDT_E4M3FN`. The model posts `fp_compare` of `a`, `b` and the magnitude of `c`
(from `fp_abs`) against the bounds, which `float_constant` creates from the
values 1/4 and 15/4. A tiny format such as E4M3FN gives a word a value set by
default, so these comparisons restrict each input exactly, and `c` keeps the
hole between its negative and positive values. Every float operation
reads the format of its operands from their domain types. The search splits
each input at the median of its remaining values in E4M3FN total order, with
`WORD_VAL_SPLIT_MIN`.

Two symmetries reduce the search. Multiplication commutes, so `a <= b`. On
positive encodings the unsigned order of the bits is the value order, so the
model posts this as `rel` with `WRT_ULQ` on the two words. Rounding is to
nearest, ties to even, which commutes with negation: negating `a` and `c`
negates every result, so positive `a` and `b` lose no generality. The model
passes no `FloatWordPolicy`, so every operation uses the default policy: IEEE
754 rounding to nearest, ties to even, with gradual subnormals.

The reference converts the inputs exactly to FP32 with `fp_convert` and then
posts `fp_multiply` followed by `fp_add`. A product of two E4M3FN values has at
most eight significant bits, so the FP32 product is exact. On this range every
product is a multiple of `2^-10`, every `c` a multiple of `2^-5`, and
`|a*b+c| < 18`, so the sum has at most 15 significant bits and is exact as well.
The FP32 result `r32` is therefore the exact value of `a*b+c`. Multiplying and
then adding gives the same result as an FP32 `fp_fma` and propagates much faster.

## The error

The objective is

```
error = |r8 - r32| / ulp_E4M3FN(r32)
```

the distance between the E4M3FN result `r8` and `r32`, in units of the E4M3FN
spacing at `r32`. The model posts `ulp_error` with E4M3FN as the reference format
and `r32` as the reference value, and maximizes its fixed-point digits
lexicographically. The documentation of `ulp_error` in
[`gecode/float-word.hh`](../gecode/float-word.hh) defines the digits and the
spacing. The model does not use `ulp_steps`, which needs both
values on the E4M3FN grid: `r32` mostly lies between E4M3FN values.

## Proved results

A completed run proves these maxima for the stated range, with RNE and gradual
subnormals:

| Computation | Worst error | Witness `(a,b,c)` | E4M3FN | FP32 | Nodes | Time |
| --- | --- | --- | --- | --- | --- | --- |
| separate | 14 ULPs | `(9/32, 15/8, -1/2)` | `0` | `7/256` | 30,793 | about 2 s |
| fused | 1/2 ULP | `(1/4, 1/4, -2)` | `-2` | `-31/16` | 60,439 | about 2.5 s |

Times are for a Release build. With separate rounding, the product `135/256`
rounds to `1/2` and cancels against `c`, so nothing of the exact result remains.
The fused computation stays within half an ULP, the bound for correct rounding.
Other inputs tie with both witnesses. An independent exact-rational evaluation
of all inputs, without the symmetries, finds the same maxima.

## Proof scope

Propagation prunes little before all three inputs are assigned, so the proof is
in effect an enumeration of the 33,792 input combinations that the symmetries
leave, cut short where the branch-and-bound bound rules a subtree out. It covers
only the stated input range, the selected computation and the default policy.

The driver's `-node`, `-fail` and `-time` limits stop the search early. A stopped
run reports `Search engine stopped...` in the summary, and its last solution is
only a lower bound on the worst error. For example, `-node 1000` stops at an
error of 1/2 ULP.

## Changing the model

- **Range:** the constants `min_magnitude` and `max_magnitude` set the
  magnitudes and must be E4M3FN values. Each further binade adds eight values to
  every input. A wider range needs a new exactness argument for the FP32
  reference, and its results must stay below the E4M3FN overflow bound 448,
  which the plain check assumes.
- **Computation:** change `post_e4m3fn_result` to try another rounding order,
  and the plain check in `evaluate_plain` to match it. A two-term dot product
  `a*b+c*d` adds a fourth input and multiplies the search by its domain size.
- **Format:** BF16 has 128 values per binade, and the model searches it more
  slowly than E4M3FN, so even one binade takes many minutes.
- **Rounding:** pass a `FloatWordPolicy` with the rounding direction after the
  operands of each operation, as in `fp_add(home, a, b, r, policy)`.
  The sign symmetry holds only for rounding to nearest. Directed rounding needs
  inputs `a` and `b` of both signs.
