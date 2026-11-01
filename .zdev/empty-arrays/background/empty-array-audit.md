# Empty-array posting audit

This audit covers every public posting function in the int, set, float and
minimodel modules that takes an array. Each was posted with empty arrays to
see whether any call path makes a zero-size space, region or heap request,
or aborts. Area: empty-arrays (Gecode 7). Area base: 0c81e65ae5. Fixes:

| Task | Commit | Families |
|---|---|---|
| empty-arrays-001 | 5f9a8e0e71 | no_overlap; allocator contract in Doxygen |
| empty-arrays-003 | d46423d0b1 | count with an array of counts (GCC), bin_packing |
| empty-arrays-004 | 79eb55e86b | LDSB branching with symmetries, relax |
| empty-arrays-005 | 46e9ea8dfa | set channel_sorted, channel with Booleans, element, rel with a constant, weights |
| empty-arrays-006 | 9e21fc1e10 | int, Bool and float linear; minimodel linear, Boolean element and float min/max expressions |
| empty-arrays-007 | c9a14405f1 | distinct over an open list |

After these commits, no public posting function aborts on empty input in a
Debug build with audit enabled. The only zero-size requests left are the
recorded ones in the last section (kernel Action/CHB storage and the DFA
and TupleSet constructors).

## Method

Inventory. Every public declaration in gecode/int.hh, gecode/set.hh,
gecode/float.hh and gecode/minimodel.hh that takes an array was listed
mechanically: libclang parsed the headers and listed each namespace-level
function or function template, and each public constructor or method of a
class defined in them, whose parameters mention an argument or variable
array, a Matrix or Slice, Symmetries, an open variable list, a SharedArray
or an initializer list. This gave 427 entries: 346 functions (posting
functions, branch and assign wrappers, symmetry builders, expression
builders and Matrix helpers) and 81 members (array, AFC, Action, CHB,
LinIntExpr, LinFloatExpr, REG, DFA, TupleSet, IntDistance and Slice
constructors and methods). A second libclang pass mapped every call in the
driver to the declaration it resolves to, so the comparison between the
inventory and the driver is exact rather than by name.

Driver. A throwaway program posted the inventory with empty arrays and
otherwise minimal valid arguments. It ran every probe in a forked child, so
one abort did not hide the rest, and called status(). The model space held
two integer variables with domain 0..2, a Boolean variable, two set
variables with lub 0..3 and a float variable with domain 0..1, used where an
overload needs a non-array argument. An open list was probed in two
states: created with maximum length 0, and unbounded with no position
created yet.

The first round had 375 probes. It ran in a Debug build with audit, fault
injection, CBS and Gist enabled: before the repairs (at 5f9a8e0e71, after
empty-arrays-001; 5fa5de8b16 only adds task files) and again at
9e21fc1e10, after them. The no_overlap results before its repair come from
empty-arrays-001's own check at the area base. The comparison with the
inventory then showed that this round called 270 of the 427 entries and
missed 100 public functions, among them six of the eight cumulatives
overloads, every open-list overload, element over Matrix<IntSetArgs>, the
element overloads with offsets, 15 of the 18 at_most, at_least and exactly
overloads, and several branch and assign wrappers.

A second round added 180 probes for the missed entries and ran all 555 at
9e21fc1e10, in the same Debug build and in the instrumented build, each
once with status() only and once with cloning and a short depth-first
search (at most 200 nodes), which also runs copy() and commit(). The
first-round probes gave the same results as before. The driver now calls
391 inventory entries. The remaining 36 are covered by source trace: copy
constructors and assignment of argument and variable arrays (ArgArrayBase
uses its inline buffer for size zero, VarArray copies a handle), init()
methods with the same body as their constructors, VarBranch::expand(),
which every AFC, ACTION and CHB branching probe reaches, open-list handle
conversions, the Slice constructor reached through Matrix::slice(), and
three declarations without a definition (recorded below). Rows added in
the second round were checked at 9e21fc1e10. Of the area's commits, only
empty-arrays-004 (LDSB rows) and empty-arrays-007 (open-list distinct)
touch their paths. For 007, the full probe run was repeated at 11d9731307
and with the 007 change applied: only the three open-list distinct probes
changed.

Instrumented build. A throwaway copy of the sources replaced the
`sz > 0` assertion in MemoryManager::alloc, and added checks to
Region::ralloc and Heap::ralloc, so that each zero-size request printed a
backtrace instead. The same probes ran against it, except cbs_branch, which
needs a CBS build. This shows every zero-size request on the executed path,
including those that Region and the heap accept without any assertion.
The driver, the inventory scripts and the instrumented build were throwaway
and are not part of the tree.

Source trace. For each family that the driver or the instrumented build
flagged, for each repaired family, and for each inventory entry that the
driver does not call, the code was read to confirm where the size comes
from, which checks come first, and what the documented result is.

In the tables, "driver" means the driver and the instrumented build;
"trace" means a source trace; "both" means both. The empty-input result is
the result after the repairs: "solved" or "fails" is the status() of the
space, "throws" names the exception thrown when posting, and "fixed" names
the task that repaired the path. A result with no fix was already correct
at the area base and makes no zero-size request. "Open list" means an
OpenIntVarList or OpenBoolVarList in either state above.

## Kernel containers

The ViewArray constructors (gecode/kernel/data/array.hpp) and
SharedArray::SAO (gecode/kernel/data/shared-array.hpp) do not allocate for
size zero, so no caller needed a change for them. The IntVarArray,
BoolVarArray, SetVarArray and FloatVarArray constructors with size zero
solve with no zero-size request (driver), and so do IntArgs, IntVarArgs,
BoolVarArgs, SetVarArgs and FloatValArgs built from no elements, IntArgs
and FloatValArgs built from an empty SharedArray, and the IntVarArray,
BoolVarArray, SetVarArray and FloatVarArray constructors from empty
argument arrays (driver). The IntSet constructors from an empty initializer
list abort (recorded below). Region::ralloc
(gecode/kernel/memory/region.hpp) has no size assertion and Heap::ralloc(0)
is malloc(0) (gecode/support/heap.hpp). That is why a zero-size Region or
heap request gives a correct result and cannot fail a test. The allocator
contract (sizes greater than zero) is in the Doxygen of Space::alloc,
Space::ralloc and MemoryManager::alloc (empty-arrays-001).

## Int module

Sources are in gecode/int/ unless a path is given.

| Posting function | Checked | Empty-input result | Fix |
|---|---|---|---|
| dom(x, ...) for IntVarArgs and BoolVarArgs (value, range, IntSet, dom(x, d)) | driver | solved, nothing posted | - |
| rel(x, irt, y) and rel(x, irt, c), array against a variable or constant | driver | solved | - |
| rel(x, irt), pairwise | driver | solved; IRT_NQ fails | - |
| rel(x, irt, y) lexicographic over arrays and IntArgs, int and Bool | driver | IRT_EQ, IRT_LQ solve; IRT_LT, IRT_NQ, IRT_GR fail | - |
| rel(BoolOpType, x, y) and rel(BoolOpType, x, n) | driver | BOT_AND, BOT_OR, BOT_XOR, BOT_EQV decided (BOT_OR with 1 and BOT_XOR with 1 fail); BOT_IMP throws Int::TooFewArguments (documented) | - |
| clause(op, x, y, z) and clause(op, x, y, n) | driver | solved; BOT_OR with 1 fails | - |
| precede(x, s, t) and precede(x, c) | driver | solved | - |
| member(x, y), int and Bool | driver | fails; reified forms solve (r decided) | - |
| element, all twelve one-dimensional overloads, with and without offset | driver | throws Int::TooFewArguments (documented) | - |
| element, all eight two-dimensional overloads, with and without offsets, w = h = 0 | both | throws Int::VariableEmptyDomain (undocumented, recorded below) | - |
| inter_distance(x, p) and inter_distance(x, IntDistance) | driver | solved | - |
| distinct(x), distinct(n, x), distinct(b, x), distinct(x, c) | driver | solved | - |
| channel(x, y), channel(x, xo, y, yo) | driver | solved | - |
| channel(BoolVarArgs x, IntVar y, o) | driver | fails (y cannot take a value in o..o-1) | - |
| sorted(x, y) and sorted(x, y, z) | driver | solved | - |
| count(x, n / IntSet / y / IntArgs, irt, m / z) | driver | solved; EQ 1 fails | - |
| count(x, c) and count(x, c, v), all five GCC overloads: IntVarArgs or IntSetArgs counts with or without values, and IntSet counts with values (gcc.cpp, gcc/post.hpp) | both | empty x: every count becomes 0 (constant counts fail unless they contain 0), nothing posted; x non-empty with no values: fails | fixed, 003 |
| nvalues(x, irt, y / c), int and Bool | driver | EQ 0 and variable forms solve; EQ 1 fails | - |
| sequence(x, s, q, l, u), int and Bool | driver | throws Int::TooFewArguments (documented) | - |
| extensional(x, DFA) and all extensional(x, TupleSet, ...) overloads (with and without the positive flag, plain and reified), int and Bool | driver | decided by whether the empty word or tuple is accepted; constructor requests recorded below | - |
| minimum_distance(x, ...) | driver | throws Int::TooFewArguments (documented); IntDistance(0) throws Int::OutOfLimits | - |
| min, max, argmin, argmax (int and Bool, argmin and argmax with and without offset) | driver | throws Int::TooFewArguments (documented) | - |
| product(x, y), product_mod(x, m / M, y), plain and reified | driver | solved | - |
| linear, every IntVarArgs, IntArgs-weighted and BoolVarArgs overload (linear-int.cpp, linear-bool.cpp) | both | decided at posting, unchanged: EQ 0 solves, EQ 1 and NQ 0 fail, IntVar right-hand side fixed or decided, reification decided | fixed, 006 (zero-size request removed) |
| bin_packing(l, b, s) (bin-packing.cpp, bin-packing/propagate.cpp) | both | no items: every load becomes 0, nothing posted | fixed, 003 |
| bin_packing(d, l, b, s, c), multi-dimensional | both | no items: loads 0, returns IntSet::empty; size mismatch throws Int::ArgumentSizeMismatch; d = 0 recorded below | fixed, 003 |
| no_overlap, all four overloads (no-overlap.cpp) | both | solved, nothing posted | fixed, 001 |
| unary, every overload (start/duration, tasks, end times, optional) | driver | solved, nothing posted | - |
| cumulative, every overload (int and IntVar capacity, tasks, end times, optional) | driver | solved, nothing posted; negative capacity throws Int::OutOfLimits | - |
| cumulatives(m, s, p, e, u, c, at_most), all eight overloads (m, p and u as IntArgs or IntVarArgs), c empty or one capacity, at_most true and false | driver | solved, propagator posted, no zero-size request | - |
| circuit and path, every overload, with and without offset | driver | throws Int::TooFewArguments (documented) | - |
| wait(x, c), unshare(x), int and Bool | driver | solved | - |
| IntAFC and BoolAFC on empty arrays | driver | solved | - |
| IntAction, BoolAction, IntCHB, BoolCHB on empty arrays, constructors and init() (init() of the Bool classes shares the constructor body, traced) | both | solved; kernel heap request recorded below | - |
| branch and assign, int and Bool, every wrapper overload except assign with tie-break (next row): with a variable selection, with tie-break, and with a value selection only; with filter and print functions; NONE, size, degree, AFC, merit, random and value-function selections; value sets (branch.cpp) | driver | solved, brancher posted | - |
| assign with tie-break, int and Bool (declared in gecode/int.hh) | trace | does not link (recorded below) | - |
| branch with ACTION or CHB merits, int and Bool | driver | solved; kernel heap request recorded below | - |
| branch with Symmetries, int and Bool, plain and tie-break, every symmetry builder over empty arrays (ldsb.cpp, ldsb/brancher.hpp) | both | empty handles dropped; empty or no symmetries give a plain LDSB brancher; LDSBBadValueSelection unchanged | fixed, 004 |
| cbs_branch, int and Bool | driver | solved, brancher posted (CBS build only) | - |
| relax(x, sx, r, p), int and Bool (relax.cpp) | both | returns after its argument checks, nothing posted | fixed, 004 |
| trace(x), trace(x, te), int and Bool | driver | solved | - |

The branch and assign wrappers share one path per variable type: expand()
the variable selections, build a ViewArray from x and post a view-value
brancher. With no views, the brancher's status() is false at once and the
space is solved. Cloning and search over these probes found no zero-size
request.

Open lists. Sources are in gecode/int/ unless a path is given.

| Posting function | Checked | Empty-input result | Fix |
|---|---|---|---|
| rel(x, irt) over an open list (rel/open.hpp) | driver | solved (consecutive pairs only, documented); no zero-size request | - |
| precede(x, s, t) and precede(x, c) over an open list (precede/open.hpp) | driver | solved; nothing posted for an empty c | - |
| distinct(x) over an open list (distinct/open.hpp) | both | maximum length 0: solved, nothing posted; unbounded: solved at the root, and positions added later must be distinct | fixed, 007 |
| sequence(x, s, q, l, u) and sliding_sum(x, q, l, u) over an open list (sequence/open.hpp) | driver | solved (a list shorter than q satisfies them, documented) | - |
| extensional(x, DFA) over an open list, int and Bool (extensional/open.hpp) | driver | maximum length 0: solved if the DFA accepts the empty word, fails otherwise; unbounded: solved at the root | - |
| min(x, y) and max(x, y) over an open list | driver | maximum length 0 fails (the sequence must be nonempty, documented); unbounded: solved at the root | - |
| branch over an open list, int and Bool: default, standard selectors and tie-break (branch/open.hpp) | driver | maximum length 0: solved; unbounded: branches, and search extends the list | - |
| branch over an open list with ACTION or CHB merits | driver | maximum length 0: solved, kernel heap request recorded below; unbounded: throws Int::UnknownBranching (documented) | - |

## Set module

Sources are in gecode/set/ unless a path is given.

| Posting function | Checked | Empty-input result | Fix |
|---|---|---|---|
| dom(x, srt, ...), dom(x, d), cardinality(x, i, j) | driver | solved, nothing posted | - |
| rel(op, x, y) and rel(op, IntVarArgs x, y) | driver | SOT_UNION, SOT_DUNION: y empty; SOT_INTER fails with the default universe; SOT_MINUS throws Set::IllegalOperation (documented) | - |
| rel(op, x, IntSet z, y) and rel(op, IntVarArgs x, z, y), SOT_UNION and SOT_DUNION (rel-op/union.hpp, rel-op/partition.hpp) | both | y = z, nothing posted | fixed, 005 (zero-size request removed) |
| rel(SOT_INTER, x, IntSet z, y), set and int arrays | driver | solved (y = z), propagator posted, no zero-size request | - |
| sequence(x) | both | throws Set::TooFewArguments (undocumented, recorded below) | - |
| sequence(y, x) | driver | solved | - |
| atmost_one(x, c) | driver | solved | - |
| weights(elements, weights, x, y) (int/weights.hpp) | both | x empty, y = 0, nothing posted; size mismatch throws Set::ArgumentSizeMismatch | fixed, 005 (zero-size request removed) |
| channel(IntVarArgs x, SetVarArgs y) and channel(SetVarArgs x, SetVarArgs y) | driver | solved | - |
| channel_sorted(x, y) (channel/sorted.hpp) | both | y empty, nothing posted | fixed, 005 |
| channel(BoolVarArgs x, SetVar y) (channel/bool.hpp) | both | y empty, nothing posted | fixed, 005 |
| precede(x, s, t) and precede(x, c) | driver | solved | - |
| element(op, SetVarArgs / IntVarArgs x, y, z, u) (element/union.hpp, inter.hpp, disjoint.hpp) | both | union and disjoint union: y and z empty; intersection: z = u (fails with the default universe); nothing posted | fixed, 005 (zero-size request removed) |
| element(op, IntSetArgs / IntArgs x, y, z, u) (element/unionConst.hpp, inter.hpp) | both | union and disjoint union: y and z empty, nothing posted; intersection: z = u | fixed, 005 |
| element(x, IntVar y, z) for SetVarArgs and IntSetArgs, and the two-dimensional forms | both | throws Set::TooFewArguments (undocumented, recorded below) | - |
| wait(x, c) | driver | solved | - |
| SetAFC on empty arrays | driver | solved | - |
| SetAction, SetCHB, branch with ACTION or CHB merits | driver | solved; kernel heap request recorded below | - |
| branch and assign, every wrapper overload: with a variable selection, with tie-break, and with a value selection only; NONE, size, degree, AFC and random selections (branch.cpp) | driver | solved, brancher posted | - |
| branch with Symmetries, plain and tie-break (gecode/set/ldsb.cpp) | both | empty handles dropped, as for int | fixed, 004 |
| relax(x, sx, r, p) (relax.cpp) | both | returns after its argument checks | fixed, 004 |
| trace(x), trace(x, te) | driver | solved | - |

## Float module

Sources are in gecode/float/ unless a path is given.

| Posting function | Checked | Empty-input result | Fix |
|---|---|---|---|
| dom(x, n), dom(x, l, u), dom(x, d) | driver | solved, nothing posted | - |
| rel(x, frt, c) and rel(x, frt, y) | driver | solved | - |
| min(x, y) and max(x, y) | driver | throws Float::TooFewArguments (documented) | - |
| linear, every FloatVarArgs and FloatValArgs-weighted overload (linear.cpp) | both | decided at posting, unchanged: EQ 0 and LQ 0 solve, EQ 1 fails, FloatVar right-hand side and reification decided | fixed, 006 (zero-size request removed) |
| wait(x, c) | driver | solved | - |
| FloatAFC on empty arrays | driver | solved | - |
| FloatAction, FloatCHB, branch with ACTION or CHB merits | driver | solved; kernel heap request recorded below | - |
| branch and assign, every wrapper overload: with a variable selection, with tie-break, and with a value selection only; NONE, size, degree, AFC and random selections (branch.cpp) | driver | solved, brancher posted | - |
| relax(x, sx, r, p) (relax.cpp) | both | returns after its argument checks | fixed, 004 |
| trace(x), trace(x, te) | driver | solved | - |

## Minimodel module

Sources are in gecode/minimodel/ unless a path is given.

| Posting function | Checked | Empty-input result | Fix |
|---|---|---|---|
| at_most, at_least, exactly, all 18 overloads (value as int, IntVar or IntArgs; count as int or IntVar) | driver | decided (count = 0): a required count of 1 fails; an IntVar count is constrained accordingly | - |
| lex(x, irt, y), int and Bool | driver | IRT_LQ solves, IRT_LT fails | - |
| values(x, IntSet) | driver | solved for the empty set, fails otherwise | - |
| channel(x) returning a SetVar (default and IPL_DOM), channel(x, SetVar) | driver | solved | - |
| range(x, y, z) (posts set element union) | both | nothing posted | fixed, 005 (zero-size request removed) |
| roots(x, y, z) | driver | solved | - |
| rel and expr over sum(IntVarArgs), sum(IntArgs, IntVarArgs), sum(BoolVarArgs), sum(IntArgs, BoolVarArgs), sum(IntArgs), the four LinIntExpr array constructors, sums over a Matrix and a Slice; plain and reified (int-expr.cpp) | both | decided, unchanged | fixed, 006 (zero-size request removed) |
| expr and rel over min, max and element of IntVarArgs, BoolVarArgs and IntArgs | driver | throws Int::TooFewArguments (documented) | - |
| element(BoolVarArgs, LinIntExpr) expression (bool-expr.cpp, BElementExpr) | both | throws Int::TooFewArguments, unchanged | fixed, 006 (zero-size request removed) |
| element over a 0x0 Matrix of IntArgs (IntVar and BoolVar result), IntVarArgs, BoolVarArgs | driver | throws Int::VariableEmptyDomain (the 2D int element, recorded below) | - |
| element over a 0x0 Matrix of IntSetArgs and SetVarArgs | driver | throws Set::TooFewArguments (recorded below) | - |
| extensional(x, REG) with REG of an empty array | driver | throws MiniModel::TooFewArguments (documented) | - |
| rel over Boolean expressions built from empty sums or conjunctions | driver | solved | - |
| set expressions inter(x), set_union(x), set_disjoint_union(x) | driver | solved; rel(s == inter(x)) fails unless s is the universe | - |
| rel and expr over sum(FloatVarArgs), sum(FloatValArgs, FloatVarArgs) and the two LinFloatExpr array constructors; plain and reified (float-expr.cpp) | both | decided, unchanged | fixed, 006 (zero-size request removed) |
| expr over min(FloatVarArgs) and max(FloatVarArgs) (float-arith.cpp, ArithNonLinFloatExpr) | both | throws Float::TooFewArguments, unchanged | fixed, 006 (zero-size request removed) |
| rows_interchange, columns_interchange, rows_reflect, columns_reflect and diagonal_reflect over a 0x0 matrix (also 3x0, 0x3, and Bool and set matrices) | both | handle dropped by LDSB branching | fixed, 004 |
| Matrix and Slice helpers on a 0x0 matrix: slice(), get_array(), Slice reverse(), the five Slice operator+ forms, operator<< (matrix.hpp) | driver | empty arrays, no allocation | - |

## Repaired families: zero-size requests before and after

Counts are zero-size requests on one probe run, from the instrumented
build at the area base (before) and with the task applied (after); for 007,
before is 11d9731307, which already has the other repairs. Every request
was 0 bytes. "Abort" means the Debug build aborted; a count before
an abort stops at the abort.

| Family | Case | Before | After |
|---|---|---|---|
| no_overlap (001) | every overload, zero boxes | abort, 1 space (ManBox/OptBox array) | 0 |
| GCC count (003) | all five overloads, x = c = v empty (also the value-array overload at IPL_DOM and IPL_BND, which is the same overload) | abort (index 0 of an empty array, gcc/post.hpp), 1 region | 0 |
| GCC count (003) | x empty, one variable count | solved, 2 region (post_side_constraints, is_distinct) | 0 |
| GCC count (003) | x non-empty, no values | abort, 1 region | 0, fails |
| bin_packing (003) | no items, single or multi-dimensional | abort (index 0 of an empty item array, bin-packing/propagate.cpp) | 0 |
| bin_packing (003) | d = 0, all empty | solved, 3 region (conflict graph, maximum_clique) | 0 |
| LDSB branching (004) | Symmetries() or a symmetry over no variables or values, int, Bool, set | abort, 1 or 2 space (SymmetryImp array, symmetry objects) | 0 |
| LDSB branching (004) | LDSBBrancher copy with no symmetries | abort on clone, 1 space per clone | 0 |
| relax (004) | int, Bool, set, float, x empty | abort (bit 0 of an empty bitset, support/bitset-base.hpp) | 0 |
| set channel_sorted (005) | x empty | abort during propagation (index 0) | 0 |
| set channel with Booleans (005) | x empty | abort (intersect with 0..-1, integerset.hpp) | 0 |
| set element over constant sets (005) | SOT_UNION, SOT_DUNION, IntSetArgs and IntArgs | abort, 1 space + 2 region | 0 |
| set element over constant sets (005) | SOT_INTER | fails with the default universe, 1 region | 0, unchanged result |
| set element over variables (005) | SOT_UNION, SOT_DUNION, SOT_INTER; also minimodel range | correct result, 1 region | 0 |
| set rel with a constant (005) | SOT_UNION / SOT_DUNION, set and int arrays | correct result, 6 / 8 region | 0 |
| set weights (005) | no elements | correct result, 1 region | 0 |
| int and Bool linear (006) | every overload with an integer right-hand side, plain and reified | correct result, 1 region each | 0 |
| float linear (006) | every overload with a constant right-hand side, plain and reified | correct result, 1 region each | 0 |
| linear with a variable right-hand side (006) | int, Bool, float | correct result, 0 (buffer holds n + 1 terms) | 0 |
| LinIntExpr relations (006) | rel(sum(...) == c), reified, constant and Matrix sums | correct result, 1 region | 0 |
| LinFloatExpr relations (006) | rel(sum(...) == c), reified | correct result, 1 region | 0 |
| BElementExpr (006) | element(BoolVarArgs, LinIntExpr) | throws, 1 heap (constructor) | 0, still throws |
| ArithNonLinFloatExpr (006) | float min and max | throws, 1 heap each (constructor) | 0, still throws |
| open-list distinct (007) | maximum length 0, IPL_DEF | solved, 2 region (propagate(), current and next buffers), 1 propagator posted | 0, nothing posted |
| open-list distinct (007) | unbounded, no position yet, IPL_DEF and IPL_DOM | solved, 2 region per propagation | 0 |

For 006, one test-level check confirms the per-post count: with the source
part of the change reversed, each new Empty test makes one zero-size
request per empty post (the harness runs each Base test five times), and
none with it applied.

Probe results that changed between the area base and the final tree: every
abort above became solved, branching or failed as listed, and
rel(SOT_UNION / SOT_DUNION, x, z, y), set element over variables,
minimodel range, and weights now post no propagator. All other probes give
the same status before and after.

## FlatZinc

FlatZinc posts through the public functions, so the core fixes cover every
path it reaches; no FlatZinc change was needed. Checked by source trace
(gecode/flatzinc/registry.cpp, registry-set.cpp, flatzinc.cpp):

- global_cardinality, global_cardinality_closed, global_cardinality_low_up
  and global_cardinality_low_up_closed reach the GCC count funnels (003).
- bin_packing_load reaches single-dimensional bin_packing (003).
- gecode_link_set_to_booleans with index 0 reaches channel with Booleans
  (005); gecode_set_channel_sorted reaches channel_sorted (005).
- gecode_set_weights reaches weights (005); gecode_range reaches set element
  union (005).
- int_lin_*, bool_lin_* and float_lin_* with empty arrays reach the linear
  posting functions (006).
- array_set_element and the array_set_element_union, _intersect,
  _intersect_in and _partition variants never pass an empty array: they
  build the arrays with offset 1, which adds a dummy element.
- relax_and_reconstruct (LNS) does not call Gecode::relax, so the relax
  repair does not affect it.
- FlatZinc, the instance reader and the tools never create an open list,
  so the open-list distinct repair (007) does not affect them.

## Instance reader

The instance module (gecode/instance.hh, gecode/instance/) only reads data
and returns IntArgs, numbers and strings. It posts no constraints and makes
no space allocation. Its growable sequence (Seq::grow in
gecode/instance/impl.hpp) requests a capacity of at least two elements, so
it makes no zero-size heap request. Nothing to fix.

## Tests

Each repaired family has a Test::Base test next to its existing tests.
Test::Int::Test cannot express arity zero (CpltAssignment reads its first
domain, test/int.hpp).

| Test | File | Before the fix (Debug, audit) |
|---|---|---|
| Int::NoOverlap::Empty | test/int/no-overlap.cpp | aborts |
| Int::GCC::Empty | test/int/gcc.cpp | aborts |
| Int::BinPacking::Empty | test/int/bin-packing.cpp | aborts |
| LDSB::EmptyInt, LDSB::EmptyBool, LDSB::EmptySet | test/ldsb.cpp | abort |
| Search::Relax::Empty | test/search.cpp | aborts |
| Set::Channel::Sorted::Empty, Set::Channel::Bool::Empty | test/set/channel.cpp | abort |
| Set::Element::Const::Empty | test/set/element.cpp | aborts |
| Set::Element::Var::Empty | test/set/element.cpp | passes (zero-size region only) |
| Set::RelOpConst::Empty | test/set/rel-op-const.cpp | passes (zero-size region only) |
| Set::Int::Weights::Empty | test/set/int.cpp | passes (zero-size region only) |
| Int::Linear::Empty | test/int/linear.cpp | passes (zero-size region only) |
| Float::Linear::Empty | test/float/linear.cpp | passes (zero-size region only) |
| Int::MiniModel::LinRel::Empty | test/int/mm-lin.cpp | passes (zero-size region only) |
| Float::MiniModel::LinRel::Empty | test/float/mm-lin.cpp | passes (zero-size region only) |
| Int::MiniModel::BoolElement::Empty | test/int/mm-bool.cpp | passes (zero-size heap only) |
| Float::Arithmetic::MinMax::Empty | test/float/arithmetic.cpp | passes (zero-size heap only) |
| Int::OpenList::DistinctEmpty | test/int/open-var-list.cpp | its no-propagator check fails; the rest passes (zero-size region only) |

Tests that pass before the fix document the empty-input result. For them,
the before and after request counts above are the evidence, as the brief
asks; no allocator instrumentation was added to the tree.

## Recorded, not fixed

Each item was checked against the source at 9e21fc1e10.

- Kernel Action and CHB storage. The Action storage constructor
  (gecode/kernel/branch/action.hpp, `heap.alloc<double>(x.size())`) and the
  CHB storage constructor (gecode/kernel/branch/chb.hpp,
  `heap.alloc<Info>(x.size())`) make a zero-size heap request on empty
  arrays. This affects IntAction, BoolAction, SetAction, FloatAction, the
  matching CHB classes, and branch with ACTION or CHB merits. Results are
  correct. Out of scope: the area owns only allocation Doxygen in
  gecode/kernel, and this is not a posting function.
- bin_packing with d = 0. bin_packing(home, d, l, b, s, c) computes
  `l.size() / d` (gecode/int/bin-packing.cpp) before checking any size, an
  integer division by zero. It is undefined behaviour, not an allocation.
  On arm64 the all-empty probe still solves. Fixing it needs a decision on
  the result for d < 1, such as a new documented exception.
- Two-dimensional int element with w * h = 0. The element overloads with
  width and height create `IntVar xy(home, 0, w * h - 1)` in pair and
  pair_with_offsets (gecode/int/element.cpp), which throws
  Int::VariableEmptyDomain. One-dimensional element throws
  Int::TooFewArguments, and the 2D Doxygen in gecode/int.hh documents only
  OutOfLimits and ArgumentSizeMismatch. The minimodel Matrix element
  inherits this. Behaviour is consistent; documentation is missing.
- Set sequence(x) and set element(x, y, z). sequence(home, SetVarArgs)
  (gecode/set/sequence.cpp) and element(home, SetVarArgs / IntSetArgs, IntVar,
  SetVar) with its 2D forms (gecode/set/element.cpp) throw
  Set::TooFewArguments for an empty array. gecode/set.hh does not document
  it.
- Search::relax ignores a failing post. Search::relax (gecode/search/relax.hh)
  calls `home.failed()` where `home.fail()` is meant when a post returns
  other than ES_OK, so the space is not marked failed. This concerns
  non-empty input and is outside this area.
- DFA and TupleSet constructors. DFA::init (gecode/int/extensional/dfa.cpp)
  makes a zero-size Region request for its transition buffer when there are
  no transitions. TupleSet(0) makes zero-size heap requests in
  TupleSet::Data::Data (gecode/int/extensional/tuple-set.hpp) and in
  TupleSet::Data::finalize (tuple-set.cpp). Results are correct. These are
  constructors, not posting functions.
- IntSet from an empty initializer list. IntSet(std::initializer_list<int>)
  and IntSet(std::initializer_list<std::pair<int, int>>) assert `n > 0`
  (gecode/int/int-set.cpp), so an empty list aborts in Debug. The array
  forms IntSet(const int r[], int n) and IntSet(const int r[][2], int n)
  return the empty set for n = 0. Out of scope: a constructor of a value
  type, not a posting function.
- Declarations without a definition. The coverage check found three
  public declarations that a call cannot link against, whatever the input
  size. assign(home, IntVarArgs, TieBreak<IntVarBranch>, IntAssign, ...)
  is declared in gecode/int.hh, but gecode/int/branch.cpp defines that body
  under the name branch. assign(home, BoolVarArgs, TieBreak<BoolVarBranch>,
  BoolAssign, ...) is declared with IntBranchFilter and IntVarValPrint, but
  defined with BoolBranchFilter and BoolVarValPrint.
  FloatValArgs(std::initializer_list<FloatVal>) in gecode/float.hh has no
  definition. All three date from before this area and are outside it.
  Their empty-input paths were traced instead: the two assign bodies take
  the same path as the tie-break branch wrappers, which the driver covers.
- The branch(..., Symmetries) Doxygen in gecode/int.hh and gecode/set.hh
  could state that empty symmetries are ignored.
