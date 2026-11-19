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

The hooks run on these spaces:

- RBS calls `hub()` after each solution and at each restart. The hub
  carries the last solution and the no-goods from one restart to the next.
  RBS calls `spoke()` on each new spoke, a clone of the hub, including the
  first. The default `hub()` constrains the hub with the last solution, if
  any, and posts the no-goods.
- PBS calls `hub()` once, then calls `spoke()` with the asset number on the
  spoke of each asset, the hub or a clone of it. The default `hub()`
  removes all branchers, so `spoke()` must post the branching of each
  asset.

The examples show typical overrides:

- `examples/crossword.cpp`: a `hub()` that posts no-goods and continues
  after a solution instead of restarting.
- `examples/photo.cpp`: an RBS `spoke()` that relaxes the last solution for
  large neighbourhood search.
- `examples/qcp.cpp`: a PBS `spoke()` that posts a different branching for
  each asset.

## Cutoff generators implement copy()

`Search::Cutoff` has a new pure virtual member function:

```cpp
virtual Gecode::Search::Cutoff* copy(void) const;
```

It returns a copy of the generator in its current state that continues the
sequence on its own. A copy of `Cutoff::rnd()` draws its own random
sequence, so that the assets restart at different times. A portfolio of
restart-based engines, `PBS<T, RBS>`, gives each asset its own copy of the
cutoff in its options and deletes the original once the portfolio is built.
In Gecode 6.6, the assets drew their values from one shared cutoff sequence.

A custom cutoff generator must implement `copy()`. For a generator whose
state is plain data, the copy constructor suffices:

```cpp
Gecode::Search::Cutoff*
MyCutoff::copy(void) const {
  return new MyCutoff(*this);
}
```

A generator that owns other generators copies them with their own
`copy()`.
