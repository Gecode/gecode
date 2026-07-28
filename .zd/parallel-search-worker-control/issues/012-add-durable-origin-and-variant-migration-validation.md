+++
schema_version = 1
id = "parallel-search-worker-control-012"
proposal_key = "add-origin-variant-migration-validation-artifacts"
proposal_hash = "sha256:60a6966bbaa83881dcb1fc8a4ca4e7a34413fa3aa6b0e45f7df98aee78ed5390"
area = "parallel-search-worker-control"
kind = "implementation"
interaction = "afk"
priority = "normal"
status = "done"
blocked_by = ["parallel-search-worker-control-010"]
+++
# Add durable Origin and Variant migration validation

## Source

The completed standalone verification research run research-8a6415956366 found that issue parallel-search-worker-control-011 cannot be accepted from the current durable evidence: explicit negative API compilation, build-checked migration material, migration-sensitive lifecycle coverage, and an exhaustive retained terminology inventory are missing.

## What to build

Add the durable compile fixtures, executable migration-focused tests, build-checked migration example, and retained terminology inventory needed for issue 011 to independently verify the breaking Origin/Variant API migration. Integrate them with the repository's existing Autoconf and CMake validation surfaces so the checks run in normal contributor workflows.

## Non-goals

- Restoring Gecode 6 compatibility aliases.
- Renaming unrelated domain terminology or third-party text.
- Changing Origin/Variant runtime semantics beyond fixing a demonstrated migration regression.

## Acceptance criteria

- [x] A repository-owned compile-pass fixture overrides both Space::origin(const MetaInfo&) and Space::variant(const MetaInfo&) with override, while a compile-fail check rejects the removed master/slave hook signatures and would fail if either obsolete virtual hook returned.
- [x] Executable tests cover Origin and Variant independently and together for RBS, single-asset PBS, sequential multi-asset PBS, and threaded parallel PBS, including RBS return-value decisions and PBS ignored-return behavior.
- [x] Focused tests cover the migration-sensitive no-good, incumbent, clone-ownership, stop, and reset behavior identified by research-8a6415956366.
- [x] A build-checked Gecode 7 migration example gives the exact master-to-origin and slave-to-variant override mapping and states that no compatibility aliases are provided.
- [x] A retained terminology inventory records every remaining case-sensitive and case-insensitive master/slave occurrence in maintained source, tests, examples, and documentation as unrelated, historical migration text, third-party material, or a fixed defect, with no stale meta-search terminology.

## Validation

- Run the new compile-pass and compile-fail API checks and demonstrate that the negative check detects reintroduced master/slave virtual hooks.
- Run the focused Origin/Variant RBS and PBS tests in sequential, threaded, and thread-disabled builds.
- Run the complete Search test suite in threaded and thread-disabled configurations, classifying any reproduced baseline-only failure with before-and-after evidence.
- Build the documentation and examples that contain the migration material.
- Run the retained case-sensitive and case-insensitive terminology sweep and zd issues check parallel-search-worker-control --format json.
