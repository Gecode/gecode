+++
schema_version = 1
id = "boost-002"
key = "nextafter-study"
area = "boost"
status = "done"
complexity = "advanced"
afk = true
blocked_by = ["boost-001"]
+++
# Measure nextafter outward rounding against switched rounding modes

## Outcome

We know how outward rounding with std::nextafter, applied after round-to-nearest operations, compares with the current switched rounding modes: the tightness of the bounds, the float test results, the float example solutions and search statistics, and the runtime.

## Context

This answers the brief's open question for approach 3. Build a prototype under build/ that replaces the RoundingBase operations used by FloatVal arithmetic with nextafter outward rounding. Keep MPFR transcendental functions as they are unless a function needs the change. Compare against the unchanged build on the float suite (`bin/gecode-test -test Float::`) and on the float examples archimedean-spiral, cartesian-heart, descartes-folium, and golden-spiral (solutions, nodes, failures, and runtime in Release). If background/rounding-probe.md reports a soundness failure, the switched-rounding reference must be the repaired implementation. If that repair is not committed yet, stop and report a blocker instead of measuring. Name the reference revision in the study.

## Boundaries

- Keep prototype sources and build outputs under build/, using a source copy there if necessary. Do not switch branches or edit maintained sources for the prototype. Commit only the study and the brief index.

## Done when

- [x] .zdev/boost/background/nextafter-study.md records the prototype's approach, the commands, and the measured differences in tightness, test results, example solutions and statistics, and runtime, and the brief indexes it.
- [x] If the prototype cannot be built within the task, the study says what blocked it, and the task stays open as a blocker instead of being completed.

## Validation

- Every number names the command that produced it.

## Result

Measured nextafter outward rounding (P1, always widening) and an error-term variant (P2) against the switched-mode reference 3f4b5272f9 (boost-006); both sound, P1 much looser with changed search, P2 tight except near zero, underflow, and overflow; the study reports facts only for the boost-004 decision

Validation:

- Rounding probe (AppleClang, LLVM 22, GCC 14 at O0/O2/O3) and MPFR harness in four caller modes: 0 unsound rows for ref, P1, P1 without barrier, and P2
- Float:: with 8 threads: ref 1561/0, P1 1109/452, P2 1554/7; with harness cuts clamped P1 1445/116, P2 1561/0; failures replayed and classified
- Examples min of 11, microbenchmark min of 7, one indicative single-threaded suite run per build; every number names its command
- Independent Codex verifier: pass on round 3 after corrections to the Fast2Sum exactness claims, the microbenchmark interpretation, and the brief entry
