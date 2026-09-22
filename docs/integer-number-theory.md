# Integer number theory constraints

These APIs require `<gecode/int.hh>`. They use bounds and algebraic reasoning;
the `IntPropLevel` argument currently does not select different strengths.
They do not promise bounds or domain consistency.

Inside a `Space` constructor, this model has `g=6`, `p=-216`, and `r=1`:

```cpp
IntVar x(*this,-12,-12), y(*this,18,18);
IntVar g(*this,0,18), p(*this,-300,300), r(*this,0,6);
gcd(*this,x,y,g);
product(*this,IntVarArgs({x,y}),p);
product_mod(*this,IntVarArgs({x,y}),7,r);
```

Call `status()` on the space to run propagation. `product_mod` uses the
Euclidean residue: `-216 = -31*7 + 1`. In contrast, Gecode's `mod` uses a
dividend-signed remainder and would return `-6` for `-216 mod 7`.

To require divisibility, pass a true reification variable:

```cpp
BoolVar yes(*this,1,1);
IntVar divisor(*this,6,6), dividend(*this,-30,30);
divides(*this,divisor,dividend,Reify(yes));
```

This relation means that some integer multiplier exists. Zero divides zero,
but does not divide a nonzero value. GCD is always nonnegative, with
`gcd(0,0)=0`. The exact product of an empty array is one; its modular product
is `1 mod m`, which is zero when `m=1`.

Every reified overload supports equivalence (`RM_EQV`), `b` implying the
relation (`RM_IMP`), and the relation implying `b` (`RM_PMI`). For example,
`product_mod(*this,factors,m,result,Reify(enabled,RM_IMP))` requires the
modular relation only when `enabled=1`.

A fixed integer modulus must lie in `1..Int::Limits::max`; an invalid constant
throws `Int::OutOfLimits` even when an implication is inactive. With an
`IntVar` modulus, positivity is part of the reified proposition. Consequently,
an inactive implication does not constrain that modulus or the result.

## Propagation and cost

For a fixed positive modulus `m`, assigned factors are reduced to a coefficient
`c`. Every result is a multiple of `gcd(c,m)`, even with several unassigned
factors. A zero coefficient fixes the result to zero. Thus `6*x*z mod 12 = 1`
fails without branching, and assigned factors 2 and 3 force a zero residue
modulo 6. With one free factor and an assigned result, a linear congruence
also restricts the factor bounds.

A nonzero dividend bounds the magnitude of every possible divisor, including
when the dividend is not assigned. Negating `divides(0,x)` removes zero from
`x`. For GCD, an assigned nonzero operand bounds the result to its magnitude;
a fixed quotient band can identify its only possible divisor. When both an
operand and the GCD are assigned, endpoint probes check that the remaining
quotient is coprime. For example, `gcd(6,x)=3` with `x=6..12` assigns `x=9`.
The probes inspect at most 64 arithmetic candidates per endpoint and retain
the original bound if that budget is exhausted. They use the interval hull,
so interior holes do not change a bounds-subscribed actor's fixpoint.

Exact-product posting sorts equal views together. The grouping survives
cloning, and subsequent propagation groups adjacent occurrences in linear
time. Assigned units are compacted in place while preserving that order.
Fixed- and variable-modulus products use prefix/suffix intervals to
compute all cofactors in linear time per filtering pass, with linear temporary
storage. The temporary intervals are rebuilt after bounds change. Repeated
factors in exact products additionally require integer-root bounds; their
arithmetic work is at most proportional to the factor count times the number
of bits in the integer limit. Local closure can require several passes,
especially across sparse domain endpoints.

Modular interval reasoning uses exact representable endpoints. If an endpoint
overflows, that interval deduction is skipped; saturation is not valid for
modular arithmetic. Assigned modular products remain safe because each
multiplication is reduced before the next factor. Cheap coefficient and
zero checks precede allocation and cofactor filtering. Fixed-modulus actors
fold assigned occurrences into a persistent residue coefficient and remove
them from the active view array. They also retain the exact coefficient while
it fits in the integer limits, for interval reasoning; an unavailable exact
coefficient never replaces the modular one. Later scans and clones visit only
the remaining occurrences. Variable-modulus actors remove assigned ones, but
retain other constants until the modulus is fixed. Both forms exclude zero
endpoints from factors when the result is strictly positive, including factors
with negative domains.

These are deliberately incomplete rules. They do not enumerate all divisors
or all supported residues. Undecided or negated reified constraints generally
perform less filtering than asserted relations. The variable-modulus interval
stage handles nonnegative factors in a single quotient band; fixing the
modulus enables the fixed-modulus rules. Bounds subscriptions can miss
opportunities caused solely by interior domain removal in the propagators
that declare weak monotonicity. Fully assigned tuples are still checked
exactly. Faster propagation passes alone do not establish faster solving;
compare search and end-to-end time on representative models.
