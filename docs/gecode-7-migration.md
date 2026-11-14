# Gecode 7 migration notes

## Meta-search hooks: master() and slave() are now hub() and spoke()

Rename the overrides. Their signatures and return values are unchanged:

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
is marked `override`. Restart-based search (RBS) and portfolio-based search
(PBS) call the renamed hooks at the same points and with the same defaults
as before.

The names come from a wheel. The hub is the persistent centre: in RBS it
carries the last solution and the no-goods from one restart to the next,
and in PBS it is the common model. Each spoke is a disposable clone of the
hub: RBS searches one spoke per restart, and PBS searches one spoke per
asset. A spoke is not a long-lived peer that exchanges information with the
hub, which is what hub and spoke means in some other solvers, such as
mpi-sppy.

- RBS calls `hub()` after each solution and at each restart, and `spoke()`
  on each new spoke, including the first. The default `hub()` constrains
  the hub with the last solution, if any, and posts the no-goods.
- PBS calls `hub()` once, then clones one spoke per asset and calls
  `spoke()` on it with the asset number. The default `hub()` removes all
  branchers, so `spoke()` must post the branching of each asset.

The examples show typical overrides:

- `examples/crossword.cpp`: a `hub()` that posts no-goods and continues
  after a solution instead of restarting.
- `examples/photo.cpp`: an RBS `spoke()` that relaxes the last solution for
  large neighbourhood search.
- `examples/qcp.cpp`: a PBS `spoke()` that posts a different branching for
  each asset.
