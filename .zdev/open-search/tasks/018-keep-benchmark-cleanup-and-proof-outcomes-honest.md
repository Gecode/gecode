+++
schema_version = 1
id = "open-search-018"
key = "repair-benchmark-outcomes"
area = "open-search"
status = "done"
complexity = "standard"
afk = true
priority = "high"
blocked_by = []
+++
# Keep benchmark cleanup and proof outcomes honest

## Outcome

The TSPTW runner cannot resume past an unconfirmed child exit or publish contradictory feasible and infeasible outcomes for one case.

## Context

misc/tsptw-benchmark.py:455–461 writes a reap event even if kill_group() raises; budget_state():74–85 then removes that pending launch. README.md:646 records observed termination-permission interruptions that required manual recovery. comparison_report():631–656 rejects inconsistent optimality claims but omits the symmetric check for an infeasibility claim alongside another row's independently checked tour.

## Boundaries

- Keep recovery local to the existing runner and single-writer workflow. Preserve archived runs and their historical interpretation; do not rewrite frozen results or rerun a campaign.
- A failed spawn that created no child may close its attempt; an unconfirmed termination must remain unresolved. Do not signal a recovered PID without confirming it still belongs to the recorded launch.

## Done when

- [x] Only confirmed child exit, confirmed absence during explicit recovery, or a failed spawn closes the launch; unresolved termination blocks new solver launches and reports useful owned-process identity.
- [x] Cleanup failures preserve the original error and partial captures, and elapsed cleanup work is accounted for without claiming an unobserved reap time.
- [x] An infeasibility proof combined with any checked tour for the same case is rejected, including tours from timeout/RSS rows.

## Validation

- Add focused fixtures to misc/test-tsptw-benchmark.py for kill/wait failure and blocked resume, successful exit/recovery, and cross-row infeasibility contradictions. Run the existing offline runner tests. No solver campaign is needed.

## Result

Runner preserves unresolved launch identity and partial captures, blocks unsafe resume, supports absence-only recovery with conservative timing, and rejects infeasibility claims alongside checked tours.

Validation:

- All 11 offline runner tests, recovery help, syntax compilation and diff checks passed. Independent verification passed with unchanged context; no solver campaign or archived-result changes.
