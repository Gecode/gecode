# Gecode 7 migration notes

## Meta-search Space roles

Gecode 7 names the two `Space` roles used by restart-based search (RBS) and
portfolio-based search (PBS) **Origin** and **Variant**.
The [Origin and Variant terminology contract](origin-variant-terminology.md)
records the lifecycle and naming rationale behind these roles.

An Origin is the persistent or common Space from which an exploration Space
is derived. A Variant is the derived Space configured for one RBS restart
episode or one PBS portfolio asset. These are logical roles. In RBS, the
prepared object becomes the explored Variant while its clone becomes the next
Origin.

The same terms apply to both engines because both use the same derivation:
prepare common state, derive an exploration Space, and specialize that Space.
The callback contracts remain engine-specific. RBS uses the return values to
control restart and completeness behavior, while PBS ignores them during
one-time Origin and per-asset Variant configuration.

Gecode 7 makes the following breaking override rename:

```cpp
// Gecode 6
bool master(const Gecode::MetaInfo& mi) override;
bool slave(const Gecode::MetaInfo& mi) override;

// Gecode 7
bool origin(const Gecode::MetaInfo& mi) override;
bool variant(const Gecode::MetaInfo& mi) override;
```

There are no compatibility aliases. Keeping `override` on model hooks makes
the compiler identify every model that still uses a Gecode 6 signature.
The build-checked
[Origin and Variant migration example](../examples/origin-variant.cpp)
contains both current override declarations.

The rename does not change callback timing or behavior:

- RBS calls `origin` when persistent restart state is updated and calls
  `variant` for each derived restart exploration.
- PBS calls `origin` once before constructing assets and calls `variant` once
  per asset, including the single-asset path.
- The default RBS Origin behavior still constrains against the last solution,
  posts no-goods, and requests a restart.
- The default PBS Origin behavior still removes common branchers before
  asset-specific branching is installed.
- The default Variant behavior still performs no additional configuration and
  returns `true`.

Origin and Variant describe Spaces. They do not replace runtime terms such as
portfolio coordinator, wrapped engine, or worker thread where those terms
describe different objects.

The [retained terminology inventory](origin-variant-terminology-inventory.md)
classifies every remaining occurrence of the Gecode 6 role terms.
