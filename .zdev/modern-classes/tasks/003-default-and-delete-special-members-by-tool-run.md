+++
schema_version = 1
id = "modern-classes-003"
key = "equals-default"
area = "modern-classes"
status = "done"
complexity = "standard"
afk = true
priority = "normal"
blocked_by = ["modern-classes-002"]
+++
# Default and delete special members by tool run

## Outcome

Special members that only copy, move, or destroy members are defaulted, deleted members use = delete, and modernize-use-equals-default and modernize-use-equals-delete are enabled.

## Context

Apply the inventory's treatment for the two checks: run clang-tidy 22 with -checks=-*,modernize-use-equals-default,modernize-use-equals-delete --fix over the owned translation units with the lint header filter and the options the inventory proposes, change generated class code through misc/genvarimp.py, then run uv run misc/check.py format. Cover maintained headers that no translation unit includes, and record how. Apply the inventory's manual candidates (move members, private undefined members, out-of-line defaults) by hand or by a recorded script, in the same commit. FloatVal's hand-written members (gecode/float/val.hpp:44, 55, 64) are defaulted like any other class. Review each fix where the inventory flagged a behaviour risk. Enable both checks in .clang-tidy, state the rule (rule of zero; = default over empty bodies; = delete over private undefined members) in docs/code-style.md, and add a changelog entry if any public class changes triviality or its special members. Make the mass edit one commit whose message gives the full command sequence.

## Boundaries

- Do not hand-merge the mass edit; rerun the tool.
- Keep hand-written members the inventory marks as behaviour-changing.

## Done when

- [x] clang-tidy with both checks reports nothing over the lint configuration, both checks are enabled in .clang-tidy, and every manual candidate in the inventory is applied or kept with the reason in the result.
- [x] The commit message gives the complete command sequence, and rerunning it on the parent commit reproduces the commit.
- [x] docs/code-style.md states the rule.

## Validation

- Build target regenerate, then .github/ci/generated-source-freshness.sh.
- uv run misc/check.py lint --build-dir build/full and check-standard in build/full.
- uv run misc/check.py all --build-dir build/full --jobs N before the commit.

## Result

Enabled modernize-use-equals-default and modernize-use-equals-delete. Special members were defaulted and deleted by a reproducible clang-tidy 22 --fix run plus recorded scripts in .zdev/modern-classes/background/equals-default/: genvarimp.py change and regenerate, NOLINT for 68 kept members (26 cloning constructors, a configuration-dependent constructor, 41 default constructors of value-initialized types), relocation of 194 out-of-line defaults in-class, and a manual patch (private undefined members made public = delete, deleted operator new/delete, Heap's PEAKHEAP counters as default member initializers, .clang-tidy, docs/code-style.md, changelog including the const default-initialization source break).

Validation:

- Independent verifier: full review found two issues (Heap PEAKHEAP zeroing lost; changelog missed the const default-init break), fixed in one correction pass, focused recheck PASS. Evidence: reproduce.sh on a fresh git archive of 0ecc0550d2 gives an identical tree; regenerate + generated-source freshness passed; build/full and build/local (Release, experimental OFF) incl. gecode-test with 0 warnings; lint clean; check-standard 6820/6820; focused tests after the fix 26065/26065; check.py all rc=0.
