+++
schema_version = 1
id = "word-063"
key = "word-public-docs-release"
area = "word"
status = "done"
blocked_by = ["word-059", "word-060", "word-061", "word-062"]
+++
# Refresh Word diagnostics and release documentation

## Outcome

Public diagnostics, propagation inventory, and release notes accurately describe the completed Word module.

## Context

The Doxygen inventory still describes multiplication as decomposed and omits reductions, carry/borrow, overflow, and combined divmod. OutOfLimits misleadingly mentions only width/mask despite covering other Word arguments. The changelog has no Word module classification or release entry.

## Boundaries

- Do not add new APIs or alter arithmetic semantics.
- Keep the existing OutOfLimits exception type.
- Follow the established changelog format and module vocabulary.

## Done when

- [x] The public propagation inventory reflects current native actor shapes and all later public operation families.
- [x] OutOfLimits reports a correct general Word-argument diagnostic and its focused test checks the message.
- [x] The changelog recognizes the word module and contains a major new-module entry summarizing WordVar, MiniModel, search, core operations, and SMT-LIB edge semantics.
- [x] Generated documentation inputs remain valid.

## Validation

- Build the affected Word and documentation-facing targets.
- Run Word::TestFramework and relevant focused tests.
- Run changelog/documentation validation available in the repository.
- Run zd check word and git diff --check.

## Result

Refreshed Word diagnostics, public propagation inventory, and current-release changelog integration.

Validation:

- Release gecodeword_shared and gecode-test built; Word::TestFramework passed.
- Changelog tidy validation and Doxygen/current/plain generators passed with Word vectors output.
- CMake doc and VPATH Make changelog targets passed.
- Independent Spec and Standards verification, zd check word, and git diff --check passed.
