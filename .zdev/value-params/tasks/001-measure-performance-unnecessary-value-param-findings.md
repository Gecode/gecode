+++
schema_version = 1
id = "value-params-001"
key = "inventory"
area = "value-params"
status = "done"
afk = true
priority = "high"
blocked_by = []
+++
# Measure performance-unnecessary-value-param findings

## Outcome

A repair-ready inventory lists the performance-unnecessary-value-param diagnostics for one unique function parameter per row, with a proposed AllowedTypes regex for shallow handles and the diagnostics grouped into coherent repair groups.

## Context

The brief defines shallow handles and lists the owned or reference-counted types to fix. Run clang-tidy 22 from /opt/homebrew/opt/llvm@22/bin with the check selected through `-checks=-*,performance-unnecessary-value-param` and AllowedTypes set under CheckOptions in `--config`, since -checks cannot set options. Use the same coverage as `uv run misc/check.py lint`: its owned-source filters, -UNDEBUG, and a compile database that includes experimental, CBS, and Gist code. Deduplicate header diagnostics that repeat across translation units. For each unique parameter, record the declaration and definition locations, whether the callee only reads the value or keeps it (stores, moves, or copies it beyond the call), the proposed treatment (const&, or by value plus std::move), and coupled virtual overrides or callers. Example of a delicate case: the DFA path from gecode/int/extensional-regular.cpp through OpenRegular::post into its constructor, which uses dfa0 both to initialize the stored DFA and to compute distances, so an early move would be wrong. Trace stored callbacks through SharedData and its SDO (gecode/kernel/data/shared-data.hpp), which today take only `const Data&` and copy into storage, and record whether each receiving constructor actually moves. Group the parameters so that each group keeps a signature, its definitions, its override set, and the caller changes it forces together (for example: search callbacks and options, branch configurations, shared arrays and sets in int, set and float posting functions). Do not edit .clang-tidy yet.

## Boundaries

- Read-only for source. Commit only the background file and the brief index.

## Done when

- [x] .zdev/value-params/background/value-param-inventory.md records the command, the --config used, the AllowedTypes regex, one row per unique parameter with the fields listed in Context, and the repair groups, and the brief indexes it.

## Validation

- The recorded command reproduces the recorded counts.

## Result

Inventory of performance-unnecessary-value-param findings: 834 unique diagnostics on 708 parameters with the brief-conforming AllowedTypes (FloatVal as a separate exception: 776/655), each row with declarations, use, treatment and coupling, grouped G1-G16 with order

Validation:

- clang-tidy 22 over build/full with misc/check.py lint coverage (parser and lexer action passes): no AllowedTypes 172584 raw / 2016 unique; default 117975 / 834; with FloatVal 113879 / 776
- Appendix scripts extracted from the document reproduce every table byte-identically (replayed by the implementer and two verifiers)
- Prescribed DomArgs change passes a syntax-only compile
- Independent Codex verifier: pass after two rework rounds
