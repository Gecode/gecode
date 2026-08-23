+++
schema_version = 1
id = "word-001"
key = "complete-word-branching-contracts"
area = "word"
status = "open"
complexity = "standard"
blocked_by = []
+++
# Complete Word branching contracts

## Outcome

Ranked Word branching accepts arrays with irrelevant assigned compact variables, and the public custom value and commit callback seam preserves full-width WordValue payloads.

## Context

The ranked branch and assign posting checks in gecode/word/branch.cpp reject every non-bounded array member, including assigned compact values that the brancher will skip. gecode/word.hh exposes WordBranchVal and WordBranchCommit, but no callback selector is constructible, and gecode/word/branch/traits.hpp declares the callback payload as unsigned int rather than WordValue. Follow the established Int, Set, and Float callback brancher pattern.

## Boundaries

- Skip the boundedness check only for assigned members; ranked branching must still reject unassigned compact variables.
- Preserve all built-in Word selectors, archive behavior, ranked no-goods, and the established limitation that generic custom commits do not provide generic no-good literals.
- Do not redesign the generic branching framework.

## Done when

- [ ] Rank split and ranked assignment work on arrays containing assigned compact WordVars and unassigned bounded WordVars.
- [ ] A public Word callback selector and commit path is constructible through the normal Gecode factory pattern.
- [ ] BranchTraits uses WordValue and a width-64 callback test proves that upper payload bits are not truncated.
- [ ] The public branching documentation states the callback and no-good contracts.

## Validation

- Run the focused Word branching tests, including archive, no-good, clone, and recomputation cases.
- Run TestFramework.
