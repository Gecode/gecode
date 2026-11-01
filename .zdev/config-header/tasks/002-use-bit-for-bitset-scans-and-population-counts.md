+++
schema_version = 1
id = "config-header-002"
key = "bit-ops"
area = "config-header"
status = "done"
afk = true
blocked_by = ["config-header-001"]
+++
# Use <bit> for bitset scans and population counts

## Outcome

BitSetData::next() and BitSetData::ones() in gecode/support/bitset-base.hpp use std::countr_zero and std::popcount instead of __builtin_ffsll, __builtin_popcountll, and the MSVC _BitScanForward/__popcnt intrinsics. The GECODE_HAS_BUILTIN_FFSLL and GECODE_HAS_BUILTIN_POPCOUNTLL probes and macros are removed.

## Context

The intrinsics and their #pragma intrinsic lines are in gecode/support/bitset-base.hpp, in BitSetData::next() (scan for the next set bit) and BitSetData::ones() (population count). The word type Base is 32 bits under 32-bit MSVC and 64 bits elsewhere. The probes are check_c_source_compiles calls near the end of the config probes in CMakeLists.txt. Search the tree for any other use of the two macros before removing them. The C++23 baseline guarantees <bit>, and AGENTS.md allows standard library facilities in the core library.

## Boundaries

- Keep the BitSetData interface, the Base widths, and the documented precondition of next() that a matching set bit exists.

## Done when

- [x] No GECODE_HAS_BUILTIN_* macro, compiler bit builtin, or MSVC bit intrinsic remains in gecode/ outside gecode/third-party.
- [x] The Release disassembly of one next() and one ones() instantiation on the local AArch64 target is recorded in the task result. It shows the hardware scan and population-count sequences (for example rbit+clz, and cnt with its reduction) with no software loop or helper call.

## Validation

- Build the default configuration and run ctest.
- Run the set and int integrity tests that use bitsets, for example `bin/gecode-test -test Set::` and `-test Int::Dom`.
- Inspect the disassembly with objdump or `-S` output.

## Result

BitSetData::next() and ones() use std::countr_zero and std::popcount; MSVC intrinsics, GCC builtins, software fallbacks and the GECODE_HAS_BUILTIN_* probes and macros are removed

Validation:

- Release AArch64 disassembly: next() = lsr; rbit; clz; add; ret. ones() = cnt.8b; addv.8b; fmov; ret. No loop, branch or call
- Default Release build (0 warnings) and ctest 7/7; Set:: 1220/1220 and Int::Dom 18/18
- Debug with audit: Int::Extensional 245/245, Int::BinPacking 37/37, Set::Rel 854/854
- No GECODE_HAS_BUILTIN_*, bit builtins or MSVC bit intrinsics remain outside gecode/third-party; tidy and format-check passed
- Independent Codex verifier: pass, no findings; MSVC branch reviewed by hand only
