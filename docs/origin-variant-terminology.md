# Origin and Variant terminology contract

## Decision

Gecode 7 uses **Origin** and **Variant** for the two `Space` roles shared by
restart-based search (RBS) and portfolio-based search (PBS).

An Origin is the persistent or common `Space` from which an exploration space
is derived. A Variant is the derived `Space` configured for one RBS restart
episode or one PBS portfolio asset. These are logical roles. An RBS Origin can
itself be a clone of the state used by the preceding episode while remaining
the Origin of the next exploration.

## Lifecycles

```text
RBS
origin[n] -- origin(restart_info) --> prepared state
                                      |-- clone --> origin[n+1]
                                      `-- variant(restart_info) --> explored variant[n]

PBS
input -- optional clone --> origin -- origin(portfolio_info)
                                   |-- derive/configure --> variant[0]
                                   |-- derive/configure --> variant[1]
                                   `-- derive/configure --> variant[n]
```

The RBS allocation identity changes across an episode: the prepared object
becomes the explored Variant and its clone becomes the next Origin. Origin
therefore names the source role for the next derivation, not the oldest object
in the solve. In single-asset PBS, one object passes through the Origin stage
and then the Variant stage; the names do not require separate allocations.

## Behavior matrix

| Engine | Role | Invocation and relationship | Information and result | Default behavior |
| --- | --- | --- | --- | --- |
| RBS | Origin | Repeated after solutions and at restart boundaries; prepares state before the next Origin clone is retained | Restart reason, counts, last solution, and no-goods. The Boolean can decide whether a solution triggers a restart | Constrain against the last solution when present, post no-goods, and request restart |
| RBS | Variant | Once for each derived restart exploration | Restart information. The Boolean declares the exploration complete or incomplete | No additional configuration; complete |
| PBS | Origin | Once on the common space before asset construction | Portfolio metadata; result ignored | Remove common branchers so assets can install their own branching |
| PBS | Variant | Once per asset, including the single-asset path | Asset number starting at zero; result ignored | No additional configuration |

The common nouns do not require one callback contract. PBS and RBS have
different invocation frequencies and result semantics. Gecode 7 can later
represent those differences with engine-specific metadata, result types, or
overloads while retaining Origin and Variant. The rename itself preserves
current behavior.

## Why topology names are weaker

`Controller/Worker`, `Driver/Executor`, and `Scheduler/Worker` describe runtime
components that allocate or execute work. The public hooks belong to `Space`,
whose relevant relationship is persistence, cloning, specialization, and
exploration. The PBS coordinator is an engine object rather than the `Space`
passed to the first hook, and the retained RBS `Space` stores persistent state
without scheduling the underlying engine.

Internal names therefore follow the actual objects. A retained or common
`Space*` can be an `origin`; a derived exploration space or per-asset runnable
can be a `variant`. A pointer to the enclosing PBS object is a `portfolio` or
`coordinator`, and its wrapped `Engine*` is an `engine` or `variant_engine`.

## Alternatives considered but not selected

| Pair | Strength | Reason not selected |
| --- | --- | --- |
| Archetype / Variant | Exact common-form relationship | `Archetype` is less immediate in a low-level API |
| Basis / Variant | Handles an evolving RBS source | `Basis` carries linear-algebra associations |
| Coordinator / Explorer | Good engine-topology terminology | The first public hook is on a `Space` that does not coordinate execution |
| Principal / Agent | Clear noun pair | Suggests delegation and autonomy rather than cloning |
| Archetype / Instance | Clear type-to-realization relation | `Instance` already commonly means a problem instance |
| Control / Asset | `Asset` fits PBS | `Control` can be a verb and the pair fits RBS poorly |
| Portfolio / Asset; Origin / Episode | Locally expressive for each engine | Four role names hide the shared derivation and complicate models supporting both engines |

These are considered alternatives. They were not explicitly rejected by the
user.

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
