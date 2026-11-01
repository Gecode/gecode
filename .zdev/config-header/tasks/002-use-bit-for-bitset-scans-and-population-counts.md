+++
schema_version = 1
id = "config-header-002"
key = "bit-ops"
area = "config-header"
status = "open"
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

- [ ] No GECODE_HAS_BUILTIN_* macro, compiler bit builtin, or MSVC bit intrinsic remains in gecode/ outside gecode/third-party.
- [ ] The Release disassembly of one next() and one ones() instantiation on the local AArch64 target is recorded in the task result. It shows the hardware scan and population-count sequences (for example rbit+clz, and cnt with its reduction) with no software loop or helper call.

## Validation

- Build the default configuration and run ctest.
- Run the set and int integrity tests that use bitsets, for example `bin/gecode-test -test Set::` and `-test Int::Dom`.
- Inspect the disassembly with objdump or `-S` output.
