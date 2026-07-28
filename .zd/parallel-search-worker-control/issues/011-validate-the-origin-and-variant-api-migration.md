+++
schema_version = 1
id = "parallel-search-worker-control-011"
proposal_key = "validate-origin-variant-migration"
proposal_hash = "sha256:65df77bb32f35a9d1a4f4ff329edb41def6ca2a7951d11319902c181a3c986b9"
area = "parallel-search-worker-control"
kind = "verification"
interaction = "afk"
priority = "normal"
status = "done"
blocked_by = ["parallel-search-worker-control-010", "parallel-search-worker-control-012"]
+++
# Validate the Origin and Variant API migration

## Source

A breaking rename across public virtual functions, sequential and parallel meta-engines, documentation, and model overrides needs explicit source-compatibility and behavioral validation beyond the implementation's focused tests.

## What to build

Create a migration-focused validation slice for the Origin/Variant rename. Add compile-time API tests and small executable models that exercise RBS and PBS overrides independently and together. Verify default behavior, custom no-good and incumbent handling, RBS completeness decisions, PBS asset-index specialization, single-asset PBS, sequential portfolios, parallel portfolios, cloning ownership, stop/reset, and documentation examples. Produce a terminology sweep report that classifies every remaining master/slave occurrence as unrelated, historical migration text, third-party material, or a defect to fix.

## Non-goals

- Maintaining Gecode 6 source compatibility.
- Broad terminology cleanup unrelated to RBS, PBS, and their Space hooks.
- Changing the selected Origin/Variant terminology based only on incidental identifier collisions.

## Acceptance criteria

- [x] Compile tests fail if the old Space hook names return or if the new signatures cannot be overridden with override.
- [x] Behavioral tests cover both Origin and Variant callbacks in RBS, single-asset PBS, sequential multi-asset PBS, and parallel PBS.
- [x] Tests demonstrate that RBS return-value semantics and PBS ignored-return semantics have not accidentally changed during the rename.
- [x] The migration example builds against the Gecode 7 headers and gives users an exact old-to-new override mapping.
- [x] Every remaining master/slave occurrence in the maintained Gecode source and documentation is reviewed and recorded, with no stale meta-search terminology left behind.

## Validation

- Run the complete search test suite in sequential and threaded configurations, plus the documentation/example build.
- Run focused case-sensitive and case-insensitive terminology scans over maintained source, tests, examples, and documentation and attach the classified results.
- Run `zd issues check parallel-search-worker-control` after all migration evidence is linked.

## Evidence lineage

Issue `parallel-search-worker-control-012` added the durable validation artifacts
required by this issue and is complete. Its corrective implementation is commit
`c1dba83d0c34cf1c1dccffd86d419f7db599308f`, collected by run
`rework-parallel-search-worker-control-012-41f8fc58c673` and integrated through
zdev after independent verification.

The authoritative verifier run is
`verify-parallel-search-worker-control-012-901a2bbc618a`. It passed all five
acceptance criteria after reconstructing the collected patch on its recorded
base. Its recorded validation includes:

- independent positive and negative compilation checks for the current and
  removed hooks;
- repeated threaded and thread-disabled MetaDispatch scenarios for RBS and
  every PBS shape;
- migration-sensitive no-good, reset, ownership, stop, and BAB incumbent
  propagation checks;
- CMake and Autoconf example, documentation, terminology, and `check` targets;
- a passing complete threaded Search suite; and
- an exact before-and-after match for the twelve pre-existing thread-disabled
  PBS expectation failures.

The retained verification report is
`.zd/_runs/verify-parallel-search-worker-control-012-901a2bbc618a/result/report.md`.
The retained source-backed audit is
`.zd/parallel-search-worker-control/reports/research-24d6135c01fe151cb625712535c02aea6522636a982c0131995fb52283aaf358.md`.
