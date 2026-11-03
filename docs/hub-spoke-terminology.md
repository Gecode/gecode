# Hub and Spoke terminology contract

## Decision

Gecode 7 uses **Hub** and **Spoke** for the two `Space` roles shared by
restart-based search (RBS) and portfolio-based search (PBS).

The names come from one picture, a wheel. The hub stays at the centre and
holds what persists, and each spoke goes out from it to explore. A Hub is the
persistent or common `Space` from which an exploration space is derived. A
Spoke is the derived `Space` configured for one RBS restart episode or one
PBS portfolio asset.

These are logical roles. An RBS Hub can itself be a clone of the state used
by the preceding episode while remaining the Hub of the next exploration.

## Lifecycles

```text
RBS
hub[n] -- hub(restart_info) --> prepared state
                                      |-- clone --> hub[n+1]
                                      `-- spoke(restart_info) --> explored spoke[n]

PBS
input -- optional clone --> hub -- hub(portfolio_info)
                                   |-- derive/configure --> spoke[0]
                                   |-- derive/configure --> spoke[1]
                                   `-- derive/configure --> spoke[n]
```

The RBS allocation identity changes across an episode: the prepared object
becomes the explored Spoke and its clone becomes the next Hub. Hub therefore
names the centre that the next Spoke leaves from, not the oldest object in the
solve. In single-asset PBS, one object passes through the Hub stage and then
the Spoke stage; the names do not require separate allocations.

## Behavior matrix

| Engine | Role | Invocation and relationship | Information and result | Default behavior |
| --- | --- | --- | --- | --- |
| RBS | Hub | Repeated after solutions and at restart boundaries; prepares state before the next Hub clone is retained | Restart reason, counts, last solution, and no-goods. The Boolean can decide whether a solution triggers a restart | Constrain against the last solution when present, post no-goods, and request restart |
| RBS | Spoke | Once for each derived restart exploration | Restart information. The Boolean declares the exploration complete or incomplete | No additional configuration; complete |
| PBS | Hub | Once on the common space before asset construction | Portfolio metadata; result ignored | Remove common branchers so assets can install their own branching |
| PBS | Spoke | Once per asset, including the single-asset path | Asset number starting at zero; result ignored | No additional configuration |

The common nouns do not require one callback contract. PBS and RBS have
different invocation frequencies and result semantics. Gecode 7 can later
represent those differences with engine-specific metadata, result types, or
overloads while retaining Hub and Spoke. The rename itself preserves current
behavior.

## Why Hub and Spoke

- **One metaphor.** Each word names a part of the same wheel, so either word
  suggests the other and the relationship between them.
- **Both engines.** A portfolio is a hub with several spokes at once. Restart
  search sends out one spoke at a time from a hub that keeps learning between
  them through the last solution and no-goods.
- **Short and memorable.** Both are one-syllable nouns that read well as hook
  names, `hub()` and `spoke()`, and in prose.
- **Not used for other things.** Before this change, neither word occurred in
  the Gecode sources or in MPG outside word-list data, and neither names
  anything in the C++ standard library. Searching for them finds only these
  roles.

The picture does not say that a spoke is a clone of its hub. The
documentation states that relationship explicitly.

Hub and spoke describes positions, not agency. The hub does not schedule or
run its spokes: the engine does. Topology names that describe the runtime,
such as `Controller/Worker`, `Driver/Executor`, and `Scheduler/Worker`, were
not chosen because the PBS coordinator is an engine object rather than the
`Space` passed to the first hook, and because `worker` already names threads.

Internal names follow the actual objects. The retained RBS `Space*` is the
`hub`, and each per-asset PBS runnable is a `Spoke`. A spoke's pointer to the
enclosing PBS engine is its `portfolio`, and the search engine it wraps is its
`engine`.

## Alternatives considered but not selected

| Pair | Strength | Reason not selected |
| --- | --- | --- |
| Origin / Variant | Clear derivation relationship; close to the "variants" of a model in MPG | `variant` already has many unrelated uses, such as model variants in the examples and `std::variant`; `origin` suggests the oldest object, but the RBS Hub is replaced every episode |
| Theme / Variation | One familiar metaphor that fits both engines | Less immediate as a hook name, and a musical theme stays fixed while the Hub gains constraints |
| Anchor / Link; Hyperspace / Subspace | Explicit connection between the two words | `link` and `anchor` occur elsewhere in the sources; a Spoke for a portfolio asset is often not a smaller space |
| Archetype / Variant | Exact common-form relationship | `Archetype` is less immediate in a low-level API |
| Basis / Variant | Handles an evolving RBS source | `Basis` carries linear-algebra associations |
| Coordinator / Explorer | Good engine-topology terminology | The first public hook is on a `Space` that does not coordinate execution |
| Principal / Agent | Clear noun pair | Suggests delegation and autonomy rather than cloning |
| Archetype / Instance | Clear type-to-realization relation | `Instance` already commonly means a problem instance |
| Control / Asset | `Asset` fits PBS | `Control` can be a verb and the pair fits RBS poorly |
| Portfolio / Asset; Hub / Episode | Locally expressive for each engine | Four role names hide the shared derivation and complicate models supporting both engines |

These alternatives were considered but not rejected outright.

## Explicitly rejected names and designs

- **Master / Slave** is the terminology being removed. It describes authority
  and subordination rather than common state and a derived exploration.
- **Run / Search** was rejected because both words are verbs and already occur
  frequently in this API. As member names, they do not clearly denote roles.
- **Different public role vocabularies for PBS and RBS** were rejected. Contract
  differences should be represented by contracts, not by replacing one shared
  derivation vocabulary with four unrelated nouns.

## Migration boundary

The Gecode 7 migration renames the public virtual functions, overrides, calls,
documentation, tests, and role-bearing internal identifiers. It does not add
deprecated aliases, change restart decisions, change completeness reporting,
change PBS asset numbering, or mechanically rename unrelated historical and
third-party occurrences.
