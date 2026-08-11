+++
schema_version = 1
id = "word-007"
key = "branch-engineering"
area = "word"
status = "open"
blocked_by = ["word-006"]
+++
# Add standard word selectors and tracing

## Outcome

Word branching participates in applicable size, degree, AFC, action, and CHB selection patterns, and trace consumers receive word deltas for newly fixed zero and one bits.

## Boundaries

- Do not add a new search engine or make GecodeWord depend on Qt.
- Add Gist inspection only through an existing non-Qt variable-inspector extension point.

## Done when

- [ ] Applicable standard variable selectors and merit state work with WordVar branch and assign posting.
- [ ] AFC, action, and CHB state survives the required cloning, disposal, and recomputation paths.
- [ ] Word trace deltas distinguish newly fixed zero and one masks and integrate with ordinary trace filtering.

## Validation

- Run focused selector, merit, AFC, action, CHB, and trace tests.
- Run mixed WordVar/BoolVar search under cloning and recomputation.
