+++
schema_version = 1
id = "word-006"
key = "branch-basic"
area = "word"
status = "open"
blocked_by = ["word-002"]
+++
# Make word-only models searchable

## Outcome

WordVar branch and assign posting can select least-significant, most-significant, and random unknown bits and commit complementary zero/one alternatives through cloning and recomputation.

## Boundaries

- Treat this as an implementation spike: follow established Gecode patterns, reuse the normal framework and test machinery, and avoid novel infrastructure, special-case test paths, exhaustive hardening, or validation beyond what is proportionate to getting the slice working.
- Leave advanced size, degree, AFC, action, CHB, tracing, and Gist integration to the branch engineering task.

## Done when

- [ ] Choices contain only stable variable/bit/alternative data, archive and reconstruct deterministically, and print meaningful decisions.
- [ ] Commit handles cloned and recomputed spaces, and no-good literals prune the complementary bit decision correctly.
- [ ] Assigned variables and fixed bits are skipped without repeated choices or incomplete trees.

## Validation

- Run complete small-domain search and assignment tests using the shared WordVar framework.
- Run archive round-trip, cloned/recomputed commit, NGL, complementary-alternative, and random-seed determinism tests.
