+++
schema_version = 1
id = "parallel-search-worker-control-009"
proposal_key = "document-origin-variant-terminology"
proposal_hash = "sha256:dfb8dfaa20238b7a8b97b42a41deb326a55722bac3d48b3923cfac1f7df8190c"
area = "parallel-search-worker-control"
kind = "research"
interaction = "afk"
priority = "high"
status = "done"
blocked_by = []
+++
# Record the Origin and Variant meta-search terminology contract

## Source

For Gecode 7, the approved breaking-change direction is to replace the master/slave Space roles with the same noun pair in both PBS and RBS. Origin/Variant was selected because it describes the shared derivation relationship between persistent or common state and a specialized exploration space. The user explicitly rejected master/slave and the proposed run/search pair; run and search are verbs that already occur frequently in this domain.

## What to build

Write a concise, source-backed terminology and migration note that defines Origin as the persistent or common Space from which an exploration Space is derived, and Variant as the derived Space configured for one RBS restart episode or one PBS portfolio asset. Include side-by-side PBS and RBS lifecycle diagrams, explain why structural Space roles are more accurate than controller/worker topology, and explain why one vocabulary is retained even though PBS and RBS have different callback contracts. Record the alternatives Archetype/Variant, Basis/Variant, Coordinator/Explorer, Principal/Agent, Archetype/Instance, Control/Asset, and engine-specific Portfolio/Asset plus Origin/Episode, with a concrete strength and weakness for each. Add a separate explicit-rejections section for master/slave, run/search, and using different public role vocabularies for PBS and RBS; do not mislabel merely unselected alternatives as user-rejected.

## Non-goals

- Reopening the approved Origin/Variant naming decision.
- Renaming unrelated uses of master, slave, origin, or variant outside Gecode meta-search.
- Choosing names for the external adjustable-worker control API.

## Acceptance criteria

- [x] The note defines Origin and Variant in terms of logical Space roles, including that an RBS origin can itself be a clone while remaining the origin of the next exploration.
- [x] A PBS/RBS behavior matrix covers invocation frequency, cloning relationship, MetaInfo data, return-value meaning, and default behavior for both roles.
- [x] The rationale explains why Origin/Variant fits both engines and why controller/worker, driver/executor, and scheduler/worker describe runtime topology rather than these Space hooks.
- [x] The alternatives section evaluates every named alternative and clearly separates considered-but-not-selected names from explicitly rejected names.
- [x] The note records that consistent role nouns do not require retaining today's conflated bool-returning PBS/RBS callback contract.

## Validation

- Review the behavior matrix against Space::master, Space::slave, their default implementations, PBS construction, sequential RBS restart handling, and the parallel PBS runnable.
- Run `zd research check` on the terminology note and verify that every external or CPKB claim has a traceable source.
