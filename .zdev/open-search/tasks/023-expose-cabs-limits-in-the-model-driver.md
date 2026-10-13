+++
schema_version = 1
id = "open-search-023"
key = "model-driver-cabs-limits"
area = "open-search"
status = "done"
complexity = "standard"
afk = true
priority = "normal"
blocked_by = []
+++
# Expose CABS limits in the Model driver

## Outcome

Model CLI users can set native CABS width and retained-frontier limits without writing a custom search wrapper.

## Context

gecode/model/driver.hh:829–834 always constructs Search::CABSOptions() while the adjacent Column wrapper receives configured limits. docs/model/README.md documents this restriction. Native CABS and ModelHeuristicSearch already support both limits.

## Boundaries

- Keep unlimited defaults and pass options directly to native CABSOptions. Preserve the distinction between local resource termination and proof.

## Done when

- [x] Two documented driver controls reach CABSOptions and appear in help.
- [x] Width- and frontier-limited runs report stopped/incomplete rather than infeasible or optimal solely because the configured limit was reached.

## Validation

- Extend the existing Model driver fixture with one width-limited and one frontier-limited run; reuse native CABS proof/stop tests. Build the worked Model example and inspect help output.

## Result

Added unlimited-default CABS width/frontier controls to the Model driver, preserving native stopped versus proof semantics and documenting both controls in help.

Validation:

- Independent whole-task PASS at Wde5d6ab343ac2e2b; both exact snapshot comparisons equal.
- Required builds, focused driver/native/inherited tests, check target, worked-example help/default/limited runs and four open examples passed; git diff --check clean.
