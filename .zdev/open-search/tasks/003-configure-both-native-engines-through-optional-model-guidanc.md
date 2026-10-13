+++
schema_version = 1
id = "open-search-003"
key = "model-guidance"
area = "open-search"
status = "done"
complexity = "advanced"
afk = true
priority = "normal"
blocked_by = ["open-search-002"]
+++
# Configure both native engines through optional Model guidance

## Outcome

A Model user selects A* or CABS with a safe scalar objective-bound default or explicit expert guidance, compiles a bounded integer optimization model, and obtains/renders solutions without a Space subclass.

## Context

Read Initial search contract and Native, Model, and portfolio integration in ../brief.md, plus Model seams in ../background/integration.md. Start with gecode/model/runtime.hh, model/runtime.cpp, model/model.cpp, model/compile.cpp, model/optimize.cpp, model/driver.hh, and test/model/compile.cpp. CompiledModel already exposes objective family/sense and materializes a scalar objective variable. Custom-objective dependencies demonstrate validation, retention, alias barriers, and context resolution. Explicit guidance must retain its own dependencies even when they are not output or branching variables.

## Boundaries

- Support scalar integer minimization/maximization and diagnose float, lexicographic, and custom objective families for this initial route.
- Keep guidance lazy and explicit; do not retain every variable, use dummy output/posting to keep handles alive, or build a generic callback framework.
- Keep all engine/frontier algorithms in the native search layer. Reuse existing objective/dependency information; do not add automatic domain-heuristic discovery or a general model analyzer.

## Done when

- [x] The owning Model search journey configures the same native A*/CABS engines, preserves artifact/root lifetimes, and renders returned solutions through existing output APIs. No Model helper becomes a prerequisite for native callers.
- [x] Declared guidance dependencies are validated and survive applicable compilation levels, handle remapping, repeated root creation, and independent clones without adding guidance cost to undeclared models.
- [x] A small closed CP example demonstrates minimization/maximization and compares A*, CABS, and BAB. Document scalar objective-bound defaults, explicit valid bounds/ranking, incumbent publication, and stop versus proof.
- [x] Explicit driver selection of either engine derives scalar sense and uses the propagated objective lower/upper bound when no expert bound is declared. Unsupported objective families and invalid dependencies produce clear diagnostics; explicit expert guidance can replace the default.
- [x] Focused Model tests use dependencies absent from output/ordinary branching, compare optima with BAB or tiny exhaustive reference results, and exercise compile/root lifetime and unsupported-family diagnostics.
- [x] Documentation explains what existing Model objective/dependency information configures today and what structural branching/ranking suggestions remain investigations; all settings translate to native calls.

## Validation

- Build gecode-test and the changed Model example(s); run the focused guidance/compilation tests and existing Model lifecycle, objective, and open-list checks relevant to the declaration changes.
- Run the existing check target and a focused installed-header consumer through the established repository pattern; no new installation harness.

## Result

Added optional retained Model guidance, an owning native A*/CABS facade, explicit driver selection, and a min/max quickstart.

Validation:

- Independent verifier passed snapshot Wd89080cd583215fc with no findings and unchanged checkout.
- Release build, 18 focused Model/native tests, complete check target, quickstart and both driver routes passed.
- Installed exported-target consumer and both installed driver routes, inherited checks, and four open examples passed.
