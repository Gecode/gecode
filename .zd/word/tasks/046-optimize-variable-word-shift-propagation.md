+++
schema_version = 1
id = "word-046"
key = "optimize-variable-shift"
area = "word"
status = "open"
blocked_by = ["word-045"]
+++
# Optimize variable Word shift propagation

## Outcome

Variable shift actors reduce repeated cube-hull and WordView publication work while preserving their bounded shift-class propagation contract.

## Boundaries

- Do not add variable rotations, a barrel network, bit channeling, or a new propagation claim.
- Keep existing APIs, fixed-amount rewrites, alias proxies, actor lifecycle, and O(width) bounded-class semantics.
- Keep benchmark artifacts outside the repository.

## Done when

- [ ] The variable shift local fixpoint reuses or closes over local state and publishes only genuinely changed views where measurement supports the change.
- [ ] Normal Structure VariableShift and lifecycle tests preserve assigned semantics, partial soundness, aliases, width boundaries, cloning, recomputation, and subsumption.
- [ ] Focused Release width-scaling and decoder-search benchmarks preserve exact solutions and document runtime, propagation, search, and memory effects.

## Validation

- Build the focused Release Word library and gecode-test and run VariableShift, Structure, and Word TestFramework filters.
- Run temporary width 8/16/32/64 assigned, partial, and decoder-search benchmarks with a sampling profile.
- Run git diff --check.
