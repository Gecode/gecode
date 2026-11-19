# Gecode 7 migration notes

## Meta-search hooks: hub() and spoke() take contexts

The hooks `master()` and `slave()` and the class `MetaInfo` are replaced:

```cpp
// Gecode 6
bool master(const Gecode::MetaInfo& mi) override;
bool slave(const Gecode::MetaInfo& mi) override;

// Gecode 7
void hub(Gecode::HubContext& hc) override;
void spoke(Gecode::SpokeContext& sc) override;
```

There are no compatibility aliases. `MetaInfo` no longer exists, so an old
override fails to compile. Restart-based search (RBS) calls `hub()` on its
hub and `spoke()` on each spoke, a clone of the hub that one episode
searches. Portfolio-based search (PBS) calls `hub()` once and `spoke()` on
the spoke of each asset.

Both contexts provide what `MetaInfo` provided:

| Gecode 6                               | Gecode 7                                           |
|----------------------------------------|----------------------------------------------------|
| `mi.type()`: `RESTART`, `PORTFOLIO`    | `engine()`: `MetaEngine::Restart`, `Portfolio`     |
| `mi.reason()`: `RR_INIT`               | `event()`: `MetaEvent::Start`                      |
| `RR_SOL`                               | `MetaEvent::Solution`                              |
| `RR_LIM`                               | `MetaEvent::Limit`                                 |
| `RR_CMPL`                              | `MetaEvent::Exhausted`                             |
| (none)                                 | `MetaEvent::Incomplete`, such as LDS at its limit  |
| `RR_NO` (portfolio)                    | `MetaEvent::Start`                                 |
| `mi.restart()`                         | `episode()`                                        |
| `mi.fail()`                            | `statistics().fail`                                |
| `mi.solution()`                        | none: `event()` tells whether there was a solution |
| `mi.last()`                            | `last()`                                           |
| `mi.nogoods()`                         | `nogoods()`                                        |
| `mi.asset()`                           | `asset()`, and `assets()` for their number         |

`episode()` counts from zero and increases by one for every new spoke,
whatever ended the previous episode. In `hub()` it is the episode that
ended, in `spoke()` the episode that starts. `statistics()` holds the
statistics of the episode that ended. `last()` is only valid while the hook
runs; clone the solution to keep it.

The return values become explicit decisions:

| Gecode 6                               | Gecode 7                                               |
|----------------------------------------|--------------------------------------------------------|
| `master()` returns `true`              | `hc.decision(MetaDecision::Restart)`, the default      |
| `master()` returns `false` after a solution | `hc.decision(MetaDecision::Continue)`             |
| (none)                                 | `hc.decision(MetaDecision::Continue)` after a limit    |
| (none)                                 | `hc.decision(MetaDecision::Finish)`                    |
| `slave()` returns `true`               | nothing, `SpokeCoverage::Full` is the default          |
| `slave()` returns `false`              | `sc.coverage(SpokeCoverage::Partial)`                  |
| (none)                                 | `sc.coverage(SpokeCoverage::Reduced)`                  |

`Continue` is only legal after `MetaEvent::Solution` and `MetaEvent::Limit`;
setting it for another event throws `IllegalMetaDecision`. `Finish` ends the
engine with `EndReason::Incomplete`. To prove that no further solution
exists, fail the hub.

The coverage tells the engine what a spoke covers of the hub's search space:

- `Full`: the spoke adds only branching and constraints that the hub
  implies. Exhausting it is a proof, and its no-goods hold in the hub.
- `Reduced`: the spoke prunes solutions but keeps one, or an optimal one, if
  any exists, such as symmetry breaking. Exhausting it is a proof, but its
  no-goods do not hold in the hub. Search for all solutions misses the
  solutions that the spoke prunes.
- `Partial`: the spoke searches part of the hub's search space, such as a
  large neighbourhood. Exhausting it is no proof, and its no-goods do not
  hold in the hub.

The engine trusts the declared coverage. A spoke that declares more than it
covers, such as `Full` for a spoke that removes solutions, gives undefined
results.

`nogoods()` is empty unless the spoke of the episode that ended was `Full`.
PBS honours the coverage of each asset: an asset whose `Partial` spoke
exhausts is dropped instead of ending the portfolio.

Other changes to the hooks:

- RBS calls `hub()` with `MetaEvent::Start` before it clones the first
  spoke. Gecode 6 did not call `master()` then.
- For a best solution engine such as `RBS<T, BAB>`, the engine constrains
  the hub with the last solution before it calls `hub()`. The default
  `hub()` no longer calls `constrain()`, and an override that does not call
  the default keeps the bound. For other engines, such as `RBS<T, DFS>`,
  RBS no longer calls `constrain()` at all.
- After an episode that ends without a proof, such as LDS at its
  discrepancy limit, RBS calls `hub()` with `MetaEvent::Incomplete`. By
  default, it finishes when the spoke was `Full` or `Reduced`, as a new
  spoke would repeat the same search, and restarts when it was `Partial`.
  Both `Restart` and `Finish` are legal.
- PBS calls `status()` on the hub after `hub()`, so `hub()` can post
  constraints. In Gecode 6, cloning such a hub threw `SpaceNotStable`.
- The default PBS `hub()` still removes all branchers, so `spoke()` must post
  the branching of each asset. When the hub had branchers but a spoke has
  none after `spoke()`, PBS throws `Search::SpokeNoBrancher` instead of
  returning the unassigned spoke as a solution.

The examples show typical overrides:

- `examples/crossword.cpp`: a `hub()` that posts no-goods and continues
  after a solution instead of restarting.
- `examples/photo.cpp`: an RBS `spoke()` that relaxes the last solution for
  large neighbourhood search and declares `SpokeCoverage::Partial`.
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
on its own without that proof, for example limited discrepancy search at its
discrepancy limit or CABS at its width limit. Calling `next()` again does not
continue an incomplete search. `Limit` means that a stop object stopped the
engine, and calling `next()` again continues the search.

Code that used `!stopped()` as a proof, such as reporting optimality, should
test for `EndReason::Exhausted`. Code that resumes a search after a limit
should test for `EndReason::Limit`.

## Portfolio stop objects see the whole portfolio

The stop object in the options of `PBS` is now checked with the statistics
of the whole portfolio, the sum over all assets. In Gecode 6, it was checked
for each asset with the statistics of that asset, so a failure limit of
`n` for a portfolio of `k` assets allowed up to `k * n` failures. A stop
object that should limit each asset on its own belongs in the options of an
engine builder:

```cpp
Gecode::Search::Options a, b;
a.stop = &dfs_stop;
b.stop = &lds_stop;
Gecode::SEBs sebs({Gecode::dfs<Model>(a), Gecode::lds<Model>(b)});
Gecode::PBS<Model> engine(model, sebs, portfolio_options);
```
