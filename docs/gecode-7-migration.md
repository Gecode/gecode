# Gecode 7 migration notes

## Meta-search Space roles

Gecode 7 names the two `Space` roles used by restart-based search (RBS) and
portfolio-based search (PBS) **Hub** and **Spoke**.
The [Hub and Spoke terminology contract](hub-spoke-terminology.md)
records the lifecycle and naming rationale behind these roles.

A Hub is the persistent or common Space from which an exploration Space
is derived. A Spoke is the derived Space configured for one RBS restart
episode or one PBS portfolio asset. These are logical roles. In RBS, the
prepared object becomes the explored Spoke while its clone becomes the next
Hub.

The same terms apply to both engines because both use the same derivation:
prepare common state, derive an exploration Space, and specialize that Space.
The callback contracts remain engine-specific. RBS uses the return values to
control restart and completeness behavior, while PBS ignores them during
one-time Hub and per-asset Spoke configuration.

Gecode 7 makes the following breaking override rename:

```cpp
// Gecode 6
bool master(const Gecode::MetaInfo& mi) override;
bool slave(const Gecode::MetaInfo& mi) override;

// Gecode 7
bool hub(const Gecode::MetaInfo& mi) override;
bool spoke(const Gecode::MetaInfo& mi) override;
```

There are no compatibility aliases. `Space` declares `master()` and
`slave()` as deleted, so an old override fails to compile whether or not it
is marked `override`.

The build-checked
[Hub and Spoke migration example](../examples/hub-spoke.cpp)
contains both current override declarations.

The rename does not change callback timing or behavior:

- RBS calls `hub` when persistent restart state is updated and calls
  `spoke` for each derived restart exploration.
- PBS calls `hub` once before constructing assets and calls `spoke` once
  per asset, including the single-asset path.
- The default RBS Hub behavior still constrains against the last solution,
  posts no-goods, and requests a restart.
- The default PBS Hub behavior still removes common branchers before
  asset-specific branching is installed.
- The default Spoke behavior still performs no additional configuration and
  returns `true`.

Hub and Spoke describe Spaces. They do not replace runtime terms such as
portfolio coordinator, wrapped engine, or worker thread where those terms
describe different objects.
