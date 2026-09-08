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
