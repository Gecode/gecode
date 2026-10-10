# MiniZinc native propagator audit

The audit asks whether a standard MiniZinc expression already reaches a native Gecode propagator, including compiler rewrites and solver-library definitions. When a standard solver hook can select a missing actor, the implementation upgrades that hook. Additional public `gecode_*` predicates cover the remaining native capabilities. A different decomposition alone does not establish that a new name is needed.

Verification uses MiniZinc 2.10.1 (build 33348285743) and this checkout's FlatZinc solver. Standard-library filenames below refer to that installation. A local MiniZinc 2.9.3 compiler checkout supplied additional source evidence; the installed compiler's generated FlatZinc was checked independently. Native declarations and posting code are in [int.hh](../gecode/int.hh), [set.hh](../gecode/set.hh), [float.hh](../gecode/float.hh), and their implementation directories.

[gecode.mzn](../gecode/flatzinc/mznlib/gecode.mzn) supplies the public overloads and semantic relation/task enums. [gecode_fzn.mzn](../gecode/flatzinc/mznlib/gecode_fzn.mzn) contains typed primitive declarations, including internal leaves used by standard translations. Public predicates that already have a single typed signature are documented at their declaration there, without a redundant forwarding wrapper. [registry.cpp](../gecode/flatzinc/registry.cpp) posts those primitives. Existing registry aliases are reused rather than registered twice. Both build systems install the entire solver-library directory.

## Automatic standard routes

These capabilities receive no additional public modeling names. The solver library selects the native actor from standard syntax, preserving the standard relation and its boundary behavior.

| Standard expression or global | Solver route |
| --- | --- |
| Optional all-different, single-exception all-different | `fzn_all_different_int_opt.mzn`, `fzn_alldifferent_except.mzn`, and `fzn_alldifferent_except_0.mzn` use native optional/single-exception distinct. Multiple exceptions and reified cases retain suitable decomposition. |
| Scalar count comparisons | Four `fzn_count_*` inequality overrides call the native count relation. MiniZinc rewrites comparisons on scalar `count` and corresponding equality comprehensions to these hooks. Standard `count_leq` means `n <= count`, so its native relation is greater-or-equal. |
| Binary conditionals, all four variable types | Eight `fzn_if_then_else[_var]_{int,bool,float,set}.mzn` overrides select native ITE for two branches ending in a known true condition. Longer and partial condition lists retain standard guarded-equality behavior. |
| Integer/Boolean array `!=` | `solver-native-overloads.mzn` preserves standard matching-index checks, then uses native array disequality in positive root contexts. Other contexts retain existential scalar disequality. Integer enums are converted explicitly. |
| `not all_equal(x)` | The integer reification hook and Boolean overload select native not-all-equal when the result is fixed false. General reification keeps the standard linear-size comparison translation. |
| Boolean `nvalue` equality | A typed Boolean overload selects native Boolean nvalues. Integer equality already had a native route. |
| Boolean `regular` | Typed standard overloads normalize Boolean transitions and select the native Boolean DFA. Guarded/reified expressions retain the standard integer translation. |
| `regular_nfa`, integer and Boolean words | Existing `fzn_regular_nfa` hooks select native `DFA::nfa` construction without determinization. Integer-size and range alphabets retain their standard validation, and empty words test initial-state acceptance. Reification remains unsupported as in the standard library. |
| Integer `product` | A more specific one-dimensional enum/integer index overload selects native nary product in positive root contexts, including generator syntax and the standard optional-value translation. Other contexts and direct multidimensional arrays retain recursive binary multiplication. Parameter products retain the builtin route. Specialized native equivalence/implication actors are exposed separately. |
| Optional cumulative, optional strict disjunctive | Native flexible optional tasks support variable durations, and optional cumulative now accepts variable capacity. Standard cumulative filters zero-duration tasks; strict disjunctive retains zero-duration ordering. |
| Array intersection of sets | `fzn_array_set_intersect.mzn` selects native nary intersection for nonempty arrays. Empty standard intersection remains false. |
| Two-dimensional set lookup | `redefinitions-2.5.2.mzn` selects native row/column set element, shifting standard indexes. Empty lookup remains undefined. |
| Set `min`/`max` | The documented `redefinitions-2.1.1.mzn` function hooks select native extrema. Partial contexts guard native implication with nonemptiness, without an extra surrogate set. Integer and enum sets preserve standard undefinedness. |
| Float array `min`/`max` | `redefinitions-2.0.mzn` selects native nary extrema rather than a binary chain. Empty expressions retain their standard errors. |
| Float power | The standard `float_pow` hook selects native fixed nonnegative integer exponent or fixed positive base. General varying-base/varying-exponent power was unsupported before and remains unsupported, with a clear compile error. |
| Scalar float implication | Existing scalar and normalized linear implication hooks select native scalar implication. General linear forms retain equivalence plus implication. |
| Set equality/disequality/subset/superset implication | Standard half-reification hooks select native implication. |
| Negated integer/Boolean tables | Already exposed: a positive reified table with result false selects native negative actors in the compact, compressed, and sparse posting implementations. No negative-table names or posters are added. |

The additional typed standard overloads are included automatically from `redefinitions.mzn`. Models do not need to include `gecode.mzn` to receive these routes.

## Remaining public families

The following names cover capabilities without a matching standard automatic route. A family may also accept cases already exposed; those cases reuse existing routes rather than adding another actor.

| Public names | Why the remaining native variant is needed |
| --- | --- |
| `gecode_inter_distance` | Native pairwise minimum absolute distance, including variable distance and basic/advanced algorithms; pairwise arithmetic has no matching global rewrite. |
| `gecode_all_different` | Offset form only. Standard arithmetic plus all-different introduces auxiliary variables instead of native offset views. The registry alias already existed. |
| `gecode_channel` | Native one-hot Boolean-array/index and direct Boolean/float channels. `bool2float` has an existing function body through integer conversion, without a solver override hook. |
| `gecode_sort_permutation` | Native input-to-output permutation permits arbitrary tie order. Standard `arg_sort` returns a stable inverse mapping. |
| `gecode_count` | Set-target inequalities and position-dependent target arrays have no compiler comparison rewrite. Scalar count is automatic; set equality/disequality reuse among. |
| `gecode_global_cardinality_sets` | Closed cardinality with arbitrary allowed count sets, including holes; standard GCC exposes variable counts or interval bounds instead. |
| `gecode_nvalues` | Native inequality propagation has no standard nvalue comparison rewrite. Equality/disequality reuse the existing route or the native Boolean equality actor. |
| `gecode_gcd`, `gecode_divides` | Native signed-operand GCD and zero-aware divisibility have no standard variable hooks. Their typed equivalence/implication leaves support inferred reification of the public predicates. |
| `gecode_product_reif`, `gecode_product_imp` | Specialized native nary product equivalence/implication differs from an equality against a separately computed standard product. Ordinary product already has an automatic route. |
| `gecode_product_mod` | Native canonical Euclidean residue of a nary product, including a variable modulus. There is no matching standard global; truncating mod of a recursively computed product differs for negative products. Known positive moduli select the fixed actor. Modulus positivity is part of the reified proposition, so constant folding never turns a false relation into a precondition error. |
| `gecode_minimum_distance` | Native table-distance global, optional pair requirements, and single/decomposed actors lack a standard route. Site values use the matrix's indexes; repeated sites are allowed. |
| `gecode_divmod` | Joint truncating division and remainder. Standard `int_div` and `int_mod` are separate hooks. |
| `gecode_nroot` | Native integer roots and float roots of general positive degree. Float square roots reuse the standard route. |
| `gecode_path` | Hamiltonian successor representation, terminal sentinel, and total/per-edge costs. Standard graph path uses selected graph nodes and edges. |
| `gecode_order` | Native two-task ordering Boolean has no joint standard ordering hook. |
| `gecode_unary_typed`, `gecode_cumulative_typed` | Native fixed-start/fixed-end task types, including optional tasks, are absent from standard scheduling globals. |
| `gecode_nooverlap` | Native optional rectangles with fixed or variable dimensions and explicit ends. Standard diffn covers mandatory rectangles. |
| `gecode_convex`, `gecode_set_sequence` | Native convex hull and ordered set sequence/sequence union lack matching standard globals. One-set convexity and sequence aliases already existed in the registry. |
| `gecode_set_atmost_one` | Native equal fixed cardinality plus pairwise intersection size at most one. Standard at_most1 does not carry this cardinality parameter. |
| `gecode_set_channel_sorted` | Native set/strictly-increasing-enumeration channel lacks a standard global. |
| `gecode_set_operation` | Combined operation/relation propagation, including native intersection/subset and union/superset actors. Ordinary operation equality reuses standard set operations. |
| `gecode_array_int_set_operation`, `gecode_array_int_set_element` | Singleton-view array operations with an initial set, and selected intersection/disjoint union. Selected union already has a native range route and reuses it. |
| `gecode_min_reif`, `gecode_max_reif` | Specialized native set-extremum equivalence actors are not selected by the standard function hooks. A false result selects native NotMin/NotMax; separate negated names are unnecessary. |
| `gecode_set_singleton_rel`, `_reif`, `_imp` | Native singleton views for relations beyond membership, without an intermediate variable set. Superset/membership uses the standard route. |
| `gecode_rel_imp` | Set disjointness, full-universe complement, and Gecode characteristic-function ordering implication. Equality/disequality/subset/superset reuse standard hooks. |
| `gecode_log` | Arbitrary fixed-base logarithm with variable argument. Standard two-argument log is parameter-only; bases 2 and 10 reuse existing native routes. |
| `gecode_arg_min`, `gecode_arg_max` | Native arbitrary tie choice has different solutions from standard smallest-index tie breaking. Standard tie breaking is reused when requested. |

Gecode set ordering compares characteristic functions starting at the smallest element. MiniZinc documents sorted-list lexicographic ordering. The inherited Gecode standard ordering posters already use the native order; this change does not add automatic ordering implication hooks or modify those existing posters. The explicit Gecode relation enum documents its own order.

## Other candidates excluded

| Candidate | Existing translation or native implementation |
| --- | --- |
| Integer/float squares | Integer power of degree two and multiplication of a variable by itself select native square propagation. |
| Variable-right-hand-side float linear constraints | Native posting appends the right-hand-side variable with coefficient -1, matching existing arithmetic normalization. |
| Scalar relations and scalar loops over arrays | Existing primitive posters already reach the native relations; the array helpers merely repeat them. |
| Mandatory flexible scheduling and mandatory rectangles | Existing cumulative/disjunctive and diffn routes already reach their actors. |
| Knapsack-augmented cumulative overload checking | The native advanced algorithm is selected through the existing cumulative routes and new algorithm annotations. No additional constraint name is needed. |
| Integer/set precedence chains | Native posting repeats the existing adjacent-pair precedence propagators. |
| Reified set cardinality | Native posting combines cardinality and an existing reified integer relation. |
| Set-wide integer relations | Native posting uses extrema plus scalar relations, nonempty cardinality plus nonmembership, or the singleton relation. Those component routes are reachable. |
| Multidimensional bin packing | The helper decomposes into packing constraints and clique all-different constraints; no additional actor is missing. |
| Ordinary set complement equality | Full-universe set difference selects the native complement-view equality actor. |

## Boundaries and verification

Optional distinct preserves absent values. If native distinct cannot allocate temporary values outside a full input domain, the registry uses guarded pairwise relations. Irrelevant out-of-range exceptions are eliminated before FlatZinc integer-limit checks. Array shape/index and semantic enum-code checks cover the new bindings; existing unsharing is retained where native actors reject repeated unassigned variables.

Compiler checks cover every remaining typed leaf and public overloads, inferred reification, shifted indexes, and consistency annotations. Standard-call checks compare solution sets/counts against installed MiniZinc/Gecode and exercise root, reified, implication, negated, empty, and enum cases. Generated FlatZinc confirms automatic native dispatch without an explicit Gecode library include. These checks establish semantics and routing, not performance.

Fixed-base float testing exposed a rounded scalar logarithm and an invalid shared-variable shortcut in [exp-log.hpp](../gecode/float/transcendental/exp-log.hpp). The implementation uses interval logarithms, handles base-one power, and retains propagation after its own updates. Both build systems enable rounding-aware float compilation. [Float regressions](../test/float/transcendental.cpp) cover exact powers, decreasing bases, base one, and shared variables.

Arithmetic, minimum-distance, and NFA bindings were checked with 35 exhaustive/routing cases, including signed and zero inputs, false guards, invalid variable moduli, shifted matrices, nondeterministic words, and parameter/enum products.

Propagation algorithms are selected with annotations, never predicate arguments. `gecode_basic_propagation`, `gecode_advanced_propagation`, and `gecode_full_propagation` map to `IPL_BASIC`, `IPL_ADVANCED`, and their combination. They combine with existing domain/bounds/value propagation annotations. `gecode_decomposed_propagation` selects separate pair actors for minimum distance; the default is a single actor with the same filtering. Unsupported algorithm choices retain the native constraint's normal behavior. [Registry regressions](../test/flatzinc/native-registry.cpp) check semantic boundaries and demonstrate different native inter-distance filtering under basic and advanced algorithms.

```minizinc
include "gecode.mzn";
constraint gecode_inter_distance(x, distance) :: gecode_basic_propagation;
constraint gecode_inter_distance(y, distance)
  :: bounds_propagation :: gecode_advanced_propagation;
constraint gecode_count(x, targets, GecodeLe, limit) :: domain_propagation;
```

To run the retained native regressions from an existing CMake build:

```sh
cmake --build build/minizinc-registry --target fzn-gecode gecode-test -j6
build/minizinc-registry/bin/gecode-test -test '^FlatZinc::' -iter 1 -threads 6
build/minizinc-registry/bin/gecode-test -test '^Float::Transcendental::' -iter 1 -threads 6
```

For a MiniZinc routing check, select this checkout as the solver library with
`-G`; an include directory alone does not replace the installed solver overrides:

```sh
minizinc --compile --solver gecode -G "$PWD/gecode/flatzinc/mznlib" \
  --output-fzn-to-file /tmp/native-check.fzn model.mzn
build/minizinc-registry/bin/fzn-gecode -p 6 /tmp/native-check.fzn
```

The compiler and solution-comparison checks described above were run during the
audit; they are not an additional checked-in test suite.
