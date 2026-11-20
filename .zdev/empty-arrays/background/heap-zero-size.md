# Zero-size heap requests and null-pointer library calls

This record lists every zero-size heap request, zero-size reallocation, and
null pointer passed to `memcpy`, `memmove`, or `memset` that three searches
found: a bounded static audit, the `check-standard` suite run with a
scratch heap check, and a UBSan `check-standard` run. For each site it gives
the call path, a minimal public input, the existing test that reaches it,
the kind of evidence, the proposed fix, and the task that owns the fix.

Baseline: commit 61728ea4fe on feature/breaking-7, checked on 2026-11-03.
The heap check ran on macOS 27.0.1 (arm64, Apple clang 21.0.0). The UBSan
run used Linux on erebor (Ubuntu, x86_64, glibc, Ubuntu clang 21.1.8). No
tracked source changed. The patch, builds, probes, and logs are under
build/heap-audit in the checkout and ~/heap-audit on erebor.

Each fix follows the brief: no code path makes a zero-size request. A site
skips the allocation or the copy when the count is zero, keeps a null
pointer for an empty buffer, and frees instead of reallocating to zero.
Documented results stay as they are. Task 009 (empty Action and CHB
objects) owns the Action and CHB element buffers; task 010 (the remaining
sites) owns every other site.

## Sites

| # | Site | Request | Evidence | Existing test | Owner |
|---|---|---|---|---|---|
| 1 | Action::Storage, action.hpp:274 | `ralloc(0)` | heap check | none | 009 |
| 2 | CHB::Storage, chb.hpp:258 | `ralloc(0)` | heap check | none | 009 |
| 3 | TupleSet::Data::Data, tuple-set.hpp:65 | two `ralloc(0)` | heap check | Int::Extensional::TupleSet::Sparse::Nullary::{+,-} | 010 |
| 4 | TupleSet::Data::finalize, tuple-set.cpp:259, 352, 367 | `ralloc(0)` each | heap check | Int::Extensional::TupleSet::Sparse::Nullary::+ (259, 352) | 010 |
| 5 | TupleSet::Data::resize, tuple-set.cpp:555 | `rrealloc(p, 0)` | heap check | none | 010 |
| 6 | Archive copy constructor, archive.cpp:45-46 | `ralloc(0)`, `memcpy(d, nullptr, 0)` | heap check, UBSan | none | 010 |
| 7 | Archive assignment, archive.cpp:52-53 | `rrealloc(nullptr, 0)`, `memcpy(nullptr, nullptr, 0)` | heap check, UBSan | none | 010 |
| 8 | FlatZinc vs2is, flatzinc.cpp:367 | `ralloc(0)` | heap check | none | 010 |
| 9 | FlatZinc new_set_var (assigned), flatzinc.cpp:968 | `ralloc(0)` | heap check | none | 010 |
| 10 | FlatZinc new_set_var (upper bound), flatzinc.cpp:981 | `ralloc(0)` | heap check | none | 010 |
| 11 | FlatZinc p_regular, registry.cpp:1273 | `ralloc(0)`, then a write before the block | heap check | none | 010 |
| 12 | Dictionary::init (examples), scowl.hpp:7823 | `ralloc(0)` | heap check | none | 010 |
| 13 | Space::rrealloc, core.hpp:3007 | `memcpy(p, nullptr, 0)` | UBSan | none | 010 |
| 14 | DFA::init, dfa.cpp:198 | Region request of 0 bytes | Region source trace | Int::Extensional::Reg::Empty::REG | 010 |
| 15 | TupleSet(int, const DFA&), tuple-set.cpp:636-637 | two Region requests of 0 bytes | Region source trace | Int::Extensional::TupleSet::{Dense,DenseCompressed,Sparse}::{+,-}::Empty (sweep) | 010 |
| 16 | IntSet from an empty initializer list, int-set.cpp:202, 215 | Debug assertion; Release Region request of 0 bytes | Region source trace | none | 010 |

The suite run with the heap check found only sites 3 and 4, and the UBSan
suite run found nothing. Every other site comes from the static audit and
was confirmed with a scratch probe or a patched example binary (see
Runtime runs).

### 1. Action element buffer

- Location: `Action::Storage::Storage` (gecode/kernel/branch/action.hpp:274),
  `a(heap.alloc<double>(x.size()))`, freed in gecode/kernel/branch/action.cpp:40.
- Call path: `IntAction(home, x)` → `Action::init` → `Storage(home, x, d, bm)`
  → `Heap::alloc<double>(0)` → `Heap::ralloc(0)`. BoolAction, SetAction,
  FloatAction, and branching with an ACTION merit over an empty array take
  the same path.
- Minimal input: `IntAction a(home, IntVarArgs());`
- Existing test: none. The heap-check suite run logged no request from
  `Action::Storage`.
- Fix: as the brief's Terms settle it, keep the shared Storage object but
  leave `a` null when `n == 0`, free it only when `n > 0`, and post no
  recorder.

### 2. CHB element buffer

- Location: `CHB::Storage::Storage` (gecode/kernel/branch/chb.hpp:258),
  `chb(heap.alloc<Info>(x.size()))`, freed in gecode/kernel/branch/chb.cpp:40.
- Call path: `IntCHB(home, x)` → `CHB::init` → `Storage(home, x, bm)` →
  `Heap::alloc<Info>(0)` → `Heap::ralloc(0)`. The other CHB classes and
  branching with a CHB merit take the same path.
- Minimal input: `IntCHB c(home, IntVarArgs());`
- Existing test: none, as for Action.
- Fix: as for Action.

### 3. TupleSet data for arity zero

- Location: `TupleSet::Data::Data` (gecode/int/extensional/tuple-set.hpp:65):
  `td(heap.alloc<int>(n_initial_free * a))` and `vd(heap.alloc<ValueData>(a))`.
  `~Data` frees both (tuple-set.cpp:561-562).
- Call path: `TupleSet(0)` → `new Data(0)` → two `Heap::ralloc(0)`.
  `TupleSet(0, dfa)` constructs the same `Data(0)` (tuple-set.cpp:609).
- Minimal input: `TupleSet t(0);`
- Existing test: Int::Extensional::TupleSet::Sparse::Nullary::+ and ::-
  (class `SparseTupleSetNullary`) build `TupleSet(0)` in their constructor,
  which runs during static initialization of gecode-test, so every
  gecode-test run reaches this site.
- Fix: leave `td` and `vd` null when the arity is zero, and free each only
  when it is non-null. `heap.rfree` is not safe on null: under
  `GECODE_PEAKHEAP` it measures the block first.

### 4. TupleSet finalization for arity zero

- Location: `TupleSet::Data::finalize` (gecode/int/extensional/tuple-set.cpp):
  line 259 `heap.alloc<int>(n_tcells)`, line 352 `heap.alloc<Range>(n_ranges)`,
  and line 367 `heap.alloc<BitSetData>(n_support_entries_u32)` for the dense
  representation. With arity zero and at least one tuple, `n_tcells`,
  `n_ranges`, and `n_support_entries_u32` are all zero. With no tuples the
  function returns at line 216 before any of them.
- Call path: `TupleSet::finalize()` → `Data::finalize(epk)` → `Heap::alloc`
  with zero.
- Minimal input: `TupleSet t(0); t.add(IntArgs()); t.finalize();` (dense
  default representation reaches all three lines).
- Existing test: Int::Extensional::TupleSet::Sparse::Nullary::+ reaches lines 259
  and 352 (its sparse representation does not build the dense support).
  No test reaches line 367.
- Fix: skip each allocation when its count is zero and leave the pointer
  null, as lines 419, 437, 460, and 508 already do.

### 5. TupleSet growth for arity zero

- Location: `TupleSet::Data::resize` (gecode/int/extensional/tuple-set.cpp:555),
  `heap.realloc<int>(td, n_tuples * arity, n * arity)`. The `int`
  specialization of `Heap::realloc` forwards `m * sizeof(int)` to
  `Heap::rrealloc` without looking at `n`, so arity zero gives
  `rrealloc(td, 0)`.
- Call path: `TupleSet::add` → `_add` → `Data::resize` once the 1024
  initial free slots are used.
- Minimal input: `TupleSet t(0);` then `t.add(IntArgs())` 1025 times.
- Existing test: none.
- Fix: with site 3's fix `td` stays null for arity zero; `resize` then only
  updates `n_free` and requests nothing when `arity == 0`.

### 6. Archive copy constructor

- Location: `Archive::Archive(const Archive&)` (gecode/kernel/archive.cpp:45-46):
  `_a = heap.alloc<unsigned int>(_n)` and
  `heap.copy<unsigned int>(_a, e._a, _n)`.
- Call path: copying an Archive with `_n == 0` → `Heap::ralloc(0)`, then the
  `unsigned int` specialization of `Heap::copy` → `memcpy(_a, nullptr, 0)`
  when the source was never written (its `_a` is null).
- Minimal input: `Archive a; Archive b(a);`
- Existing test: none. No library code copies an Archive; only the public
  copy constructor reaches it.
- Fix: when `e._n == 0`, set `_a` to null and `_size` to zero and copy
  nothing.

### 7. Archive assignment

- Location: `Archive::operator=` (gecode/kernel/archive.cpp:52-53):
  `_a = heap.realloc<unsigned int>(_a, _size, e._n)` and
  `heap.copy<unsigned int>(_a, e._a, e._n)`.
- Call path: assigning an empty Archive → `Heap::rrealloc(_a, 0)` (with
  `_a` null when the target is also empty), then `memcpy(_a, e._a, 0)` with
  null pointers.
- Minimal input: `Archive a; Archive b; b = a;`
- Existing test: none, as for site 6.
- Fix: when `e._n == 0`, free the buffer (if any), set `_a` to null and
  `_size` and `_n` to zero, and copy nothing.

### 8. FlatZinc integer variable with an empty set domain

- Location: `vs2is` (gecode/flatzinc/flatzinc.cpp:367),
  `heap.alloc<int>(sl->s.size())`.
- Call path: `initialize_space` → `FlatZincSpace::new_int_var` → `vs2is` for
  a domain written as a set literal with no elements.
- Minimal input (FlatZinc): `var {}: x :: output_var; solve satisfy;`
  (prints `=====UNSATISFIABLE=====`).
- Existing test: none.
- Fix: build the IntSet without the heap buffer, for example with
  `IntSet(const std::vector<int>&)`, which returns the empty set for an
  empty vector (int-set-1.hpp:118).

### 9. FlatZinc set variable assigned the empty set

- Location: `FlatZincSpace::new_set_var` (gecode/flatzinc/flatzinc.cpp:968),
  `heap.alloc<int>(vsv->s.size())` for an assigned set variable.
- Minimal input: `var set of 1..3: s :: output_var = {}; solve satisfy;`
- Existing test: none.
- Fix: as for site 8.

### 10. FlatZinc set variable with an empty upper bound

- Location: `FlatZincSpace::new_set_var` (gecode/flatzinc/flatzinc.cpp:981),
  `heap.alloc<int>(vsv->s.size())` for the upper bound.
- Minimal input: `var set of {}: s :: output_var; solve satisfy;`
- Existing test: none.
- Fix: as for site 8.

### 11. FlatZinc regular with a reversed final-state range

- Location: `p_regular` (gecode/flatzinc/registry.cpp:1273),
  `heap.ralloc(sizeof(int) * (finals->max - finals->min + 2))`, then
  `f[finals->max - finals->min + 1] = -1`.
- Call path: `gecode_regular` or `gecode_regular_set` → `p_regular`. When
  the final states are an interval with `max == min - 2`, such as `3..1`,
  the request is zero bytes and the terminator is written to `f[-1]`,
  before the block. In the patched Debug build the probe below exits with
  SIGBUS (status 138). For `max < min - 2` the size is negative and wraps
  to a huge `size_t`; `1..0` works.
- Minimal input:
  `var 1..1: x :: output_var; constraint gecode_regular([x], 1, 1, [1], 1, 3..1); solve satisfy;`
- Existing test: none.
- Fix: treat any interval with `max < min` as empty, so the buffer holds
  only the terminator. The same function releases the buffer with
  `::free(f)` (registry.cpp:1285) although `heap.ralloc` allocated it; that
  breaks with the mimalloc backend and under `GECODE_PEAKHEAP`, and should
  become `heap.rfree(f)` in the same change.

### 12. Example dictionary file with no word

- Location: `Dictionary::init` (examples/scowl.hpp:7823),
  `chunk = heap.ralloc(sz)` for a dictionary read from a file. `~Dictionary`
  frees it with `heap.rfree(chunk)` (scowl.hpp:7903).
- Call path: the crossword and word-square examples with `-file <dict>` →
  `main` → `dict.init(opt.file())` → `Heap::ralloc(0)`. The first pass adds
  `n + 1` bytes for each line that is a lowercase word shorter than 64. The
  last `getline` yields an empty line, counted as one byte, only when the
  file is empty or ends with a newline. When the last line has no newline,
  the `getline` that reads it sets end of file and the loop stops, so a file
  with no accepted line and no final newline gives `sz == 0`.
- Minimal input: a dictionary file holding exactly `123`, with no final
  newline: `printf '123' > dict.txt; word-square -file dict.txt`.
- Existing test: none. No test or CI job runs the examples with `-file`.
- Fix: skip the allocation when `sz == 0` and leave `chunk` null (the
  start-pointer loops then only add zero to it), and free `chunk` in
  `~Dictionary` only when it is non-null.

### 13. Space::rrealloc with an empty old block

- Location: `Space::rrealloc` (gecode/kernel/core.hpp:3007), `memcpy(p, b, n)`
  in the growing branch.
- Call path: the scalar specializations of `Space::realloc`
  (`GECODE_KERNEL_REALLOC`, core.hpp:3116) and the pointer overload
  (core.hpp:3153) forward to `rrealloc`. Growing from an old count of zero
  with a null block calls `memcpy(p, nullptr, 0)`. No library code calls
  these overloads: the two library uses of `Space::realloc` (kernel/core.cpp:166
  and set/ldsb/brancher.hpp:127) name a pointer element type explicitly and
  take the generic element-wise template. Only model or propagator code
  calling the public function reaches the site.
- Minimal input: `int* p = home.realloc<int>(nullptr, 0, 4);`
- Existing test: none.
- Fix: copy only when `n > 0`.

Region requests (sites 14 to 16) never reach the heap: a zero-size request
always fits the current Region chunk (`Region::Chunk::alloc`,
region.hpp:334-343), so the heap fallback `Region::heap_alloc` only sees
positive sizes. They cannot fail a test; the brief asks for their before and
after sizes from a source trace.

### 14. DFA without transitions

- Location: `DFA::init` (gecode/int/extensional/dfa.cpp:198),
  `region.alloc<Transition>(n_trans)`.
- Input: `DFA d(0, {}, {0});` or any DFA specification whose transition
  list is empty.
- Existing test: Int::Extensional::Reg::Empty::REG. Its empty `REG`
  becomes a DFA with one state and no transitions; lldb stopped at
  dfa.cpp:198 with `n_trans = 0` and `n_states = 1`.
- Before: one Region request of `0 * sizeof(Transition)` = 0 bytes. All
  other requests in `DFA::init` count `n_states` (at least one, the start
  state) or add one. The heap side is already safe: `DFAI(int nt)` keeps
  `trans` null for `nt == 0` (dfa.hpp:82), and the symbol table has at
  least two entries.
- After: no request; `trans` stays null and the loops over `n_trans` do not
  run.

### 15. TupleSet from a DFA without transitions

- Location: `TupleSet::TupleSet(int a, const DFA& dfa, ExtensionalPropKind)`
  (gecode/int/extensional/tuple-set.cpp:636-637):
  `r.alloc<Edge>(dfa.max_degree())` and `r.alloc<Support>(dfa.n_symbols())`.
- Input: `DFA d(0, {}, {0}); TupleSet t(1, d);` A scratch probe printed
  `max_degree=0 n_symbols=0` for this DFA and gave an empty tuple set.
- Existing test: the sweep-tagged
  Int::Extensional::TupleSet::{Dense,DenseCompressed,Sparse}::{+,-}::Empty.
  `TupleSetTest` rebuilds its empty arity-4 set as
  `TupleSet(ts.arity(), tuple_set_to_dfa(ts), epk)` (test/int/extensional.cpp:1219);
  lldb stopped at tuple-set.cpp:636 in Dense::+::Empty with
  `max_degree = 0`, `n_symbols = 0`, and `n_trans = 0`. No standard-tagged
  test reaches the site.
- Before: two Region requests of 0 bytes (`0 * sizeof(Edge)`,
  `0 * sizeof(Support)`).
- After: no request when the count is zero; the forward pass then finds no
  support and finalizes the empty set as today.

### 16. IntSet from an empty initializer list

- Location: `IntSet(std::initializer_list<int>)` and
  `IntSet(std::initializer_list<std::pair<int, int>>)`
  (gecode/int/int-set.cpp:200-224) assert `n > 0` at lines 202 and 215.
- Input: `IntSet s({});` with either element type.
- Existing test: none.
- Before: Debug builds abort on the assertion. Release builds make a Region
  request of `0 * sizeof(Range)` = 0 bytes, and `normalize` then does
  nothing (it allocates only for `n > 0`), so the result is the empty set.
  No heap request either way.
- After: return the empty set for `n == 0` with no Region request, as the
  array forms `IntSet(const int[], int)` and `IntSet(const int[][2], int)`
  already do (int-set-1.hpp:105-114).

### Noticed outside the record's classes

- `Space::realloc(T** b, n, m)` (gecode/kernel/core.hpp:3153) passes
  `n * sizeof(T)` as the old size where `n * sizeof(T*)` is meant, so it
  copies and frees the wrong number of bytes whenever
  `sizeof(T) != sizeof(T*)`. No library code calls it (see site 13). Task
  010 can fix it while touching `rrealloc`.

## Static audit

### Scope

The audit covered every caller, outside gecode/third-party, of:

- `Heap::alloc`, `realloc`, `copy`, `ralloc`, and `rrealloc` (gecode/support/heap.hpp),
  whether called through `heap.`, `Gecode::heap.`, or `Heap::`;
- the allocator-parameterized containers in gecode/support (`DynamicArray`,
  `DynamicStack`, `DynamicQueue`, `StaticStack`, `RawBitSetBase` and
  `BitSetBase`, `BitSet`, `BlockAllocator`) and every instance of them
  backed by `Heap`;
- `Support::allocator.alloc`, `realloc`, and `memcpy` (gecode/support/allocator.hpp
  and allocator.cpp);
- `HeapAllocated` and class-level `operator new`;
- the SharedArray, ViewArray, ArgArrayBase, and IntSet constructors,
  Region's heap fallback, and MemoryManager's heap chunks;
- every direct call of `memcpy`, `memmove`, `memset`, `malloc`, `calloc`,
  or `realloc`.

Directories: gecode (all modules, including flatzinc, gist, driver, and
instance), tools, test, and examples. Excluded: gecode/third-party, and the
definitions in gecode/support/heap.hpp themselves. The flex-generated
gecode/flatzinc/lexer.yy.cpp and bison-generated parser.tab.cpp are in
scope and were checked; they are generated, so a fix would go in the
generator inputs.

The greps match calls on one line. A call split between `heap.` and the
member name across lines would be missed; reading the hit files and the
named constructors found none.

### Commands

Run from the checkout root at 61728ea4fe. git grep uses `-P` because its
default `-E` on macOS does not understand `\b` and `\s` and silently
returns nothing.

```sh
# (a) Heap wrapper callers: 97 hits
git grep -nP '\b(Gecode::)?heap\s*\.\s*(template\s+)?(alloc|realloc|ralloc|rrealloc|copy)\b|Heap::(alloc|realloc|copy)\b' -- gecode tools test examples ':!gecode/third-party' ':!gecode/support/heap.hpp'
# (b1) Containers taking the allocator as a parameter: 17 hits
git grep -nP '\ba\s*\.\s*(template\s+)?(alloc|realloc|ralloc|rrealloc|copy)\b|A::copy' -- gecode/support gecode/kernel
# (b2) Heap-backed container instances: 37 hits, plus 2 with a size argument
git grep -nP '(DynamicArray|DynamicStack|DynamicQueue|StaticStack|BitSetBase|BitSet|BlockAllocator)\s*<[^;]*\bHeap\b|\(heap\)' -- gecode tools test examples ':!gecode/third-party'
git grep -nP '\(\s*(Gecode::)?heap\s*,' -- gecode tools test examples ':!gecode/third-party'
# (c) Support::allocator: 6 hits, all in heap.hpp
git grep -nP 'Support::allocator|allocator\s*\.\s*(alloc|realloc|memcpy)\b' -- gecode tools test examples ':!gecode/third-party'
# (d) Fixed-size requests: 93 hits
git grep -nP 'HeapAllocated|operator new\s*\(\s*size_t' -- gecode ':!gecode/third-party'
# (f) Direct C library calls: 67 hits
git grep -nP '(^|[^A-Za-z0-9_.>])(std::|::)?(memcpy|memmove|memset|malloc|calloc|realloc)\s*\(' -- gecode tools test examples ':!gecode/third-party'
# Space::realloc and rrealloc callers (for site 13)
git grep -nP 'realloc\s*<' -- gecode ':!gecode/third-party' ':!gecode/support/heap.hpp' ':!gecode/kernel/core.hpp'
```

### Verdicts

"Positive" means the request cannot be zero; the reason follows. "Site n"
refers to the list above. Three verdicts depend on the contents of an input
file: scowl.hpp:7823 (site 12), the instance reader's `Seq` (impl.hpp:139,
capacity at least 2 whatever the file holds), and the FlatZinc lexer's
`fill_buffer` (parser.hh:252, which copies only when bytes remain).

(a) Heap wrapper callers:

| Location | Verdict |
|---|---|
| examples/bin-packing.cpp:242 | positive: `n_s` is 1 or `n_same`, which starts at 1 (line 323) |
| examples/scowl.hpp:7773 | positive: the built-in word lists add `l + 1` per word |
| examples/scowl.hpp:7823 | site 12 |
| gecode/driver/options.cpp:59, 69 | positive: `strlen(s) + 1` and `l + 1` |
| gecode/flatzinc/flatzinc.cpp:367 | site 8 |
| gecode/flatzinc/flatzinc.cpp:968 | site 9 |
| gecode/flatzinc/flatzinc.cpp:981 | site 10 |
| gecode/flatzinc/registry.cpp:1273 | site 11 |
| gecode/flatzinc/registry.cpp:1278 | positive: `s.size() + 1` |
| gecode/gist/gist.cpp:112 | positive: `strlen + 32` (MSVC only) |
| gecode/gist/node.cpp:62 | positive: the `default` case of a switch that handles 0, 1, and 2 |
| gecode/gist/node.hpp:43 | positive: grows from `n >= 10`; generic element-wise realloc |
| gecode/gist/node.hpp:45, 50 | positive: `sizeof(Block)`, 10 |
| gecode/gist/visualnode.hpp:88 | positive: `sizeof(Shape)` plus `d - 1 >= 0` extents |
| gecode/instance/impl.hpp:139 | positive: capacity at least 2 |
| gecode/int/branch/view-values.cpp:40, 54 | positive: `n + 1` |
| gecode/int/extensional/dfa.hpp:82 | guarded: null for `nt == 0` |
| gecode/int/extensional/dfa.hpp:102 | positive: `1 << n_log` with `n_log >= 1` |
| gecode/int/extensional/layered-graph.hpp:308, 741 | generic `Heap::copy` of `Edge`, an element-wise loop with no `memcpy`; counts are positive |
| gecode/int/extensional/tuple-set.cpp:259, 352, 367 | site 4 |
| gecode/int/extensional/tuple-set.cpp:419, 437, 460, 508 | guarded: null for a zero count |
| gecode/int/extensional/tuple-set.cpp:436, 487 | positive: `n_offsets` is `n_vals + 1` |
| gecode/int/extensional/tuple-set.cpp:555 | site 5 |
| gecode/int/extensional/tuple-set.cpp:663, 668 | generic `Heap::copy` of local structs (element-wise); guarded by `n_edges > 0`, `n_supports > 0` |
| gecode/int/extensional/tuple-set.cpp:726 | `int` copy, so `memcpy`; length `i` is 0 on the first layer, but both pointers are Region blocks of at least one element: the destination holds `(i + 1) * n_tuples` with `n_tuples >= 1`, and only layer-0 state 0 has tuples (line 632) |
| gecode/int/extensional/tuple-set.hpp:65 | site 3 |
| gecode/int/int-set.cpp:77 | positive: every caller passes `n > 0` (int-set.cpp:162 and 231, int-set-1.hpp:73) |
| gecode/int/open-var-list.hpp:102 | guarded: `capacity > 0` |
| gecode/int/open-var-list.hpp:146 | positive: grows to at least 4 |
| gecode/kernel/archive.cpp:40 | positive: `max(n + 1, ...)`; growing from a null block with old count 0 is the valid case |
| gecode/kernel/archive.cpp:45, 46 | site 6 |
| gecode/kernel/archive.cpp:52, 53 | site 7 |
| gecode/kernel/branch/action.hpp:274 | site 1 |
| gecode/kernel/branch/chb.hpp:258 | site 2 |
| gecode/kernel/core.hpp:4605 | generic `Heap::copy<ActorLink*>` (element-wise); `prop` holds `m >= 1` entries |
| gecode/kernel/data/array.hpp:1507 | positive: heap only when `n > onstack_size` (16) |
| gecode/kernel/data/array.hpp:1519, 1540, 1547, 1573 | `a` always points to the on-stack buffer or a heap block, never null; 1547 is also guarded by `n > 0` |
| gecode/kernel/data/shared-array.hpp:187 | guarded: null for `n == 0` |
| gecode/kernel/memory/manager.hpp:207, 218 | comment and a positive chunk size |
| gecode/kernel/memory/region.cpp:88 | positive: reached only when the request exceeds the free chunk space |
| gecode/kernel/memory/region.cpp:93, 102 | positive: `sizeof(HeapInfo)` plus a positive block count |
| gecode/kernel/trace/filter.hpp:253 | guarded: null for `n == 0` |
| gecode/kernel/trace/filter.hpp:260, 268 | positive: 1 |
| gecode/minimodel/bool-expr.cpp:100, float-expr.cpp:103, float-expr.hpp:52, int-expr.cpp:138, int-expr.hpp:52, reg.cpp:99, set-expr.cpp:86 | positive: class `operator new`, `sizeof` |
| gecode/minimodel/bool-expr.cpp:669, float-arith.cpp:75, int-arith.cpp:74 | guarded: null for zero |
| gecode/minimodel/float-expr.cpp:270, 285, int-expr.cpp:428, 451, 471, 494 | guarded: `x.size() > 0` |
| gecode/search/par/bab.hpp:93, dfs.hpp:62, engine.hpp:175 | positive: `workers()`; parallel engines exist only for more than one thread |
| gecode/search/par/frontier.cpp:92 | positive: AStar and CABS build the parallel frontier only when `threads > 1` |
| gecode/search/par/pbs.hpp:196, seq/pbs.hpp:85 | positive: PBS builds a portfolio only for two or more assets (gecode/search/pbs.hpp:320-342) |
| gecode/support/dynamic-array.hpp:94, 111 | `copy` of an array whose size is the source's positive size (see b2) |
| gecode/support/thread/thread.hpp:55, 105, 193 | positive: class `operator new` |
| test/fault.cpp:597, 613, 617 | positive: 1 and 2 |
| test/package/allocator/boundary.cpp:5, 9 | pass-through used by main.cpp with 256 and 4096 |
| test/package/allocator/main.cpp:17, 46 | positive: 256, 4096 |
| test/test.cpp:144 | positive: the test count |

(b) Containers. `DynamicArray(A&, int n = 32)` and `(A&, unsigned int)`,
its copy constructor, and its assignment request `n` elements;
`DynamicStack(A&, int n = 64)` requests `n`; `DynamicQueue` requests 8 and
doubles; `StaticStack(A&, n)` requests `n`; the bit sets request
`BitSetData::data(sz + 1)`, at least one word; `BlockAllocator` requests
`sizeof(Block)`; `resize` and `realloc` grow to at least one more element.
Every Heap-backed instance found by (b2) uses the default size or 1
(driver/options.hpp:576, gist/gist.hpp:160), so no Heap-backed container
requests zero. Region-backed instances are outside the heap contract.

(c) `Support::allocator` is used only inside heap.hpp: `alloc` (362),
`realloc` (404), `free`, and `memcpy` in the scalar and pointer `copy`
specializations (600, 634). The generic `Heap::copy` copies element by
element and never calls `memcpy`. The generic `Heap::realloc` returns early
when `n == m`, but the scalar and pointer specializations call `rrealloc`
whatever `n` is (sites 5 and 7 come from this).

(d) Fixed-size requests: all 93 hits are `HeapAllocated` declarations or
class-level `operator new(size_t s)` forwarding `sizeof` of a complete
class to the heap or the space, which is positive.

(f) Direct C library calls:

| Location | Verdict |
|---|---|
| gecode/driver/options.cpp:60, 71 | positive length; both pointers non-null |
| gecode/flatzinc/lexer.yy.cpp:2032, 2053, 2352, 2384 (generated) | positive lengths (1, 8, `sizeof`) on fresh blocks |
| gecode/flatzinc/lexer.yy.cpp:2485, 2500 (generated) | `yyalloc`/`yyrealloc` wrappers; flex passes positive sizes |
| gecode/flatzinc/lexer.yy.cpp:248, parser.tab.cpp:1084 | comment and declaration |
| gecode/flatzinc/parser.hh:252 | positive: `num >= 1` because `pos < length` |
| gecode/int/ldsb/sym-imp.hpp:189, 191 | positive: only non-empty symmetries are created (`create_syms` and `is_empty` in int/ldsb.cpp:57-67 and 268-278, also used by set/ldsb.cpp:134-141), so `n_indices >= 1` and `lookup_size >= 1` on space blocks |
| gecode/kernel/core.hpp:3007 | site 13 |
| gecode/kernel/memory/region.cpp:105 | positive: `h->n >= 2` |
| gecode/search/cpprofiler/connector.hpp:316 | `sizeof hints` on a local |
| gecode/support/allocator.hpp:90, 94, 103 and allocator.cpp:50 | the wrappers themselves; their callers are (c) |
| test/flatzinc/blackbox-dll.cpp:177 | `sizeof(nan)` on locals |
| test/package/allocator/main.cpp:20, 32 | 256 |
| typed `realloc` members in core.hpp, region.hpp, heap.hpp (28 hits) | declarations and definitions of Gecode's own typed `realloc`, not the C function; their callers are in (a) and the Space::realloc grep |

Named constructors read directly: SharedArray's `SAO(int)` guards `n > 0`
(shared-array.hpp:187). ViewArray constructors and `update` allocate space
memory only for `n > 0` (array.hpp:1089-1107, 1320-1326). ArgArrayBase:
see (a). IntSet: every heap allocation goes through `IntSetObject::allocate`
with `n > 0`; the empty initializer lists are site 16. Region's heap
fallback: see (a) and the note before site 14. MemoryManager heap chunks:
positive chunk sizes.

## Runtime runs

### Scratch heap check

The check logs each zero-size `Heap::ralloc` or `Heap::rrealloc` call, and
each null pointer handed to the `memcpy`-based `Heap::copy`
specializations, with a backtrace on standard error. It does not abort, so
one run finds every site the suite reaches. It is compiled only when
`NDEBUG` is undefined. The `copy-null` kind is for discovery only: macOS
does not declare `memcpy` nonnull, so the null-pointer evidence comes from
the UBSan run on Linux.

The patch applies to a copy of the source, never to the checkout:

```sh
mkdir -p build/heap-audit/src build/heap-audit/orig
git archive HEAD | tar -x -C build/heap-audit/src
git archive HEAD | tar -x -C build/heap-audit/orig
# edit build/heap-audit/src as below, then:
(cd build/heap-audit && diff -ru orig src > heap-audit.patch)
```

build/heap-audit/heap-audit.patch:

```diff
diff -ru orig/gecode/support/heap.hpp src/gecode/support/heap.hpp
--- orig/gecode/support/heap.hpp	2026-11-03 12:37:04
+++ src/gecode/support/heap.hpp	2026-11-03 12:44:43
@@ -356,6 +356,10 @@
      */
     forceinline void*
     Heap::ralloc(size_t s) {
+#ifndef NDEBUG
+        if (s == 0)
+            HeapAudit::report("ralloc", 0);
+#endif
 #ifdef GECODE_HAS_FAULT_INJECTION
         Support::FailPoint::check(Support::FailPoint::Phase::HEAP);
 #endif
@@ -393,6 +397,10 @@
 
     forceinline void*
     Heap::rrealloc(void* p, size_t s) {
+#ifndef NDEBUG
+        if (s == 0)
+            HeapAudit::report(p == nullptr ? "rrealloc-null" : "rrealloc", 0);
+#endif
 #ifdef GECODE_HAS_FAULT_INJECTION
         Support::FailPoint::check(Support::FailPoint::Phase::HEAP);
 #endif
@@ -594,9 +602,18 @@
         return copy<T>(d, s, static_cast<long unsigned int>(n));
     }
 
+#ifndef NDEBUG
+#define GECODE_HEAP_AUDIT_COPY(d, s, n)                                                                                \
+    if ((d) == nullptr || (s) == nullptr)                                                                              \
+        HeapAudit::report("copy-null", (n));
+#else
+#define GECODE_HEAP_AUDIT_COPY(d, s, n)
+#endif
+
 #define GECODE_SUPPORT_COPY(T)                                                                                         \
     template <>                                                                                                        \
     forceinline T* Heap::copy(T* d, const T* s, long unsigned int n) {                                                 \
+        GECODE_HEAP_AUDIT_COPY(d, s, n)                                                                                \
         return static_cast<T*>(Support::allocator.memcpy(d, s, n * sizeof(T)));                                        \
     }                                                                                                                  \
     template <>                                                                                                        \
@@ -631,6 +648,7 @@
     template <class T>
     forceinline T**
     Heap::copy(T** d, const T** s, long unsigned int n) {
+        GECODE_HEAP_AUDIT_COPY(d, s, n)
         return static_cast<T**>(Support::allocator.memcpy(d, s, n * sizeof(T*)));
     }
     template <class T>
diff -ru orig/gecode/support.hh src/gecode/support.hh
--- orig/gecode/support.hh	2026-11-03 12:37:04
+++ src/gecode/support.hh	2026-11-03 12:44:43
@@ -41,6 +41,23 @@
 
 #include <gecode/support/config.hpp>
 
+#ifndef NDEBUG
+#include <cstdio>
+#include <execinfo.h>
+#include <unistd.h>
+namespace Gecode { namespace HeapAudit {
+    /// Log a zero-size heap request or null copy pointer with a backtrace
+    inline void report(const char* kind, size_t n) {
+        char line[96];
+        int len = std::snprintf(line, sizeof(line), "\n=== HEAP-AUDIT %s n=%zu\n", kind, n);
+        (void)!write(2, line, static_cast<size_t>(len));
+        void* f[32];
+        int depth = backtrace(f, 32);
+        backtrace_symbols_fd(f, depth, 2);
+    }
+}}
+#endif
+
 /*
  * Linking and compiler workarounds
  *
```

Build and run (Debug with experimental, CBS, Gist, Qt, and audit, as
build/full):

```sh
cmake -S build/heap-audit/src -B build/heap-audit/b -G Ninja -DCMAKE_BUILD_TYPE=Debug -DGECODE_ENABLE_EXPERIMENTAL=ON -DGECODE_ENABLE_CBS=ON -DGECODE_ENABLE_GIST=ON -DGECODE_ENABLE_QT=ON -DGECODE_ENABLE_AUDIT=ON -DQt6_DIR=/opt/homebrew/lib/cmake/Qt6 > build/heap-audit/cfg.log 2>&1
cmake --build build/heap-audit/b -j 10 --target gecode-test gecode-test-blackbox-exec gecode-test-blackbox-dll > build/heap-audit/build.log 2>&1
cmake --build build/heap-audit/b -j 4 --target fzn-gecode > build/heap-audit/build-fzn.log 2>&1
```

`ninja -C build/heap-audit/b -t commands check-standard | tail -1` gives the
target's command:

```sh
cd /Users/zayenz/.codex/worktrees/716f/gecode/build/heap-audit/b && /opt/homebrew/Cellar/cmake/3.27.6/bin/cmake -E env GECODE_TEST_BLACKBOX_EXEC=/Users/zayenz/.codex/worktrees/716f/gecode/build/heap-audit/b/bin/gecode-test-blackbox-exec GECODE_TEST_BLACKBOX_DLL=/Users/zayenz/.codex/worktrees/716f/gecode/build/heap-audit/b/libgecode-test-blackbox-dll.dylib GECODE_TEST_BLACKBOX_LOG=/Users/zayenz/.codex/worktrees/716f/gecode/build/heap-audit/b/gecode-test-blackbox /Users/zayenz/.codex/worktrees/716f/gecode/build/heap-audit/b/bin/gecode-test -tag standard
```

The run used the same command with `-threads 10` and separate output files:

```sh
cd build/heap-audit/b && /opt/homebrew/Cellar/cmake/3.27.6/bin/cmake -E env GECODE_TEST_BLACKBOX_EXEC=$PWD/bin/gecode-test-blackbox-exec GECODE_TEST_BLACKBOX_DLL=$PWD/libgecode-test-blackbox-dll.dylib GECODE_TEST_BLACKBOX_LOG=$PWD/gecode-test-blackbox $PWD/bin/gecode-test -tag standard -threads 10 > ../standard.out 2> ../standard.err
```

Under `-threads`, gecode-test buffers each test's output, so a stderr block
cannot be matched to a test by position. build/heap-audit/collapse.py groups
the blocks by kind and the demangled frames below the report, and names the
first `Test::` frame; `python3 -I build/heap-audit/collapse.py build/heap-audit/standard.err 4`
prints the table. build/heap-audit/probes/sites.sh stops lldb at
`Gecode::HeapAudit::report` and prints the first frame outside heap.hpp
with its file and line, because `backtrace_symbols` cannot name inlined
frames or source lines.

Result: the suite printed `Test summary: 6803 selected, 6803 ran, 6803
passed, 0 failed, 0 not run.` in 4678.3 s (exit status 0; the Mac was
heavily loaded by other work). The Linux run below selects 6804 tests: with
Gist built, `FlatZinc::Options::GistUnavailable` is not registered.
(`gecode-test -list` ignores `-tag` and lists all 54606 tests, so it cannot
count the selection.) Standard error holds 6 blocks, all `ralloc n=0`, in
two stacks:

| Blocks | Kind | Frames below the report (collapse.py) | Site (sites.sh) |
|---|---|---|---|
| 4 | ralloc | `TupleSet::TupleSet(int)`, from `SparseTupleSetNullary` | tuple-set.hpp:65:19 and 65:60, twice each (site 3) |
| 2 | ralloc | `TupleSet::Data::finalize(ExtensionalPropKind)`, from `SparseTupleSetNullary` | tuple-set.cpp:259:32 and 352:48 (site 4) |

The blocks come from the static initialization of the two
`Int::Extensional::TupleSet::Sparse::Nullary` tests. Each builds
`TupleSet(0)`, two blocks apiece. `+` adds one empty tuple before
finalizing (two blocks); `-` finalizes with no tuples, which returns before
any allocation. Static initialization constructs both tests in every
gecode-test run, so a run of `+` alone also logs all 6 blocks. There are
no `rrealloc`, `rrealloc-null`, or `copy-null` blocks, and no other stacks.

Sites the suite does not reach were checked with a scratch probe,
build/heap-audit/probes/probe.cpp, linked against the patched libraries:

```sh
cd build/heap-audit
c++ -std=c++23 -g -O0 -Isrc -Ib probes/probe.cpp -Lb -lgecodesearch -lgecodeint -lgecodekernel -lgecodesupport -Wl,-rpath,$PWD/b -o probes/probe
probes/sites.sh 2 probes/probe archive-copy
```

| Probe case | Code | Hits (kind and first frame outside the heap) |
|---|---|---|
| archive-copy | `Archive a; Archive b(a);` | ralloc at archive.cpp:45; copy-null at archive.cpp:46 |
| archive-assign | `Archive a; Archive b; b = a;` | rrealloc-null at archive.cpp:52; copy-null at archive.cpp:53 |
| int-action | `IntAction a(m, IntVarArgs());` | ralloc at action.hpp:274 |
| int-chb | `IntCHB c(m, IntVarArgs());` | ralloc at chb.hpp:258 |
| tupleset-0 | `TupleSet t(0); t.finalize();` | ralloc twice at tuple-set.hpp:65 |
| tupleset-0-grow | `TupleSet t(0);`, 1025 × `t.add(IntArgs())`, `t.finalize();` | ralloc twice at tuple-set.hpp:65; rrealloc at tuple-set.cpp:555; ralloc at tuple-set.cpp:259, 352, 367 |
| tupleset-dfa-0 | `DFA d(0, {{0, 1, 1}}, {1}); TupleSet t(0, d);` | ralloc twice at tuple-set.hpp:65 |
| tupleset-dfa-empty | `DFA d(0, {}, {0}); TupleSet t(1, d);` | none (Region only, site 15) |
| dfa-no-transitions | `DFA d(0, {}, {0});` | none (Region only, site 14) |
| tupleset-2-empty | `TupleSet t(2); t.finalize();` | none |
| space-realloc | `m.realloc<int>(nullptr, 0, 4)` | none (null `memcpy`, UBSan only, site 13) |

The FlatZinc inputs are in build/heap-audit/probes/*.fzn, run as
`build/heap-audit/b/bin/fzn-gecode <file>`:

| File | Hit |
|---|---|
| int-empty.fzn | ralloc at flatzinc.cpp:367 |
| set-assigned-empty.fzn | ralloc at flatzinc.cpp:968 |
| set-ub-empty.fzn | ralloc at flatzinc.cpp:981 |
| regular-finals-reversed.fzn | ralloc at registry.cpp:1273, then SIGBUS (exit status 138) |
| regular-finals-1-0.fzn | none; prints `=====UNSATISFIABLE=====` |

The dictionary inputs are in build/heap-audit/probes/dict-*.txt, read by
the patched examples:

```sh
cmake --build build/heap-audit/b -j 6 --target word-square crossword > build/heap-audit/build-examples.log 2>&1
cd build/heap-audit
printf '123' > probes/dict-no-word.txt
b/bin/word-square -file probes/dict-no-word.txt > probes/dict-no-word.out 2> probes/dict-no-word.err
b/bin/crossword -file probes/dict-no-word.txt > probes/cw-dict-no-word.out 2> probes/cw-dict-no-word.err
probes/sites.sh 1 b/bin/word-square -file probes/dict-no-word.txt
```

| File (contents) | Hit |
|---|---|
| dict-no-word.txt (`123`, no newline) | ralloc at scowl.hpp:7823:49, in word-square and crossword |
| dict-empty.txt (empty) | none |
| dict-newline.txt (`abc` and a newline) | none |
| dict-word-no-newline.txt (`abc`, no newline) | none |

With dict-no-word.txt, word-square logs one `ralloc n=0` block and exits
with status 1 after `Error: size must be between 0 and 0`; crossword logs
the same block, then searches and finds no solution (status 0).

### UBSan run

macOS cannot run this check. A probe on this Mac (Apple clang 21,
`-fsanitize=undefined`, also with the nullability checks) reported nothing
for `memcpy`, `memset`, or `memmove` with null pointers, because the macOS
SDK does not declare them nonnull. glibc does, so the run used erebor.

Positive control on erebor, ~/heap-audit/probe2.cpp: `memcpy(p, p, n)`,
`memmove(p, p, n)`, and `memset(p, 0, n)` with `p` null at run time and
`n = argc - 1 = 0`. Clang 21.1.8 with `-fsanitize=undefined` reports all
five null arguments at `-O0`, `-O1`, and `-O2` ("null pointer passed as
argument 1, which is declared to never be null"). GCC 15.2 at `-O0`
reports only the `memset`; at `-O1` it reports all five. The run therefore
uses Clang.

Build and run on erebor, from ~/heap-audit with the unpatched source
(`git archive HEAD | ssh erebor-dev 'mkdir -p ~/heap-audit/src && tar -x -C ~/heap-audit/src'`).
The build is Debug, so assertions stay on, but compiled at `-O1`: on four
cores the `-O0` suite would have taken far longer, and the control shows
`-O1` still reports. Gist is not built there (no Qt; `GECODE_ENABLE_GIST`
is AUTO).

```sh
CXX=clang++ CC=clang cmake -S src -B ubsan -G Ninja -DCMAKE_BUILD_TYPE=Debug "-DCMAKE_CXX_FLAGS_DEBUG=-g -O1" "-DCMAKE_C_FLAGS_DEBUG=-g -O1" -DGECODE_SANITIZER=undefined -DGECODE_ENABLE_EXPERIMENTAL=ON -DGECODE_ENABLE_CBS=ON > ubsan-cfg.log 2>&1
cmake --build ubsan -j 4 --target gecode-test gecode-test-blackbox-exec gecode-test-blackbox-dll > ubsan-build.log 2>&1
cd ~/heap-audit/ubsan && UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=0:log_path=$HOME/heap-audit/ubsan-report /usr/bin/cmake -E env GECODE_TEST_BLACKBOX_EXEC=$HOME/heap-audit/ubsan/bin/gecode-test-blackbox-exec GECODE_TEST_BLACKBOX_DLL=$HOME/heap-audit/ubsan/libgecode-test-blackbox-dll.so GECODE_TEST_BLACKBOX_LOG=$HOME/heap-audit/ubsan/gecode-test-blackbox $HOME/heap-audit/ubsan/bin/gecode-test -tag standard -threads 4 > $HOME/heap-audit/ubsan-standard.out 2> $HOME/heap-audit/ubsan-standard.err
```

The script ~/heap-audit/run-ubsan.sh runs the build and the suite, started
with `setsid nohup ./run-ubsan.sh > run-ubsan.log 2>&1 < /dev/null &`. The
target's own command, from `ninja -C ubsan -t commands check-standard`, is
the same without `-threads 4` and the environment variable.

Result: the build succeeded and the suite printed `Test summary: 6804
selected, 6804 ran, 6804 passed, 0 failed, 0 not run.` in 1147.6 s
(~/heap-audit/ubsan-status: `build-exit=0`, `test-exit=0 seconds=1149`).
UBSan wrote no report file and nothing to standard error: zero reports. The
instrumentation was active: libgecodekernel.so.52 imports
`__ubsan_handle_nonnull_arg`, gecode-test defines it, and the probe below,
linked against the same libraries, reports. The logs are copied to
build/ubsan-audit.

The suite reaches none of the null-pointer sites. The scratch probe
(build/heap-audit/probes/probe.cpp, copied to ~/heap-audit/probe-gecode.cpp)
built against the UBSan libraries shows them:

```sh
cd ~/heap-audit
clang++ -std=c++23 -g -O1 -fsanitize=undefined -Isrc -Iubsan probe-gecode.cpp -Lubsan -lgecodesearch -lgecodeint -lgecodekernel -lgecodesupport -Wl,-rpath,$HOME/heap-audit/ubsan -o probe-gecode
UBSAN_OPTIONS=print_stacktrace=1 ./probe-gecode archive-copy
addr2line -f -C -i -e ubsan/libgecodekernel.so.52 0x1bbdd
```

| Probe case | Report | Frame 0 (addr2line) |
|---|---|---|
| archive-copy | gecode/support/allocator.hpp:103:28: null pointer passed as argument 2 | `Gecode::Archive::Archive(Gecode::Archive const&)` |
| archive-assign | gecode/support/allocator.hpp:103:28: null pointer passed as argument 2 | `Gecode::Archive::operator=(Gecode::Archive const&)` |
| space-realloc | gecode/kernel/core.hpp:3007:23: null pointer passed as argument 2 | the probe (inlined `Space::rrealloc`) |
| int-action, int-chb, tupleset-0, tupleset-0-grow | none | |

Each case runs in its own process, because UBSan reports a source location
only once per process and both Archive cases fail at allocator.hpp:103.
Argument 1 is not null in the Archive cases because glibc returns a
non-null block for `malloc(0)` and `realloc(nullptr, 0)`; the C standard
does not promise that. The output is in build/ubsan-audit/probe-gecode.log.

## Reproducing

Heap check, one site the suite reaches (sites 3 and 4), in the patched
build:

```sh
build/heap-audit/b/bin/gecode-test -test 'Int::Extensional::TupleSet::Sparse::Nullary::+' -threads 1 > build/heap-audit/validate2.out 2> build/heap-audit/validate2.err
grep -c '=== HEAP-AUDIT ralloc n=0' build/heap-audit/validate2.err
build/heap-audit/probes/sites.sh 6 build/heap-audit/b/bin/gecode-test -test 'Int::Extensional::TupleSet::Sparse::Nullary::+' -threads 1
```

The test passes, the count is 6, and sites.sh prints tuple-set.hpp:65:19
and 65:60 twice each, then tuple-set.cpp:259:32 and 352:48.

UBSan, the null-pointer sites (6, 7, and 13), on erebor:

```sh
ssh -n erebor-dev 'cd ~/heap-audit && for c in archive-copy archive-assign space-realloc; do echo "== $c"; UBSAN_OPTIONS=print_stacktrace=1 ./probe-gecode $c 2>&1 | grep -E "runtime error|#0"; done'
```

Each case prints one `null pointer passed as argument 2` report:
allocator.hpp:103:28 for both Archive cases (frame 0 in
libgecodekernel.so.52 at 0x1bbdd and 0x1bd6f) and core.hpp:3007:23 for
`space-realloc`.
