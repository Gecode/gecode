+++
schema_version = 1
id = "boost-004"
key = "decide-approach"
area = "boost"
status = "done"
priority = "normal"
blocked_by = ["boost-003", "boost-007"]
+++
# Decide the interval-arithmetic approach

## Outcome

The user has chosen one approach. The brief records the decision and moves it from Open questions to Terms and settled decisions.

## Context

Present the recommendation in background/approach-study.md the measurements in background/nextafter-study.md, and the native spike in background/native-spike.md with its limitations and wider impacts to the user and ask for a decision. This task is the attended gate before the implementation task.

## Done when

- [x] The brief names the chosen approach, the user's reasons, and any accepted loss of tightness.

## Validation

- zdev check boost

## Result

User chose the native spike (approach 3, no Boost code, switched-mode rounding) with no accepted loss of tightness; interval code is Gecode/MIT, written in Gecode style with Boost.Interval as the bit-for-bit oracle; CI runs once local validation is complete, via a pull request confirmed with the user. Recorded in the brief's Terms and settled decisions.

Validation:

- zdev check boost
