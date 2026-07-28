+++
schema_version = 1
id = "parallel-search-worker-control-010"
proposal_key = "rename-meta-search-origin-variant"
proposal_hash = "sha256:a8cb297c2d76310d5cc328f7ac6a22b8102df18dcbda7e412eb72ffdeb59469a"
area = "parallel-search-worker-control"
kind = "implementation"
interaction = "afk"
priority = "high"
status = "done"
blocked_by = ["parallel-search-worker-control-009"]
+++
# Rename meta-search Space roles to Origin and Variant

## Source

Gecode 7 permits breaking API and ABI changes, and the approved terminology contract replaces the misleading master/slave role names consistently across PBS and RBS.

## What to build

Apply the Origin/Variant vocabulary end to end. Rename the public Space::master and Space::slave hooks to Space::origin and Space::variant without compatibility aliases. Update default implementations, MetaInfo documentation, RBS and PBS calls, examples, tests, and generated/reference documentation. Rename internal identifiers according to the object they actually denote: retained or common Space objects become origin; derived exploration objects and PBS runnable assets become variant; pointers to the enclosing PBS coordinator become portfolio or coordinator rather than being mechanically called origin; wrapped search engines become engine or variant_engine. Preserve all existing restart, no-good, incumbent, completeness, asset-index, stop, and ownership semantics.

## Non-goals

- Providing deprecated master/slave compatibility wrappers.
- Changing search order, restart policy, portfolio asset policy, or default hook behavior.
- Automatically splitting MetaInfo or changing bool return types unless approved as a separate API-contract change.
- Renaming unrelated third-party or historical text where the terms do not denote these meta-search roles.

## Acceptance criteria

- [x] The public Gecode 7 Space API exposes origin(const MetaInfo&) and variant(const MetaInfo&) and no longer exposes master(const MetaInfo&) or slave(const MetaInfo&).
- [x] RBS invokes origin for persistent-state updates and variant for each derived restart exploration with behavior identical to the pre-rename implementation.
- [x] PBS invokes origin once for common-space preparation and variant once per asset, including the single-asset path, with behavior identical to the pre-rename implementation.
- [x] Internal class, member, local-variable, parameter, comment, and diagnostic names describe their actual Origin, Variant, portfolio-coordinator, or wrapped-engine roles rather than applying a blind word substitution.
- [x] Public documentation points to the terminology note and includes a Gecode 7 migration example showing the two required override renames.

## Validation

- Build Gecode with and without thread support and run the focused sequential RBS, sequential PBS, and parallel PBS tests.
- Compile representative model subclasses overriding origin, variant, or both and verify calls through Space dispatch to the intended override.
- Use focused repository searches to ensure obsolete meta-search declarations, calls, identifiers, and documentation are gone while reviewing every retained occurrence.
