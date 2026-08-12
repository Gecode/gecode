+++
schema_version = 1
id = "word-047"
key = "optimize-word-element"
area = "word"
status = "open"
blocked_by = ["word-046"]
+++
# Optimize Word element lookup and clone footprint

## Outcome

Word-array element lookup spends less time scanning candidates and copying actor state on realistic lookup/register-file models without weakening its supported-candidate cube-hull propagation.

## Boundaries

- Begin with the existing mixed Int/Word actor and standard index-domain APIs; do not introduce a custom domain representation, benchmark framework, or broad advisor design without measured need.
- Add a constant Word table overload only if an established Gecode shared-array ownership pattern gives a smaller conventional actor; otherwise record that the API addition was rejected.
- Preserve duplicate entries, aliases, rewrite behavior, subscriptions, and the public variable-array API.

## Done when

- [ ] Candidate scanning and clone/copy work are measured and a focused conventional optimization is retained only when it improves the realistic lookup workload.
- [ ] Normal Word Element tests proportionately cover any retained path, including partial index/result pruning, duplicates, aliases, cloning/recomputation, and subsumption.
- [ ] An exact-baseline Release lookup benchmark and profile record semantic parity, actors, propagations, nodes, failures, runtime, clone footprint, and best-effort RSS.

## Validation

- Build the focused Release Word library and gecode-test and run Word Element, representative Int Element, and Word TestFramework filters.
- Run a temporary lookup/register-file benchmark and before/after sampling profile.
- Run git diff --check and inspect any shared-array lifetime/copy behavior.
