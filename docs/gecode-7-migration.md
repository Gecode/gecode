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

There are no compatibility aliases. An old override marked `override` fails
to compile; one without it compiles but is never called. Restart-based search (RBS) and portfolio-based search
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

## stopped() is replaced by end_reason()

`Search::Engine::stopped()` and `Search::Base<T>::stopped()` are removed,
and so is the experimental `limited()`. After `next()` returns no solution,
`end_reason()` tells why:

```cpp
enum class Gecode::Search::EndReason : unsigned char {
  NotEnded, Exhausted, Incomplete, Limit
};
```

| Gecode 6                                      | Gecode 7                                        |
|-----------------------------------------------|-------------------------------------------------|
| `stopped()` is true                           | `end_reason() == EndReason::Limit`              |
| `stopped()` is false after no solution        | `end_reason() == EndReason::Exhausted` or `EndReason::Incomplete` |
| `limited()` is true (AStar, CABS)             | `end_reason() == EndReason::Incomplete`         |
| last `next()` returned a solution             | `end_reason() == EndReason::NotEnded`           |

`Exhausted` means that the engine explored everything it was asked to
explore: there is no further solution, and the last solution of a
branch-and-bound search is optimal. `Incomplete` means that the engine ended
on its own without that proof, for example CABS at its width limit. Calling
`next()` again does not continue an incomplete search. `Limit` means that a
stop object stopped the engine, and calling `next()` again continues the
search.

Code that used `!stopped()` as a proof, such as reporting optimality, should
test for `EndReason::Exhausted`. Code that resumes a search after a limit
should test for `EndReason::Limit`.
