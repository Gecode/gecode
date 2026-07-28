# CPKB evidence for Origin and Variant terminology

This file records the small part of the local constraint-programming knowledge
base used by the Origin/Variant terminology research. The source paths remain
the authoritative place for their surrounding context.

## Search spaces are architectural objects

Source:
`/Users/zayenz/research/cp-kb/wiki/topics/search/search-architecture.md`

Source page review date: 2026-04-08.

Exact excerpt:

> These papers treat search as part of system design rather than only as a
> branching policy. The central question is which abstractions make search
> programmable, inspectable, parallelisable, and compatible with mutable
> solver state. In the Oz line the answer is computation spaces and
> first-class search services. In the later Gecode-facing line the answer is a
> tighter account of restoration, decomposition, and runtime control inside a
> propagation solver.

Exact excerpt:

> Search nodes as ephemeral states versus search nodes as first-class objects
> that can remain available for exploration, cloning, and parallel work.

## Search concerns can be factored

Source:
`/Users/zayenz/research/cp-kb/wiki/sources/2004-tack-compositional-abstractions-for-search-factories.md`

Source page review date: 2026-04-07.

Exact excerpt:

> Its starting point is that real search engines combine several concerns at
> once: restoration, best-solution search, parallelism, and visualisation. In
> Mozart those combinations had become monolithic and tied to one exploration
> strategy. The paper's answer is to factor search into orthogonal abstractions
> that can be recombined into a search engine that fits the user's needs.

## Variant is established portfolio language

Source:
`/Users/zayenz/research/cp-kb/wiki/sources/2026-lewander1-automatic-relaxation-and-multi-armed-bandit-lear.md`

Source page review date: 2026-07-20. The source page is marked draft, so this
excerpt supports terminology usage rather than a settled architectural claim.

Exact excerpt:

> We tested a portfolio of CP-based LNS variants running in parallel, with a
> multi-armed bandit to select which LNS variant to run.
