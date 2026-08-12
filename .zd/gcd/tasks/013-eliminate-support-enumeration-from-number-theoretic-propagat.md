+++
schema_version = 1
id = "gcd-013"
key = "eliminate-support-enumeration"
area = "gcd"
status = "done"
blocked_by = ["gcd-008", "gcd-009", "gcd-010", "gcd-011", "gcd-012"]
+++
# Eliminate support enumeration from number-theoretic propagators

## Outcome

Every propagator added in the gcd area uses bounds or algebraic reasoning instead of Cartesian support enumeration for propagation and reification status.

## Boundaries

- Audit Gcd/ReGcd, ReDivides, Product/ReProduct, ProductMod/ReProductMod, and ProductModVar/ReProductModVar, including shared helpers.
- Gcd/ReGcd and ReDivides must remove their 200,000-tuple thresholds, Cartesian domain scans, support arrays, and projected filtering rather than retaining enumeration for small domains.
- Do not change the public relation semantics or add proof-logging artifacts.
- Direct evaluation of a complete relation is allowed when all relevant variables are assigned. Computing one determined output from assigned inputs is also allowed; do not enumerate combinations of unassigned domains or collect/project tuple supports.
- Use the existing integer arithmetic test harness and the focused testing level in the area brief.

## Done when

- [x] No audited propagator or helper enumerates a Cartesian product of variable domains, allocates tuple-support tables, or filters domains by projected tuple supports.
- [x] Gcd/ReGcd and ReDivides use bounds and number-theoretic identities for propagation and conservative algebraic reification status, with no small-domain enumeration fallback.
- [x] Ordinary propagation is expressed through sound bounds, sign/zero classification, divisibility, congruence, or algebraic rewrites, with conservative behavior where those deductions are inconclusive.
- [x] Reified entailment and disentailment use sound bounds or algebraic tests and exact evaluation only for fully assigned relations; negative or unfixed cases wait conservatively instead of enumerating supports.
- [x] Propagation conditions, subscriptions, and costs match the information actually used, including bounds subscriptions where interior-domain events are no longer required.
- [x] Focused tests exercise GCD, divides, product, fixed-modulus product_mod, and variable-modulus product_mod on domains above the former 200,000-tuple cutoff, relevant reification modes, and fully assigned true and false leaves.
- [x] The affected integer library, all focused new-propagator tests with multiple iterations, and the broader integer arithmetic suite build and pass.

## Validation

- Search the gcd-area implementation for support-table, Cartesian-enumeration, and tuple-projection machinery and inspect every remaining domain iterator.
- Build the configured integer library and gecode-test executable.
- Run focused Gcd, Divides, Product, ProductMod, and ProductModVar tests with multiple iterations.
- Run the broader Int::Arithmetic tests.
- Run git diff --check and zd check gcd --format json.

## Result

Removed all Cartesian/support enumeration and tuple cutoffs from GCD and divides, converted audited actors to bounds/algebraic conservative propagation with weak lifecycle where needed, and added above-cutoff staged leaf coverage for every new family.

Validation:

- Built gecodeint_shared and gecode-test; Gcd, Divides, Product, ProductMod, ProductModVar and large-leaf tests passed for three iterations; broader Int::Arithmetic passed.
- Independent spec/standards verification and static anti-enumeration audit passed; git diff --check and zd check passed.
