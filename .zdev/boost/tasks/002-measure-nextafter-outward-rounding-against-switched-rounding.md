+++
schema_version = 1
id = "boost-002"
key = "nextafter-study"
area = "boost"
status = "open"
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

- [ ] .zdev/boost/background/nextafter-study.md records the prototype's approach, the commands, and the measured differences in tightness, test results, example solutions and statistics, and runtime, and the brief indexes it.
- [ ] If the prototype cannot be built within the task, the study says what blocked it, and the task stays open as a blocker instead of being completed.

## Validation

- Every number names the command that produced it.
