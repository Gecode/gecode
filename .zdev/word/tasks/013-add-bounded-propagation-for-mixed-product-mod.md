+++
schema_version = 1
id = "word-013"
key = "add-bounded-product-mod"
area = "word"
status = "open"
complexity = "advanced"
blocked_by = []
+++
# Add bounded propagation for mixed product_mod

## Outcome

The mixed Word-Int product_mod constraint retains compatible unsigned Word bounds and performs sound product, modulus, result, and inverse range reasoning.

## Context

The public product_mod actor always lowers Word operands and result to WordView. Use the IntVar implementation on `feature/gcd` as prior art, especially `gecode/int/arithmetic/product-mod.hpp` and the focused changes in `f6f821d2d1`, `78ab331449`, and `566232682a`: checked product intervals, quotient-band result and inverse propagation, one-free-factor congruence, variable-modulus rewrites, algebraic reification status, and narrow bounds/value subscriptions without support enumeration. Adopt only deductions sound for two unsigned bounded Word operands and an Int modulus, while preserving the existing Word cube reasoning and bounded-domain synchronization.

Add a reproducible benchmark case: width-nine unsigned bounded x and y in [10,20], width-nine unsigned result in [0,511], modulus fixed to Int value 509, and result = (x*y) mod modulus. Branch on result first by ranked lower split, then x and y. Since x*y is at most 400, the case is non-wrapping and has exactly 121 solutions. Its wrapping 64-bit checksum is the sum of (x<<18) xor (y<<9) xor result over solutions.

## Boundaries

- Use checked or saturating arithmetic for width 64 and Int-limit conversions.
- Preserve cube rules, aliases, assigned cases, positive-modulus validity, and existing EQV, IMP, and PMI reification modes.
- Select the bounded actor only for compatible unsigned Words and a sound numeric regime; preserve compact and mixed fallback.
- Treat the IntVar actor as algorithmic and lifecycle prior art, not as a decomposition: do not channel through IntVars or import unrelated n-ary or signed behavior into the binary Word API.
- Add the exact benchmark case to the existing Word benchmark corpus or focused public benchmark driver; do not rely on vanished temporary evidence or add a general Word-as-Int view.

## Done when

- [ ] The bounded actor propagates result less than modulus, checked product bounds, and sound quotient-derived inverse bounds where applicable.
- [ ] Reified forms activate or rewrite to bounded propagation only when their control mode makes the relation active.
- [ ] Small-width concrete oracles cover variable modulus, aliases, zero and boundary values, width-64 safety, cloning, and replay.
- [ ] The checked-in width-nine case produces 121 solutions and the same checksum on baseline and candidate, with a material search or runtime improvement.

## Validation

- Run focused ProductMod and reification tests plus the concrete oracle.
- Run interleaved Release comparison on the checked-in width-nine case and compact controls with linked binary identities.
- Run TestFramework.
