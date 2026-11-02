+++
schema_version = 1
id = "boost-003"
key = "approach-study"
area = "boost"
status = "done"
complexity = "advanced"
afk = true
priority = "high"
blocked_by = ["boost-001", "boost-002"]
+++
# Compare the refreshed-copy, improved-copy, and support-library approaches

## Outcome

A study compares the three approaches in the brief (a refreshed copy, an improved copy, and a Gecode support library) for cost, risk, and reproducibility, using measured facts. It ends with a recommendation that the user can decide on, together with background/nextafter-study.md.

## Context

Read the brief in full, then background/rounding-probe.md. Facts to establish: (1) what current upstream Boost.Interval changed since the 2012 copy, and the header closure it needs today, found in a scratch directory under build/; (2) what renaming and patching are needed, and whether a script can reproduce the vendored copy exactly; (3) for a support library, the operations Gecode uses (listed in the brief's Current state) and an estimate of the code size for each, including the rounding layer and the possibly/certainly comparison semantics that gecode/float/val.hpp gets from catching comparison_error. gecode/third-party/boost/extract.py only emits copy commands. It does not rename namespaces or apply patch.txt, and its macro-include resolver recognizes GECODE_BOOST_* definitions rather than upstream BOOST_* ones. Account for every unresolved include before calling its output the complete closure.

## Boundaries

- No changes to gecode/ or test/ in this task. Prototypes and Boost downloads stay under build/ and are not committed.
- Do not add a dependency on an installed Boost to the build, even as an experiment in tracked files.

## Done when

- [x] .zdev/boost/background/approach-study.md holds the measured facts for (1) to (3), the cost and risk of each approach, and one recommendation with its reasons that also uses background/nextafter-study.md, and the brief indexes it.
- [x] The brief's investigation questions are answered or point to the studies. If a question cannot be answered, the study says so and why, and the task is not completed as answered.

## Validation

- Every number in the study names the command or measurement that produced it.

## Result

Wrote background/approach-study.md comparing a Boost 1.92.0 refresh, an improved 2012 copy (Boost 1.47.0), and a Gecode support library on upstream changes, closures, scripted byte-exact reproduction, builds, probes, the float suite and examples, support-library size, and cost and risk. It recommends the support library with switched-mode rounding for the user's decision in boost-004; the improved copy is the second choice. Boost 1.92.0 needs unreleased upstream fix 6a59a02 to keep nth_root and the example searches unchanged.

Validation:

- Every number cites a command [C1]-[C26] with scripts in Appendix A. Test A reproduces HEAD's copy byte for byte. Float:: suite passes 1561 of 1561 for head, refresh, refreshfix, and improve. Rounding probe 9 of 9 configurations exact for every variant. Independent verifier PASS after one wording correction round.
