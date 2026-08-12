+++
schema_version = 1
id = "gcd-013"
key = "eliminate-support-enumeration"
area = "gcd"
status = "open"
blocked_by = ["gcd-008", "gcd-009", "gcd-010", "gcd-011", "gcd-012"]
+++
# Eliminate support enumeration from number-theoretic propagators

## Outcome

Every propagator added in the gcd area uses bounds or algebraic reasoning instead of Cartesian support enumeration for propagation and reification status.

## Boundaries

- Audit Gcd/ReGcd, ReDivides, Product/ReProduct, ProductMod/ReProductMod, and ProductModVar/ReProductModVar, including shared helpers.
- Do not change the public relation semantics or add proof-logging artifacts.
- Direct evaluation is allowed only when all variables relevant to the relation are assigned; do not collect or project tuple supports.
- Use the existing integer arithmetic test harness and the focused testing level in the area brief.

## Done when

- [ ] No audited propagator or helper enumerates a Cartesian product of variable domains, allocates tuple-support tables, or filters domains by projected tuple supports.
- [ ] Ordinary propagation is expressed through sound bounds, sign/zero classification, divisibility, congruence, or algebraic rewrites, with conservative behavior where those deductions are inconclusive.
- [ ] Reified entailment and disentailment use sound bounds or algebraic tests and exact evaluation only for fully assigned relations; negative or unfixed cases wait conservatively instead of enumerating supports.
- [ ] Propagation conditions, subscriptions, and costs match the information actually used, including bounds subscriptions where interior-domain events are no longer required.
- [ ] Focused tests exercise GCD, divides, product, fixed-modulus product_mod, and variable-modulus product_mod on domains above the former 200,000-tuple cutoff, relevant reification modes, and fully assigned true and false leaves.
- [ ] The affected integer library, all focused new-propagator tests with multiple iterations, and the broader integer arithmetic suite build and pass.

## Validation

- Search the gcd-area implementation for support-table, Cartesian-enumeration, and tuple-projection machinery and inspect every remaining domain iterator.
- Build the configured integer library and gecode-test executable.
- Run focused Gcd, Divides, Product, ProductMod, and ProductModVar tests with multiple iterations.
- Run the broader Int::Arithmetic tests.
- Run git diff --check and zd check gcd --format json.
