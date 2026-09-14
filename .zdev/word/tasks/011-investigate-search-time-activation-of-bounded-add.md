+++
schema_version = 1
id = "word-011"
key = "investigate-adaptive-bounded-add"
area = "word"
status = "done"
complexity = "advanced"
blocked_by = ["word-002", "word-004"]
+++
# Investigate search-time activation of bounded Add

## Outcome

A measured binary Add experiment determines whether a bounded Word actor should observe later range information and activate numeric closure after posting.

## Context

Use the exact 64-link chain in .zdev/word/brief.md: full-range width-12 unsigned states, constants repeating 1,2,3,4, every Add posted before x0 is restricted to [100,200] and x64 to [260,360], then lower-ranked branching over all states. The chain has 101 solutions. Controls are the exact symbolic-ALU cube and unsigned invocations, default word-arx-preimage, and two-step 12-unknown-bit word-md5-preimage invocations in the brief.

## Boundaries

- Keep compact Word Add unchanged.
- While unclassified, retain the existing exact cube closure and use a cheap bounded subscription or staged dispatcher.
- Rewrite or specialize only after the numeric regime becomes invariantly suitable.
- Retain production code only on repeatable search or runtime value without a material regression in any named control; otherwise record the rejection and remove the prototype.

## Done when

- [x] Posting the chain before versus after its endpoint restrictions demonstrates whether adaptive activation removes the posting-order propagation gap.
- [x] Aliases, costs, subscriptions, cloning, recomputation, exact cube behavior, 101 solutions, and a deterministic chain checksum are validated.
- [x] Twenty interleaved Release trials report solutions, nodes, failures, propagations, and runtime for the chain and every exact checked-in control invocation from the brief.
- [x] The retained result is either a justified adaptive Add implementation or a clean rejection with no prototype code left.

## Validation

- Run the bounded arithmetic oracle and focused Add lifecycle tests.
- Run the exact chain and four checked-in control invocations from the brief against linked baseline and candidate binaries.
- Run the broader Word arithmetic suite and TestFramework if code is retained.

## Result

Rejected adaptive bounded Add: it closed the posting-order propagation gap but increased target-chain work and materially regressed unsigned symbolic ALU, so no prototype code was retained.

Validation:

- Independent isolated reconstruction passed focused arithmetic oracle/lifecycle, aliases, cloning, recomputation, replay, and preserved the exact chain's 101 solutions and checksum 55338910.
- Twenty interleaved controls showed no chain search benefit, propagations rising from 6,927 to 15,817, and a material unsigned-ALU node/failure/propagation/runtime regression; cube ALU, ARX, and MD5 stayed stable and the source tree remained clean.
